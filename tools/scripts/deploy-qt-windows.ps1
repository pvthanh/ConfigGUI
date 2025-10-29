# Deploy Qt Application with Required DLLs and Plugins
# This script copies all necessary Qt dependencies to the executable directory

param(
    [string]$BuildDir = "build-windows",
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"

# Paths
$repoRoot = $PSScriptRoot | Split-Path | Split-Path
$exeDir = Join-Path $repoRoot "$BuildDir\src\qt\$Config"
$vcpkgDir = Join-Path $repoRoot "$BuildDir\vcpkg_installed\x64-windows"
$qt6Dir = Join-Path $vcpkgDir "Qt6"

Write-Host "Deploying Qt application..." -ForegroundColor Cyan
Write-Host "Executable directory: $exeDir"

if (-not (Test-Path $exeDir)) {
    Write-Error "Executable directory not found: $exeDir"
    exit 1
}

if (-not (Test-Path (Join-Path $exeDir "ConfigGUI.exe"))) {
    Write-Error "ConfigGUI.exe not found in: $exeDir"
    exit 1
}

# Create platforms plugin directory
$platformsDir = Join-Path $exeDir "platforms"
if (-not (Test-Path $platformsDir)) {
    New-Item -ItemType Directory -Path $platformsDir | Out-Null
    Write-Host "Created platforms directory"
}

# Copy Qt platform plugin
$qwindowsSrc = Join-Path $qt6Dir "plugins\platforms\qwindows.dll"
if (Test-Path $qwindowsSrc) {
    Copy-Item $qwindowsSrc -Destination $platformsDir -Force
    Write-Host "Copied qwindows.dll" -ForegroundColor Green
} else {
    Write-Warning "qwindows.dll not found at: $qwindowsSrc"
}

# Copy styles plugin if exists
$stylesDir = Join-Path $exeDir "styles"
$qmodernStyleSrc = Join-Path $qt6Dir "plugins\styles\qmodernwindowsstyle.dll"
if (Test-Path $qmodernStyleSrc) {
    if (-not (Test-Path $stylesDir)) {
        New-Item -ItemType Directory -Path $stylesDir | Out-Null
    }
    Copy-Item $qmodernStyleSrc -Destination $stylesDir -Force
    Write-Host "Copied qmodernwindowsstyle.dll" -ForegroundColor Green
}

# Copy required Qt DLLs from vcpkg bin directory
$binDir = Join-Path $vcpkgDir "bin"
$requiredDlls = @(
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Widgets.dll"
)

foreach ($dll in $requiredDlls) {
    $srcDll = Join-Path $binDir $dll
    if (Test-Path $srcDll) {
        Copy-Item $srcDll -Destination $exeDir -Force
        Write-Host "Copied $dll" -ForegroundColor Green
    } else {
        Write-Warning "$dll not found at: $srcDll"
    }
}

# Copy other runtime dependencies
$otherDlls = Get-ChildItem -Path $binDir -Filter "*.dll" | Where-Object {
    $_.Name -match "(icudt|icuin|icuuc|pcre2|zlib|zstd|brotli|harfbuzz|freetype|png16|jpeg|double-conversion|bz2)"
}

foreach ($dll in $otherDlls) {
    Copy-Item $dll.FullName -Destination $exeDir -Force -ErrorAction SilentlyContinue
}

Write-Host "Copied runtime dependencies" -ForegroundColor Green

# Create qt.conf to tell Qt where to find plugins
$qtConfPath = Join-Path $exeDir "qt.conf"
$qtConfContent = "[Paths]`nPlugins = ."

Set-Content -Path $qtConfPath -Value $qtConfContent -Encoding UTF8
Write-Host "Created qt.conf" -ForegroundColor Green

Write-Host ""
Write-Host "Deployment complete!" -ForegroundColor Green
Write-Host "Run: .\$BuildDir\src\qt\$Config\ConfigGUI.exe" -ForegroundColor Cyan
