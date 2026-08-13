# Script para registrar e iniciar FolderSizeExt inmediatamente y en cada inicio de Windows
$scriptDir = $PSScriptRoot
if (-not $scriptDir) { $scriptDir = (Get-Location).Path }
$injectScript = Join-Path $scriptDir "inject_dll.ps1"
$taskName = "EverythingFolderSizeExtension"

Write-Host "Creando Tarea Programada para iniciar FolderSizeExt con Windows..." -ForegroundColor Cyan

$action = New-ScheduledTaskAction -Execute "powershell.exe" -Argument "-ExecutionPolicy Bypass -WindowStyle Hidden -File `"$injectScript`""
$trigger = New-ScheduledTaskTrigger -AtLogOn
$principal = New-ScheduledTaskPrincipal -UserId $env:USERNAME -LogonType Interactive -RunLevel Highest
$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -ExecutionTimeLimit (New-TimeSpan -Hours 0)

Register-ScheduledTask -TaskName $taskName -Action $action -Trigger $trigger -Principal $principal -Settings $settings -Force | Out-Null

Write-Host "Inyectando la extension en las ventanas del Explorador actuales..." -ForegroundColor Cyan
& powershell.exe -ExecutionPolicy Bypass -File "$injectScript"

Write-Host "`n¡INSTALACION COMPLETADA!" -ForegroundColor Green
Write-Host "FolderSizeExt esta activo ahora mismo y se cargara automaticamente en cada inicio de Windows." -ForegroundColor Yellow
