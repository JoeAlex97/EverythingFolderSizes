# Script to register and start FolderSizeExt immediately and on Windows startup
$scriptDir = $PSScriptRoot
if (-not $scriptDir) { $scriptDir = (Get-Location).Path }
$injectScript = Join-Path $scriptDir "inject_dll.ps1"
$taskName = "EverythingFolderSizeExtension"

Write-Host "Creating Scheduled Task to launch FolderSizeExt with Windows..." -ForegroundColor Cyan

$action = New-ScheduledTaskAction -Execute "powershell.exe" -Argument "-ExecutionPolicy Bypass -WindowStyle Hidden -File `"$injectScript`""
$trigger = New-ScheduledTaskTrigger -AtLogOn
$principal = New-ScheduledTaskPrincipal -UserId $env:USERNAME -LogonType Interactive -RunLevel Highest
$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -ExecutionTimeLimit (New-TimeSpan -Hours 0)

Register-ScheduledTask -TaskName $taskName -Action $action -Trigger $trigger -Principal $principal -Settings $settings -Force | Out-Null

Write-Host "Injecting extension into open File Explorer windows..." -ForegroundColor Cyan
& powershell.exe -ExecutionPolicy Bypass -File "$injectScript"

Write-Host "`nSETUP COMPLETED SUCCESSFULLY!" -ForegroundColor Green
Write-Host "FolderSizeExt is now active and will automatically load on Windows startup." -ForegroundColor Yellow
