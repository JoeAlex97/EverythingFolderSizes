#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <propsys.h>
#include <dbghelp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>
#include "minhook/include/MinHook.h"
#include "EverythingClient.h"

#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")

static std::mutex g_LogMutex;
static void Log(const wchar_t* format, ...) {
    std::lock_guard<std::mutex> lock(g_LogMutex);
    wchar_t buf[1024];
    va_list args;
    va_start(args, format);
    vswprintf_s(buf, format, args);
    va_end(args);

    OutputDebugStringW(buf);

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring logPath = std::wstring(tempPath) + L"FolderSizeExt_debug.log";

    std::wofstream logFile(logPath, std::ios::app);
    if (logFile.is_open()) {
        logFile << buf << std::endl;
    }
}

// Signature of CFSFolder::_GetSize in windows.storage.dll
typedef HRESULT(WINAPI* CFSFolder__GetSize_t)(
    void* pCFSFolder,
    const ITEMID_CHILD* itemidChild,
    const void* idFolder,
    PROPVARIANT* propVariant
);

static CFSFolder__GetSize_t pfnCFSFolder__GetSize_Original = nullptr;
static bool g_IsHooked = false;

static std::wstring GetFolderPath(IShellFolder2* pShellFolder2, const ITEMID_CHILD* pidlChild) {
    if (!pShellFolder2) return L"";

    IShellFolder2* pChildFolder = nullptr;
    HRESULT hr = pShellFolder2->BindToObject(pidlChild, nullptr, IID_PPV_ARGS(&pChildFolder));
    if (FAILED(hr) || !pChildFolder) return L"";

    LPITEMIDLIST pidl = nullptr;
    hr = SHGetIDListFromObject(pChildFolder, &pidl);
    pChildFolder->Release();

    if (FAILED(hr) || !pidl) return L"";

    wchar_t szPath[MAX_PATH * 2] = { 0 };
    if (SHGetPathFromIDListW(pidl, szPath)) {
        CoTaskMemFree(pidl);
        return szPath;
    }

    CoTaskMemFree(pidl);
    return L"";
}

static HRESULT WINAPI Hook_CFSFolder__GetSize(
    void* pCFSFolder,
    const ITEMID_CHILD* itemidChild,
    const void* idFolder,
    PROPVARIANT* propVariant
) {
    HRESULT hr = pfnCFSFolder__GetSize_Original(pCFSFolder, itemidChild, idFolder, propVariant);

    if (propVariant->vt != VT_EMPTY) {
        return hr;
    }

    IShellFolder2* pShellFolder2 = nullptr;
    if (SUCCEEDED(((IUnknown*)pCFSFolder)->QueryInterface(IID_IShellFolder2, (void**)&pShellFolder2)) && pShellFolder2) {
        std::wstring folderPath = GetFolderPath(pShellFolder2, itemidChild);
        pShellFolder2->Release();

        if (!folderPath.empty()) {
            Log(L"CFSFolder::_GetSize llamado para carpeta: %s", folderPath.c_str());
            uint64_t folderSize = EverythingClient::GetFolderSize(folderPath.c_str());
            if (folderSize != UINT64_MAX) {
                propVariant->vt = VT_UI8;
                propVariant->uhVal.QuadPart = folderSize;
                Log(L"SUCCESS: Tamanio asignado a %s -> %llu bytes", folderPath.c_str(), folderSize);
                return S_OK;
            } else {
                Log(L"WARN: Everything retorno UINT64_MAX para %s", folderPath.c_str());
            }
        }
    }

    return hr;
}

// Hook de RegQueryValueExW para forzar que el Explorador pida System.Size en vistas de carpetas
typedef LSTATUS(WINAPI* RegQueryValueExW_t)(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
static RegQueryValueExW_t pfnRegQueryValueExW_Original = nullptr;

static LSTATUS WINAPI Hook_RegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
    LSTATUS status = pfnRegQueryValueExW_Original(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    if (lpValueName && _wcsicmp(lpValueName, L"ContentViewModeForBrowse") == 0) {
        if (status == ERROR_SUCCESS && lpData && lpcbData) {
            std::wstring val((wchar_t*)lpData, *lpcbData / sizeof(wchar_t));
            if (val.find(L"System.Size") == std::wstring::npos) {
                val += L";System.Size";
                DWORD newSize = static_cast<DWORD>((val.size() + 1) * sizeof(wchar_t));
                if (*lpcbData >= newSize) {
                    wcscpy_s((wchar_t*)lpData, *lpcbData / sizeof(wchar_t), val.c_str());
                    *lpcbData = newSize;
                }
            }
        }
    }

    return status;
}

static void* ResolveCFSFolderGetSizeSymbol(HMODULE hStorageModule) {
    wchar_t moduleDir[MAX_PATH];
    GetModuleFileNameW(GetModuleHandleW(L"FolderSizeExt.dll"), moduleDir, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(moduleDir, L'\\');
    if (lastSlash) *lastSlash = L'\0';

    std::wstring symsrvPath = std::wstring(moduleDir) + L"\\symsrv.dll";
    LoadLibraryW(symsrvPath.c_str());

    HANDLE hProcess = GetCurrentProcess();
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring symbolPath = L"srv*" + std::wstring(tempPath) + L"SymbolCache*https://msdl.microsoft.com/download/symbols;" + std::wstring(tempPath) + L"SymbolCache";

    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_INCLUDE_32BIT_MODULES);
    if (SymInitializeW(hProcess, symbolPath.c_str(), FALSE)) {
        wchar_t storagePath[MAX_PATH];
        GetModuleFileNameW(hStorageModule, storagePath, MAX_PATH);

        SymUnloadModule64(hProcess, (DWORD64)hStorageModule);

        DWORD64 baseAddr = SymLoadModuleExW(hProcess, nullptr, storagePath, nullptr, (DWORD64)hStorageModule, 0, nullptr, 0);
        if (baseAddr) {
            struct EnumContext {
                ULONG_PTR foundAddress;
            } ctx = { 0 };

            auto EnumSymCallback = [](PSYMBOL_INFOW pSymInfo, ULONG SymbolSize, PVOID UserContext) -> BOOL {
                EnumContext* pCtx = (EnumContext*)UserContext;
                if (pSymInfo->Name && wcscmp(pSymInfo->Name, L"CFSFolder::_GetSize") == 0) {
                    pCtx->foundAddress = (ULONG_PTR)pSymInfo->Address;
                    return FALSE;
                }
                return TRUE;
            };

            SymEnumSymbolsW(hProcess, baseAddr, L"*CFSFolder::_GetSize*", EnumSymCallback, &ctx);
            SymCleanup(hProcess);

            if (ctx.foundAddress) {
                Log(L"RESOLVED via PDB: Direccion de CFSFolder::_GetSize = 0x%p", (void*)ctx.foundAddress);
                return (void*)ctx.foundAddress;
            }
        }
        SymCleanup(hProcess);
    }

    // Fallback: Offset conocido (0x509c0) con los bytes del prologo
    constexpr DWORD_PTR kKnownOffset = 0x509c0;
    const unsigned char kPrologueBytes[] = { 0x40, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48 };

    BYTE* candidateAddr = (BYTE*)hStorageModule + kKnownOffset;
    if (memcmp(candidateAddr, kPrologueBytes, sizeof(kPrologueBytes)) == 0) {
        Log(L"RESOLVED via Fallback Offset (0x%X): Direccion = 0x%p", kKnownOffset, candidateAddr);
        return candidateAddr;
    }

    Log(L"ERROR: No se pudo resolver la direccion por PDB ni por Fallback Offset");
    return nullptr;
}

static bool InitHooks() {
    Log(L"--- Iniciando FolderSizeExt (con Hook RegQueryValueExW) ---");
    if (MH_Initialize() != MH_OK) {
        Log(L"ERROR: MH_Initialize fallo");
        return false;
    }

    HMODULE hStorageModule = GetModuleHandleW(L"windows.storage.dll");
    if (!hStorageModule) {
        hStorageModule = LoadLibraryW(L"windows.storage.dll");
    }
    if (!hStorageModule) {
        Log(L"ERROR: No se pudo cargar windows.storage.dll");
        return false;
    }

    void* pTargetAddress = ResolveCFSFolderGetSizeSymbol(hStorageModule);
    if (!pTargetAddress) {
        Log(L"ERROR: No se pudo resolver la direccion de CFSFolder::_GetSize");
        return false;
    }

    MH_STATUS status = MH_CreateHook(pTargetAddress, (LPVOID)&Hook_CFSFolder__GetSize, (LPVOID*)&pfnCFSFolder__GetSize_Original);
    if (status != MH_OK) {
        Log(L"ERROR: MH_CreateHook fallo con estatus: %d", status);
        return false;
    }

    // RegQueryValueExW Hook
    HMODULE hKernelBase = GetModuleHandleW(L"kernelbase.dll");
    if (hKernelBase) {
        void* pRegQuery = (void*)GetProcAddress(hKernelBase, "RegQueryValueExW");
        if (pRegQuery) {
            MH_CreateHook(pRegQuery, (LPVOID)&Hook_RegQueryValueExW, (LPVOID*)&pfnRegQueryValueExW_Original);
        }
    }

    status = MH_EnableHook(MH_ALL_HOOKS);
    if (status != MH_OK) {
        Log(L"ERROR: MH_EnableHook fallo con estatus: %d", status);
        return false;
    }

    g_IsHooked = true;
    Log(L"SUCCESS: Hooks instalados correctamente en 0x%p!", pTargetAddress);
    return true;
}

static void UninitHooks() {
    if (g_IsHooked) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        g_IsHooked = false;
        Log(L"--- FolderSizeExt Desinstalado ---");
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);
            InitHooks();
            break;
        case DLL_PROCESS_DETACH:
            UninitHooks();
            break;
    }
    return TRUE;
}
