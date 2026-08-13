========================================================================
 Everything Folder Sizes for Windows Explorer - Release v1.0.0 (x64)
========================================================================

PREREQUISITES:
1. Everything (Voidtools) installed and running in the background.
2. In Everything -> Tools -> Options -> General:
   - CHECK "Everything Service"
   - UNCHECK "Run as administrator"
3. In Everything -> Tools -> Options -> Indexes:
   - CHECK "Index folder size"

INSTALLATION INSTRUCTIONS:
1. Extract this package into a permanent folder on your PC 
   (e.g., C:\Program Files\FolderSizeExt\).
2. Right-click "install_autostart.ps1" and choose "Run with PowerShell"
   (or open PowerShell as Administrator in that folder and run:
    powershell -ExecutionPolicy Bypass -File .\install_autostart.ps1).
3. Open Windows File Explorer, switch to "Details" view, and press F5.
   Folder sizes will now appear instantly!

UNINSTALLATION:
Run "uninstall_autostart.ps1" with PowerShell as Administrator,
then you can safely delete the folder.
