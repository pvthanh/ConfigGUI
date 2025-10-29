# Windows Build Guide for ConfigGUI

This guide provides step-by-step instructions to build ConfigGUI on Windows for both Qt GUI and HTML server applications.

## Prerequisites

Install these tools before building:

### 1. CMake
```powershell
winget install -e --id Kitware.CMake
```

### 2. Git
```powershell
winget install -e --id Git.Git
```

### 3. Visual Studio 2022 Build Tools with C++ Workload
```powershell
winget install -e --id Microsoft.VisualStudio.2022.BuildTools --override "--add Microsoft.VisualStudio.Workload.VCTools --passive --norestart"
```

### 4. vcpkg (C++ Package Manager)
```powershell
# Clone vcpkg
git clone https://github.com/microsoft/vcpkg C:\vcpkg

# Bootstrap vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

## Build Instructions

### Option 1: Using the Build Script (Recommended)

From the repository root:

```powershell
# Restart PowerShell after installing tools (to refresh PATH)
powershell -ExecutionPolicy Bypass -File .\tools\scripts\build-windows.ps1 -Generator VS -BuildType Release -VcpkgRoot C:\vcpkg
```

This builds both Qt app and HTML server in one command.

### Option 2: Manual CMake Commands

```powershell
# Configure
cmake -S . -B build-windows -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DCMAKE_BUILD_TYPE=Release `
  -DBUILD_QT_APP=ON `
  -DBUILD_HTML_SERVER=ON

# Build
cmake --build build-windows --config Release --parallel
```

## Build Outputs

After a successful build, you'll find:

- **Qt Application**: `build-windows\src\qt\Release\ConfigGUI.exe` (440 KB)
- **HTML Server**: `build-windows\src\html\Release\ConfigGUIServer.exe` (494 KB)

## Deploy Qt Application

Before running the Qt app, you need to deploy Qt plugins and DLLs:

```powershell
# Run the deployment script
powershell -ExecutionPolicy Bypass -File .\tools\scripts\deploy-qt-windows.ps1
```

This script will:
- Copy Qt platform plugins (qwindows.dll) to `platforms/` subdirectory
- Copy Qt style plugins to `styles/` subdirectory
- Copy all required Qt and runtime DLLs to the executable directory
- Create `qt.conf` to tell Qt where to find plugins

## Running the Applications

### Qt GUI Application

After deploying (see above), simply run:

```powershell
.\build-windows\src\qt\Release\ConfigGUI.exe
```

**Features:**
- Load schema files (JSON Schema)
- Generate forms automatically
- Edit and save configurations (JSON, INI formats)

### HTML Server Application

```powershell
# Run the server
.\build-windows\src\html\Release\ConfigGUIServer.exe
```

Then open your browser to: http://localhost:8080

**Features:**
- Web-based configuration interface
- No Qt dependency (lightweight)
- REST API for configuration management

## Windows-Specific Build Fixes Applied

The following changes were made to ensure Windows compatibility:

1. **Qt6 API Updates**: 
   - Replaced deprecated `QCheckBox::stateChanged` with `checkStateChanged()`
   
2. **YAML Include Path**:
   - Changed from `#include <yaml.h>` to `#include <yaml-cpp/yaml.h>`
   
3. **Windows CRT Security**:
   - Used `localtime_s()` instead of `localtime()` on Windows
   
4. **Unix-Specific Headers**:
   - Wrapped `<unistd.h>` and `<fcntl.h>` includes with `#ifndef _WIN32`
   
5. **Variable Name Conflicts**:
   - Renamed local variables to avoid hiding `QWidget::data` member
   
6. **CMake Library Targets**:
   - Updated to use namespaced targets: `nlohmann_json_schema_validator::validator` and `yaml-cpp::yaml-cpp`

## Troubleshooting

### Qt Platform Plugin Error

If you get the error:
```
qt.qpa.plugin: Could not find the Qt platform plugin "windows" in ""
This application failed to start because no Qt platform plugin could be initialized.
```

**Solution**: Run the deployment script to copy Qt plugins:
```powershell
powershell -ExecutionPolicy Bypass -File .\tools\scripts\deploy-qt-windows.ps1
```

This creates the required `platforms/` directory with `qwindows.dll` and the `qt.conf` configuration file.

### Missing DLLs When Running

After running the deployment script, all required DLLs should be in the executable directory. If you still get missing DLL errors, verify the deployment completed successfully.

To manually check which DLLs are present:
```powershell
Get-ChildItem .\build-windows\src\qt\Release\*.dll | Select-Object Name
```

### CMake Not Found

Restart your PowerShell session after installing CMake, or use the full path:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" --version
```

### vcpkg Package Install Failures

If vcpkg fails to install packages, try:

```powershell
# Clean vcpkg cache
Remove-Item -Recurse -Force C:\vcpkg\buildtrees
Remove-Item -Recurse -Force C:\vcpkg\packages

# Rebuild
cmake --build build-windows --config Release --parallel
```

## Dependencies Managed by vcpkg

The `vcpkg.json` manifest automatically installs:

- nlohmann-json (JSON library)
- yaml-cpp (YAML parser)
- json-schema-validator (JSON Schema validation)
- gtest (unit testing)
- qtbase (Qt Core, Gui, Widgets)
- qttools (Qt development tools)

## Build Time

- First build (with vcpkg package downloads): ~10-15 minutes
- Incremental builds: ~1-2 minutes

## Disk Space

- Build artifacts: ~1.5 GB
- vcpkg installed packages: ~2 GB
- Total: ~3.5 GB

## Next Steps

- Run tests: `ctest --test-dir build-windows --config Release --output-on-failure`
- Package Qt app: Use `windeployqt` to create a redistributable folder
- Deploy HTML server: Copy `ConfigGUIServer.exe` and required DLLs to target machine
