# Script to uninstall automatic startup of FolderSizeExt
$taskName = "EverythingFolderSizeExtension"

Write-Host "Uninstalling FolderSizeExt..." -ForegroundColor Cyan

Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue

Write-Host "Restarting Windows File Explorer..." -ForegroundColor Yellow
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 1

if (-not (Get-Process -Name explorer -ErrorAction SilentlyContinue)) {
    Start-Process explorer.exe
}

Write-Host "`nUNINSTALLATION COMPLETED!" -ForegroundColor Green
Write-Host "Auto-start task has been removed and File Explorer memory cleared." -ForegroundColor Yellow
