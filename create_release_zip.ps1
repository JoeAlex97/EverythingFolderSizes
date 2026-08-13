# Script para empaquetar el archivo ZIP de Release para GitHub Releases
$repoDir = Get-Location
$buildDir = Join-Path $repoDir "build"
$distDir = Join-Path $repoDir "release_dist"

if (-not (Test-Path $buildDir)) {
    Write-Host "[ERROR] La carpeta 'build' no existe. Ejecuta build.bat primero." -ForegroundColor Red
    exit
}

# Crear directorio limpio de distribucion
if (Test-Path $distDir) { Remove-Item $distDir -Recurse -Force }
New-Item -ItemType Directory -Path $distDir | Out-Null

# Asegurar que dbghelp.dll y symsrv.dll esten copiadas en build
$dbghelpSrc = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\TestWindow\VsTest\x64\dbghelp.dll"
$symsrvSrc  = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\Remote Debugger\x64\symsrv.dll"

if (Test-Path $dbghelpSrc) { Copy-Item $dbghelpSrc (Join-Path $buildDir "dbghelp.dll") -Force -ErrorAction SilentlyContinue }
if (Test-Path $symsrvSrc)  { Copy-Item $symsrvSrc  (Join-Path $buildDir "symsrv.dll")  -Force -ErrorAction SilentlyContinue }

# Copiar UNICAMENTE los 7 archivos necesarios para el usuario final
Copy-Item (Join-Path $buildDir "FolderSizeExt.dll") $distDir -Force
Copy-Item (Join-Path $buildDir "dbghelp.dll") $distDir -Force
Copy-Item (Join-Path $buildDir "symsrv.dll") $distDir -Force

$readmeSrc = Join-Path $repoDir "README_Release.txt"
if (Test-Path $readmeSrc) {
    Copy-Item $readmeSrc (Join-Path $distDir "README.txt") -Force
}

Copy-Item (Join-Path $repoDir "inject_dll.ps1") $distDir -Force
Copy-Item (Join-Path $repoDir "install_autostart.ps1") $distDir -Force
Copy-Item (Join-Path $repoDir "uninstall_autostart.ps1") $distDir -Force

$zipPath = Join-Path $repoDir "EverythingFolderSizes-v1.0.0-x64.zip"
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }

# Crear el ZIP comprimido limpio
Compress-Archive -Path "$distDir\*" -DestinationPath $zipPath -Force
Remove-Item $distDir -Recurse -Force

Write-Host "ZIP de Release generado exitosamente en: $zipPath" -ForegroundColor Green
