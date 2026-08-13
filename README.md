# Everything Folder Sizes for Windows Explorer

> 🤖 **Author Notice**: This project and its source code were generated with AI assistance.  
> **Signed**: *Gemini 3.6 Flash (Antigravity by Google DeepMind)*.

---

## 📌 Description

**Everything Folder Sizes for Windows Explorer** is a standalone C++ extension that integrates instant folder size calculation directly into Windows File Explorer's Details view (Windows 10/11), powered by the in-memory index of **[Everything (Voidtools)](https://www.voidtools.com/)**.

### Key Features
* **Displays actual folder sizes** in Windows Explorer's "Size" column.
* **Instant response** (0.001s) by querying pre-indexed data from Everything.
* **100% Native & Standalone**: Operates directly inside the `explorer.exe` process without requiring third-party frameworks like Windhawk.
* **Native Explorer Rendering**: Delivers exact raw byte values directly to Windows, letting File Explorer handle formatting natively.

---

### 💡 Credits & Inspiration

This project was inspired by the Windhawk mod **[Better file sizes in Explorer details](https://windhawk.net/mods/explorer-details-better-file-sizes)**. The goal of **Everything Folder Sizes** is to provide a lightweight, independent C++ extension focused specifically on bringing folder sizes to File Explorer, without needing to install the Windhawk framework.

---

## 📖 Usage & Installation Guide

Follow these steps to set up Everything and install the extension:

### Step 1: Download Everything 1.5.0.1418b Beta (Beta)
Download and install **[Everything (Voidtools)](https://www.voidtools.com/)**.  
> ⚠️ **Important**: You must use **Everything 1.5.0.1418b Beta or any software version 1.5.0.0 or higher** to enable the required IPC pipe features.

![Everything Version 1.5.0.0+](<assets/1. everything_choose_beta_version_1.5.0.0+.png>)

---

### Step 2: Configure Everything General Options
Open Everything, navigate to **Tools $\rightarrow$ Options $\rightarrow$ General**, and ensure your settings match the following:
* ✅ **Start Everything on system startup**
* ✅ **Everything Service** *(Required to index files without elevating the main process)*
* ❌ **Run indexing process as administrator**
* ❌ **Run as administrator** *(Must be unchecked so File Explorer can query Everything via IPC without permission errors)*

![Everything General Options](<assets/2. everything_general_options.png>)

---

### Step 3: Enable Folder Size Indexing
In Everything options, navigate to **Tools $\rightarrow$ Options $\rightarrow$ Indexes**:
* ✅ Check **Index folder size** to enable instant folder size calculation.

![Enable Index Folder Size](<assets/3. everything_enable_index_folder_size.png>)

---

### Step 4: Installing the Extension

1. Download the latest release `.zip` package from the **Releases** section and extract it to a permanent folder on your PC (e.g., `C:\Program Files\EverythingFolderSizes\`).
2. Open PowerShell as Administrator and run the installation script:
   ```powershell
   sudo powershell -ExecutionPolicy Bypass -File .\install_autostart.ps1
   ```
   *(Or run `powershell -ExecutionPolicy Bypass -File .\install_autostart.ps1` in an elevated terminal).*
3. Open Windows File Explorer in **Details view** (`Ctrl + Shift + 6`) and press **`F5`** to refresh. Folder sizes will now load instantly!

![Working Demonstration](<assets/4. working_well.gif>)

> [!NOTE]
> **Dynamic Size Units (GB, MB, KB)**: By default, Windows File Explorer displays file sizes exclusively in KB. If you notice folder sizes dynamically formatted in **GB, MB, or KB** in the demonstration, this is powered by a feature available starting in **Windows 11 (KB5121003 / Build 26100+)**, enabled via **[ViVeTool](https://github.com/thebookisclosed/ViVe)** using the following command in an elevated prompt:
> ```powershell
> sudo .\ViVeTool.exe /enable /id:61014711
> ```
> For more information, see the **[Windows Latest Report](https://www.windowslatest.com/2026/08/11/windows-11-kb5121003-out-with-faster-performance-for-apps-search-handling-types-and-more-changes-direct-download-links-msu/#:~:text=vivetool%20%2Fenable%20%2Fid%3A61014711.%20This%20will%20turn%20on,the%20correct%20feature%20ID.%20Another%20notable%20File)**.

---

### 🛑 Uninstallation Instructions

If you ever want to cleanly remove the extension:
1. Open PowerShell as Administrator and run:
   ```powershell
   sudo powershell -ExecutionPolicy Bypass -File .\uninstall_autostart.ps1
   ```
2. Once complete, you can safely delete the extension folder.

---

## 🛠️ Building from Source

### Prerequisites
1. **C++ Compiler**: [Visual Studio 2022](https://visualstudio.microsoft.com/) or **Visual Studio Build Tools 2022** (with the *"Desktop development with C++"* workload).
2. **Editor**: [Visual Studio Code](https://code.visualstudio.com/) or any command prompt.
3. **CMake** (v3.15 or higher).

### Build Steps

1. **Clone the repository with submodules**:
   ```bash
   git clone --recursive https://github.com/JoeAlex97/EverythingFolderSizes.git
   cd EverythingFolderSizes
   ```

2. **Build using the automated script (Recommended)**:
   Open a command prompt and run:
   ```cmd
   build.bat
   ```
   This will compile `FolderSizeExt.dll` into the `build/` folder.

3. **Build via VS Code**:
   * Open the project folder in VS Code.
   * Press `Ctrl + Shift + B` to execute the preconfigured build task.
