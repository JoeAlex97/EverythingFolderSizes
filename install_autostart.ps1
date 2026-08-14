# Script to register and start FolderSizeExt immediately and silently on Windows startup
$scriptDir = $PSScriptRoot
if (-not $scriptDir) { $scriptDir = (Get-Location).Path }
$injectScript = Join-Path $scriptDir "inject_dll.ps1"
$vbsScript = Join-Path $scriptDir "run_silent.vbs"
$taskName = "EverythingFolderSizeExtension"

# Ensure run_silent.vbs exists
$vbsContent = @"
Set WshShell = CreateObject("WScript.Shell")
WshShell.Run "powershell.exe -ExecutionPolicy Bypass -WindowStyle Hidden -File """ & WScript.Arguments(0) & """", 0, False
"@
Set-Content -Path $vbsScript -Value $vbsContent -Encoding ASCII

Write-Host "Creating Scheduled Task to launch FolderSizeExt silently with Windows..." -ForegroundColor Cyan

$action = New-ScheduledTaskAction -Execute "wscript.exe" -Argument "`"$vbsScript`" `"$injectScript`""
$trigger = New-ScheduledTaskTrigger -AtLogOn
$principal = New-ScheduledTaskPrincipal -UserId $env:USERNAME -LogonType Interactive -RunLevel Highest
$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -ExecutionTimeLimit (New-TimeSpan -Hours 0)

Register-ScheduledTask -TaskName $taskName -Action $action -Trigger $trigger -Principal $principal -Settings $settings -Force | Out-Null

Write-Host "Injecting extension into open File Explorer windows..." -ForegroundColor Cyan
& powershell.exe -ExecutionPolicy Bypass -File "$injectScript"

Write-Host "`nSETUP COMPLETED SUCCESSFULLY!" -ForegroundColor Green
Write-Host "FolderSizeExt is now active and will automatically load silently on Windows startup." -ForegroundColor Yellow
