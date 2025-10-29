param(
  [ValidateSet("Ninja","VS")]
  [string]$Generator = "VS",
  [ValidateSet("Debug","Release")]
  [string]$BuildType = "Release",
  [string]$VcpkgRoot = "C:\\vcpkg",
  [string]$CMakePath = "",
  [switch]$QtAndServer
)

# Resolve CMake executable
$cmakeExe = $null
if ($CMakePath -and (Test-Path $CMakePath)) {
  $cmakeExe = $CMakePath
} elseif (Get-Command cmake -ErrorAction SilentlyContinue) {
  $cmakeExe = (Get-Command cmake).Source
} elseif (Test-Path "C:\\Program Files\\CMake\\bin\\cmake.exe") {
  $cmakeExe = "C:\\Program Files\\CMake\\bin\\cmake.exe"
} elseif (Test-Path "C:\\Program Files (x86)\\CMake\\bin\\cmake.exe") {
  $cmakeExe = "C:\\Program Files (x86)\\CMake\\bin\\cmake.exe"
} else {
  Write-Error "CMake not found. Install with: winget install -e --id Kitware.CMake or pass -CMakePath."
  exit 1
}

# Resolve toolchain file
$toolchain = Join-Path $VcpkgRoot "scripts/buildsystems/vcpkg.cmake"
if (-not (Test-Path $toolchain)) {
  Write-Error "vcpkg toolchain not found at $toolchain. Set -VcpkgRoot to your vcpkg directory."
  exit 1
}

# Select generator
$cmakeGen = if ($Generator -eq "Ninja") { @('-G','Ninja') } else { @('-G','Visual Studio 17 2022','-A','x64') }

# Build flags
$flags = @(
  "-DCMAKE_TOOLCHAIN_FILE=$toolchain",
  "-DVCPKG_TARGET_TRIPLET=x64-windows",
  "-DCMAKE_BUILD_TYPE=$BuildType"
)

# Default to building both unless explicitly turned off
$flags += "-DBUILD_QT_APP=ON"
$flags += "-DBUILD_HTML_SERVER=ON"

# Build directory
$buildDir = Join-Path $PSScriptRoot "..\..\build-windows"
try {
  $resolved = Resolve-Path -LiteralPath $buildDir -ErrorAction Stop
  $buildDir = $resolved.Path
} catch { }
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

Push-Location $buildDir

# Configure
$sourceDir = (Resolve-Path "$PSScriptRoot\..\..").Path
$configureArgs = @()
$configureArgs += $cmakeGen
$configureArgs += @('-DCMAKE_TOOLCHAIN_FILE=' + $toolchain)
$configureArgs += @('-DVCPKG_TARGET_TRIPLET=x64-windows')
$configureArgs += @('-DCMAKE_BUILD_TYPE=' + $BuildType)
$configureArgs += @('-DBUILD_QT_APP=ON','-DBUILD_HTML_SERVER=ON')
$configureArgs += @('-S', $sourceDir, '-B', $buildDir)
Write-Host "Configuring: `"$cmakeExe`" $($configureArgs -join ' ')"
& $cmakeExe @configureArgs
if ($LASTEXITCODE -ne 0) { Pop-Location; exit $LASTEXITCODE }

# Build
$buildArgs = @('--build', $buildDir)
if ($Generator -eq 'VS') { $buildArgs += @('--config', $BuildType) }
$buildArgs += '--parallel'
Write-Host "Building: `"$cmakeExe`" $($buildArgs -join ' ')"
& $cmakeExe @buildArgs
if ($LASTEXITCODE -ne 0) { Pop-Location; exit $LASTEXITCODE }

# Helpful PATH hint for Qt DLLs (vcpkg bin)
$binPath = Join-Path $VcpkgRoot "installed/x64-windows/bin"
$hint = "If running executables fails due to missing DLLs, run:`n`$env:Path=`"$binPath;$env:Path`""
Write-Host $hint

# Print artifacts
if ($Generator -eq "VS") {
  $qtExe = Join-Path $buildDir "src/qt/$BuildType/ConfigGUI.exe"
  $srvExe = Join-Path $buildDir "src/html/$BuildType/ConfigGUIServer.exe"
} else {
  $qtExe = Join-Path $buildDir "src/qt/ConfigGUI.exe"
  $srvExe = Join-Path $buildDir "src/html/ConfigGUIServer.exe"
}

Write-Host "Qt app:    $qtExe"
Write-Host "HTML app:  $srvExe"

Pop-Location
