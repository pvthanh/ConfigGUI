# Build Instructions

## Quick Start

### Prerequisites

- **CMake**: 3.20 or later
- **C++ Compiler**: GCC 11+, Clang 14+, or MSVC 2019+
- **C++ Standard**: C++17
- **Git**: For cloning dependencies

### Dependencies

#### Core Dependencies
```bash
# Debian/Ubuntu
sudo apt-get install libyaml-dev nlohmann-json3-dev
# Or use system packages
sudo apt-get install nlohmann-json3-dev libnlohmann-json-schema-validator-dev libyaml-dev

# Fedora/RHEL
sudo dnf install yaml-devel nlohmann_json-devel nlohmann_json_schema_validator-devel

# macOS (Homebrew)
brew install yaml-cpp nlohmann-json
```

#### Qt Development (if building Qt app)
```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-tools-dev libqt6core6 libqt6gui6 libqt6widgets6

# macOS (Homebrew)
brew install qt@6

# Fedora/RHEL
sudo dnf install qt6-qtbase-devel qt6-qttools-devel
```

#### Build Tools
```bash
# Debian/Ubuntu
sudo apt-get install build-essential cmake ninja-build

# Fedora/RHEL
sudo dnf install gcc-c++ cmake ninja-build

# macOS
xcode-select --install
brew install cmake ninja
```

## Build Configurations

### 1. Dual-Target Build (Both Qt App + HTML Server)

```bash
cd configGui
mkdir -p build && cd build

cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_QT_APP=ON \
  -DBUILD_HTML_SERVER=ON \
  -G Ninja \
  ..

cmake --build . --config Release

# Verify build
./ConfigGUI --help          # Qt application
./ConfigGUIServer --help    # Web server (Phase 3)
```

### 2. Qt-Only Build

```bash
cd configGui
mkdir -p build && cd build

cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_QT_APP=ON \
  -DBUILD_HTML_SERVER=OFF \
  -G Ninja \
  ..

cmake --build . --config Release --target ConfigGUI

# Run Qt application
./ConfigGUI
```

### 3. HTML Server-Only Build (No Qt Required)

```bash
cd configGui
mkdir -p build && cd build

cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_QT_APP=OFF \
  -DBUILD_HTML_SERVER=ON \
  -G Ninja \
  ..

cmake --build . --config Release --target ConfigGUIServer

# Run web server
./ConfigGUIServer --port 8080
# Access at http://localhost:8080
```

## Build Options

### Main Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_QT_APP` | ON | Build Qt GUI application |
| `BUILD_HTML_SERVER` | OFF | Build HTML web server |
| `CMAKE_BUILD_TYPE` | Release | Debug, Release, RelWithDebInfo, MinSizeRel |

### Common Options

```bash
# Debug build with symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Release with debug symbols
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# Minimal size release
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..

# Use specific compiler
cmake -DCMAKE_CXX_COMPILER=clang++ ..

# Parallel build with Ninja
cmake -G Ninja ..
cmake --build . --parallel 4

# Verbose output
cmake --build . --verbose
```

## Building

### Using Ninja (Recommended)

```bash
cmake --build . --config Release --parallel 4
```

### Using Make

```bash
make -j4
```

### Building Specific Target

```bash
# Build only ConfigGUI (Qt app)
cmake --build . --target ConfigGUI

# Build only core library
cmake --build . --target ConfigGUICore

# Build only tests
cmake --build . --target test_unit
```

## Testing

### Run Unit Tests

```bash
# Build tests
cmake --build . --target test_unit

# Run tests
ctest --output-on-failure --parallel 4

# Or directly
./test_unit
```

### Run Integration Tests

```bash
# Build integration tests
cmake --build . --target test_integration

# Run tests
./test_integration
```

### Test Coverage

```bash
# Build with coverage
cmake -DCMAKE_CXX_FLAGS="--coverage" -DCMAKE_EXE_LINKER_FLAGS="--coverage" ..
cmake --build .

# Generate report
ctest --output-on-failure
```

## Installation

### Install Binaries

```bash
cmake --install . --config Release --prefix /usr/local

# Verify installation
which ConfigGUI
which ConfigGUIServer
```

### Custom Installation Path

```bash
cmake --install . --prefix ~/opt/configgui
export PATH=~/opt/configgui/bin:$PATH
```

## Code Quality Tools

### Static Analysis with clang-tidy

```bash
# Build clang-tidy target
cmake --build . --target clang-tidy

# Or run directly
clang-tidy src/**/*.cpp -- -std=c++17 -Isrc
```

### Format Code with clang-format

```bash
# Check formatting
clang-format --dry-run src/**/*.{cpp,h}

# Format code
clang-format -i src/**/*.{cpp,h}
```

### Enable Strict Compilation

```bash
cmake \
  -DCMAKE_CXX_FLAGS="-Wall -Wextra -Werror -Wpedantic" \
  ..
```

## Troubleshooting

### Qt Not Found

```bash
# Specify Qt path
cmake -DQt6_DIR=/opt/Qt/6.4.2/lib/cmake/Qt6 ..

# Or use Qt environment
source ~/Qt/6.4.2/gcc_64/bin/qt-cmake-standalone-test.env
cmake ..
```

### Missing Dependencies

```bash
# Check what's missing
cmake .. 2>&1 | grep -i "not found"

# Install missing package, then reconfigure
rm -rf build
mkdir build && cd build
cmake ..
```

### YAML-cpp Not Found

```bash
# Try with specific path
cmake -Dyaml-cpp_DIR=/usr/lib/cmake/yaml-cpp ..

# Or find and link manually
find /usr -name "*yaml-cpp*" 2>/dev/null
```

### Permission Denied on Build

```bash
# Check directory permissions
ls -la build/
chmod -R u+w build/

# Or clean and rebuild
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```

## Development Workflow

### Setup Development Build

```bash
# Create debug build directory
mkdir -p build-dev
cd build-dev

# Configure with debug symbols and tests
cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_QT_APP=ON \
  -DBUILD_HTML_SERVER=ON \
  ..

# Build
cmake --build .

# Run tests
ctest --output-on-failure
```

### Incremental Development

```bash
# After code changes
cmake --build build-dev --target ConfigGUI

# Or rebuild everything
cmake --build build-dev
```

### Clean Build

```bash
# Option 1: Remove build directory
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .

# Option 2: Clean target
cmake --build build --target clean
```

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y qt6-base-dev libyaml-dev nlohmann-json3-dev
      
      - name: Configure
        run: |
          mkdir -p build && cd build
          cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON ..
      
      - name: Build
        run: cmake --build build --parallel 4
      
      - name: Test
        run: ctest --output-on-failure --test-dir build
```

### Docker Build

```dockerfile
# Multi-stage build: compile
FROM ubuntu:22.04 as builder

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build \
    qt6-base-dev libyaml-dev nlohmann-json3-dev

COPY . /app
WORKDIR /app

RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_QT_APP=OFF \
          -DBUILD_HTML_SERVER=ON \
          -G Ninja .. && \
    cmake --build . --config Release

# Runtime image
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libc6 libyaml-0 libstdc++6

COPY --from=builder /app/build/ConfigGUIServer /usr/local/bin/

EXPOSE 8080
ENTRYPOINT ["ConfigGUIServer", "--port", "8080"]
```

## Environment Variables

```bash
# Set Qt prefix
export Qt6_DIR=/opt/Qt/6.4.2/lib/cmake/Qt6

# Use specific compiler
export CXX=clang++
export CC=clang

# Parallel build jobs
export MAKEFLAGS=-j4
```

## Advanced Configuration

### Custom Installation Paths

```bash
cmake \
  -DCMAKE_INSTALL_PREFIX=/opt/configgui \
  -DLIB_INSTALL_DIR=/opt/configgui/lib \
  -DBIN_INSTALL_DIR=/opt/configgui/bin \
  ..
```

### Link-Time Optimization (LTO)

```bash
cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-flto" \
  ..
```

### AddressSanitizer (For Debugging)

```bash
cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address" \
  ..
```

## Support

For build issues:
1. Check CMake output for specific error messages
2. Verify all dependencies are installed
3. Try a clean build: `rm -rf build && mkdir build && cd build && cmake ..`
4. Check platform-specific requirements above
5. Consult ARCHITECTURE.md for module dependencies

---

*Generated during Phase 1 Implementation - T019*
*Last Updated: 2025-10*
