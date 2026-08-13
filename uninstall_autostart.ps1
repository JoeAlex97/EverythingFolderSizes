# Script para desinstalar la carga automatica de FolderSizeExt
$taskName = "EverythingFolderSizeExtension"

Write-Host "Desinstalando FolderSizeExt..." -ForegroundColor Cyan

Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue

Write-Host "Reiniciando el Explorador de Windows..." -ForegroundColor Yellow
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 1

if (-not (Get-Process -Name explorer -ErrorAction SilentlyContinue)) {
    Start-Process explorer.exe
}

Write-Host "`n¡DESINSTALACION COMPLETADA!" -ForegroundColor Green
Write-Host "Se ha eliminado el inicio automatico y se limpio el Explorador." -ForegroundColor Yellow
