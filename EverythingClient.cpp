#include "EverythingClient.h"
#include <vector>
#include <mutex>
#include <atomic>
#include <cwchar>

namespace EverythingClient {

#pragma pack(push, 1)
struct EVERYTHING3_MESSAGE {
    DWORD code;
    DWORD size;
};
#pragma pack(pop)

constexpr DWORD EVERYTHING3_CMD_GET_FOLDER_SIZE = 18;
constexpr DWORD EVERYTHING3_RESP_OK = 200;

static uint64_t QueryEverythingPipe(const wchar_t* pipeName, const wchar_t* folderPath) {
    HANDLE hPipe = CreateFileW(pipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hPipe == INVALID_HANDLE_VALUE) {
        return UINT64_MAX;
    }

    // Convert folder path to UTF-8
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, folderPath, -1, nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 1) {
        CloseHandle(hPipe);
        return UINT64_MAX;
    }

    std::vector<char> utf8Buf(utf8Len);
    WideCharToMultiByte(CP_UTF8, 0, folderPath, -1, utf8Buf.data(), utf8Len, nullptr, nullptr);

    EVERYTHING3_MESSAGE msg;
    msg.code = EVERYTHING3_CMD_GET_FOLDER_SIZE;
    msg.size = static_cast<DWORD>(utf8Len - 1); // length without null terminator

    DWORD written = 0;
    if (!WriteFile(hPipe, &msg, sizeof(msg), &written, nullptr) || written != sizeof(msg)) {
        CloseHandle(hPipe);
        return UINT64_MAX;
    }

    if (!WriteFile(hPipe, utf8Buf.data(), msg.size, &written, nullptr) || written != msg.size) {
        CloseHandle(hPipe);
        return UINT64_MAX;
    }

    EVERYTHING3_MESSAGE respMsg = {};
    DWORD bytesRead = 0;
    if (!ReadFile(hPipe, &respMsg, sizeof(respMsg), &bytesRead, nullptr) || bytesRead != sizeof(respMsg)) {
        CloseHandle(hPipe);
        return UINT64_MAX;
    }

    if (respMsg.code != EVERYTHING3_RESP_OK || respMsg.size != sizeof(uint64_t)) {
        CloseHandle(hPipe);
        return UINT64_MAX;
    }

    uint64_t resultSize = UINT64_MAX;
    if (ReadFile(hPipe, &resultSize, sizeof(resultSize), &bytesRead, nullptr) && bytesRead == sizeof(resultSize)) {
        CloseHandle(hPipe);
        return resultSize;
    }

    CloseHandle(hPipe);
    return UINT64_MAX;
}

uint64_t GetFolderSize(const wchar_t* folderPath) {
    if (!folderPath || !*folderPath) return UINT64_MAX;

    // 1. Try Everything 1.5a default pipe
    uint64_t size = QueryEverythingPipe(L"\\\\.\\PIPE\\Everything IPC", folderPath);
    if (size != UINT64_MAX) return size;

    // 2. Try Everything 1.5a named instance pipe
    size = QueryEverythingPipe(L"\\\\.\\PIPE\\Everything IPC (1.5a)", folderPath);
    if (size != UINT64_MAX) return size;

    return UINT64_MAX;
}

std::wstring FormatSize(uint64_t bytes, bool useIec) {
    if (bytes == UINT64_MAX) return L"";

    const double kKib = useIec ? 1024.0 : 1000.0;
    const wchar_t* units[] = {
        useIec ? L"B" : L"B",
        useIec ? L"KiB" : L"KB",
        useIec ? L"MiB" : L"MB",
        useIec ? L"GiB" : L"GB",
        useIec ? L"TiB" : L"TB"
    };

    if (bytes < static_cast<uint64_t>(kKib)) {
        wchar_t buf[32];
        swprintf_s(buf, L"%llu B", bytes);
        return buf;
    }

    double dSize = static_cast<double>(bytes);
    int unitIdx = 0;

    while (dSize >= kKib && unitIdx < 4) {
        dSize /= kKib;
        unitIdx++;
    }

    wchar_t buf[32];
    swprintf_s(buf, L"%.2f %s", dSize, units[unitIdx]);
    return buf;
}

} // namespace EverythingClient
