# Script to inject FolderSizeExt.dll into explorer.exe
$scriptDir = $PSScriptRoot
if (-not $scriptDir) { $scriptDir = (Get-Location).Path }

# Locate DLL in local folder (Release) or build folder (Development)
$dllPath = Join-Path $scriptDir "FolderSizeExt.dll"
if (-not (Test-Path $dllPath)) {
    $dllPath = Join-Path $scriptDir "build\FolderSizeExt.dll"
}

if (-not (Test-Path $dllPath)) {
    Write-Host "[ERROR] Could not find FolderSizeExt.dll." -ForegroundColor Red
    exit
}

$dllDir = Split-Path $dllPath -Parent

# Copy dbghelp.dll / symsrv.dll if available in Visual Studio path (Dev mode only)
$dbghelpSrc = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\TestWindow\VsTest\x64\dbghelp.dll"
$symsrvSrc  = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\Remote Debugger\x64\symsrv.dll"

if ((Test-Path $dbghelpSrc) -and (-not (Test-Path (Join-Path $dllDir "dbghelp.dll")))) {
    Copy-Item $dbghelpSrc (Join-Path $dllDir "dbghelp.dll") -Force -ErrorAction SilentlyContinue
}
if ((Test-Path $symsrvSrc) -and (-not (Test-Path (Join-Path $dllDir "symsrv.dll")))) {
    Copy-Item $symsrvSrc (Join-Path $dllDir "symsrv.dll") -Force -ErrorAction SilentlyContinue
}

Write-Host "Injecting $dllPath into explorer.exe..." -ForegroundColor Cyan

$explorerProcs = Get-Process -Name explorer -ErrorAction SilentlyContinue
if (-not $explorerProcs) {
    Write-Host "[ERROR] No explorer.exe process found." -ForegroundColor Red
    exit
}

$Win32 = Add-Type -MemberDefinition @"
[DllImport("kernel32.dll", SetLastError = true)]
public static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, int dwProcessId);

[DllImport("kernel32.dll", SetLastError = true)]
public static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

[DllImport("kernel32.dll", SetLastError = true)]
public static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out IntPtr lpNumberOfBytesWritten);

[DllImport("kernel32.dll", SetLastError = true)]
public static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

[DllImport("kernel32.dll", SetLastError = true)]
public static extern IntPtr GetModuleHandle(string lpModuleName);

[DllImport("kernel32.dll", SetLastError = true)]
public static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);
"@ -Name "Win32Inject" -Namespace "InjectUtils" -PassThru

$PROCESS_ALL_ACCESS = 0x001F0FFF
$MEM_COMMIT_RESERVE = 0x3000
$PAGE_READWRITE = 0x04

foreach ($proc in $explorerProcs) {
    Write-Host "Injecting into PID $($proc.Id)... " -NoNewline
    $hProcess = $Win32::OpenProcess($PROCESS_ALL_ACCESS, $false, $proc.Id)
    if ($hProcess -eq [IntPtr]::Zero) {
        Write-Host "[ERROR: Permission denied. Run as Administrator]" -ForegroundColor Red
        continue
    }

    $pathBytes = [System.Text.Encoding]::Unicode.GetBytes($dllPath + "`0")
    $allocAddr = $Win32::VirtualAllocEx($hProcess, [IntPtr]::Zero, [uint32]$pathBytes.Length, $MEM_COMMIT_RESERVE, $PAGE_READWRITE)

    $written = [IntPtr]::Zero
    $Win32::WriteProcessMemory($hProcess, $allocAddr, $pathBytes, [uint32]$pathBytes.Length, [ref]$written)

    $loadLibAddr = $Win32::GetProcAddress($Win32::GetModuleHandle("kernel32.dll"), "LoadLibraryW")
    $hThread = $Win32::CreateRemoteThread($hProcess, [IntPtr]::Zero, 0, $loadLibAddr, $allocAddr, 0, [IntPtr]::Zero)

    if ($hThread -ne [IntPtr]::Zero) {
        Write-Host "[OK]" -ForegroundColor Green
    } else {
        Write-Host "[FAILED]" -ForegroundColor Red
    }
}

Write-Host "`nNOTE: If you restart explorer.exe, the injection is cleared. Open a File Explorer window without restarting the process to view changes." -ForegroundColor Yellow
