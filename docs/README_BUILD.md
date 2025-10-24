# ConfigGUI - Build, Installation & Release Guide

**Project**: ConfigGUI - Schema-Driven Configuration UI Framework  
**Version**: 1.0.0  
**Status**: Release Ready  
**Date**: 2025-10-20  

---

## Table of Contents
1. [Quick Start](#quick-start)
2. [System Requirements](#system-requirements)
3. [Building from Source](#building-from-source)
4. [Installation](#installation)
5. [Verification](#verification)
6. [Docker Deployment](#docker-deployment)
7. [CI/CD Integration](#cicd-integration)
8. [Troubleshooting](#troubleshooting)

---

## Quick Start

### Linux (Ubuntu/Debian)
```bash
# Clone repository
git clone <repository-url> configgui
cd configgui

# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    cmake \
    g++ \
    qt6-base-dev \
    libyaml-dev \
    nlohmann-json3-dev

# Build
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Run tests
ctest --output-on-failure

# Run application
./src/configgui
```

### macOS
```bash
# Install Homebrew dependencies
brew install cmake qt6 libyaml

# Build
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Run
./src/configgui
```

### Windows (MSVC)
```bash
# Install Qt 6 and Visual Studio Build Tools
# Install CMake

# Build
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release

# Run
src\Release\configgui.exe
```

---

## System Requirements

### Minimum Requirements
- **CPU**: Intel/AMD x86-64 (1.5 GHz or higher)
- **RAM**: 512 MB
- **Disk**: 100 MB (build), 50 MB (runtime)
- **OS**: Linux, macOS 10.13+, Windows 10+

### Build Requirements
| Component | Minimum | Recommended |
|-----------|---------|-------------|
| CMake | 3.20 | 3.28+ |
| C++ Compiler | C++17 support | GCC 11+, Clang 14+, MSVC 2019+ |
| Qt | 6.2 | 6.5+ |
| libyaml | 0.2.4 | 0.2.5+ |
| nlohmann/json | 3.10 | 3.11+ |

### Runtime Dependencies
- **Qt Core** (libQt6Core.so or .dll)
- **Qt GUI** (libQt6Gui.so or .dll)
- **Qt Widgets** (libQt6Widgets.so or .dll)
- **libyaml** (libyaml.so or .dll)
- **C++ Standard Library** (libstdc++ or libc++)

### Development Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install -y \
    cmake \
    build-essential \
    git \
    qt6-base-dev \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libyaml-dev \
    nlohmann-json3-dev \
    nlohmann-json-schema-validator-dev

# Fedora/RHEL
sudo dnf install -y \
    cmake \
    gcc-c++ \
    git \
    qt6-qtbase-devel \
    libyaml-devel \
    nlohmann-json-devel

# macOS
brew install cmake qt6 libyaml
```

---

## Building from Source

### Step 1: Prepare Build Environment

```bash
# Clone repository
git clone <repository-url> configgui
cd configgui

# Create build directory
mkdir build
cd build
```

### Step 2: Configure with CMake

**Default Release Build**:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

**With Custom Installation Path**:
```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/configgui ..
```

**With Debug Symbols**:
```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_PREFIX_PATH=/path/to/qt6 ..
```

**Full Configuration Example**:
```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/configgui \
      -DCMAKE_PREFIX_PATH=/usr/lib/cmake/Qt6 \
      -DBUILD_SHARED_LIBS=ON \
      ..
```

### Step 3: Build

**Single-threaded Build**:
```bash
cmake --build .
```

**Multi-threaded Build (faster)**:
```bash
cmake --build . --parallel 4
```

**Verbose Output**:
```bash
cmake --build . --verbose
```

**Specific Target**:
```bash
cmake --build . --target configgui
```

### Step 4: Install

**To Default Location**:
```bash
cmake --install .
```

**To Custom Location**:
```bash
cmake --install . --prefix /opt/configgui
```

**Verification**:
```bash
ls -la /opt/configgui/bin/
```

---

## Verification

### Build Verification

**Check Executable**:
```bash
file src/configgui
ldd src/configgui  # See dependencies (Linux)
```

**Run Unit Tests**:
```bash
cd build
ctest --output-on-failure

# or run specific test
./tests/unit/validators/test_type_validator
```

**Test Specific Component**:
```bash
ctest --output-on-failure -R "ValidatorTest"
ctest --output-on-failure -R "JsonReaderWriter"
ctest --output-on-failure -R "Workflow"
```

### Runtime Verification

**Check Linked Libraries**:
```bash
ldd ./src/configgui
# or on macOS
otool -L ./src/configgui
```

**Run Application**:
```bash
./src/configgui --version
./src/configgui --help
```

**Test Configuration File**:
```bash
# Create test config
cat > test_config.json << 'EOF'
{
  "app_name": "TestApp",
  "version": "1.0.0",
  "settings": {
    "debug": false,
    "port": 8080
  }
}
EOF

# Load with configgui
./src/configgui test_config.json
```

---

## Installation

### From Built Artifacts

**Standard Installation**:
```bash
cd build
cmake --install . --prefix /usr/local
```

**System-wide (requires sudo)**:
```bash
sudo cmake --install . --prefix /usr
```

**User-local Installation**:
```bash
cmake --install . --prefix ~/.local
export PATH="$HOME/.local/bin:$PATH"
```

### Verify Installation

```bash
which configgui
configgui --version

# Check library installation
find /usr/local/lib -name "*.so" | grep config
```

### Package Installation (Future)

Once packages are available:
```bash
# Ubuntu/Debian (when available)
sudo apt-get install configgui

# macOS (when available)
brew install configgui

# Or from source with package manager
dpkg -i configgui-1.0.0-amd64.deb
```

---

## Docker Deployment

### Dockerfile

Create `Dockerfile`:
```dockerfile
FROM ubuntu:22.04

# Install build dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    qt6-base-dev \
    libyaml-dev \
    nlohmann-json3-dev \
    nlohmann-json-schema-validator-dev \
    && rm -rf /var/lib/apt/lists/*

# Clone and build
WORKDIR /build
RUN git clone <repository-url> configgui
WORKDIR /build/configgui

RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . && \
    cmake --install . --prefix /opt/configgui

# Runtime image
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libyaml0 \
    && rm -rf /var/lib/apt/lists/*

# Copy from build
COPY --from=0 /opt/configgui /opt/configgui

ENV PATH="/opt/configgui/bin:${PATH}"

WORKDIR /data

ENTRYPOINT ["configgui"]
CMD ["--help"]
```

### Build Docker Image

```bash
docker build -t configgui:1.0.0 .
docker tag configgui:1.0.0 configgui:latest
```

### Run Docker Container

```bash
# Interactive mode
docker run -it configgui:latest

# Mount configuration directory
docker run -it \
    -v $(pwd)/configs:/data \
    configgui:latest /data/config.json

# Detached mode
docker run -d \
    --name configgui-app \
    -v $(pwd)/configs:/data \
    configgui:latest
```

### Docker Compose

Create `docker-compose.yml`:
```yaml
version: '3.8'

services:
  configgui:
    build: .
    image: configgui:1.0.0
    container_name: configgui
    volumes:
      - ./configs:/data
      - ./schemas:/schemas
    environment:
      - DISPLAY=$DISPLAY
    networks:
      - configgui-network

networks:
  configgui-network:
    driver: bridge
```

Run:
```bash
docker-compose up -d
```

---

## CI/CD Integration

### GitHub Actions

Create `.github/workflows/build.yml`:
```yaml
name: Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          cmake g++ \
          qt6-base-dev \
          libyaml-dev \
          nlohmann-json3-dev
    
    - name: Configure CMake
      run: |
        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
    
    - name: Build
      run: |
        cd build
        cmake --build . --parallel 4
    
    - name: Run Tests
      run: |
        cd build
        ctest --output-on-failure
```

### GitLab CI

Create `.gitlab-ci.yml`:
```yaml
stages:
  - build
  - test
  - deploy

variables:
  BUILD_DIR: build

build:ubuntu:
  stage: build
  image: ubuntu:22.04
  before_script:
    - apt-get update
    - apt-get install -y cmake g++ qt6-base-dev libyaml-dev nlohmann-json3-dev
  script:
    - mkdir $BUILD_DIR
    - cd $BUILD_DIR
    - cmake -DCMAKE_BUILD_TYPE=Release ..
    - cmake --build . --parallel 4
  artifacts:
    paths:
      - $BUILD_DIR/src/configgui

test:
  stage: test
  image: ubuntu:22.04
  before_script:
    - apt-get update
    - apt-get install -y cmake g++ qt6-base-dev libyaml-dev nlohmann-json3-dev
  script:
    - mkdir $BUILD_DIR
    - cd $BUILD_DIR
    - cmake -DCMAKE_BUILD_TYPE=Release ..
    - cmake --build . --parallel 4
    - ctest --output-on-failure
```

### Jenkins

Create `Jenkinsfile`:
```groovy
pipeline {
    agent any
    
    environment {
        BUILD_DIR = 'build'
    }
    
    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        
        stage('Setup') {
            steps {
                sh '''
                    apt-get update
                    apt-get install -y cmake g++ qt6-base-dev libyaml-dev nlohmann-json3-dev
                '''
            }
        }
        
        stage('Build') {
            steps {
                sh '''
                    mkdir -p ${BUILD_DIR}
                    cd ${BUILD_DIR}
                    cmake -DCMAKE_BUILD_TYPE=Release ..
                    cmake --build . --parallel 4
                '''
            }
        }
        
        stage('Test') {
            steps {
                sh '''
                    cd ${BUILD_DIR}
                    ctest --output-on-failure
                '''
            }
        }
    }
}
```

---

## Troubleshooting

### Build Issues

**CMake Configuration Fails**
```bash
# Clear CMake cache
rm -rf build/CMakeCache.txt

# Check Qt installation
cmake --find-package-debug-mode Qt6

# Use explicit Qt path
cmake -DCMAKE_PREFIX_PATH=/path/to/qt6 ..
```

**Missing Qt Libraries**
```bash
# Find Qt installation
find /usr -name "Qt6Config.cmake"

# Set explicitly
cmake -DCMAKE_PREFIX_PATH=/usr/lib/cmake/Qt6 ..
```

**Linker Errors**
```bash
# Check dependencies
ldd ./src/configgui

# Build with verbose output
cmake --build . --verbose

# Check compiler warnings
cmake --build . 2>&1 | grep -i warning
```

### Runtime Issues

**Missing Libraries**
```bash
# Check dependencies
ldd ./src/configgui

# Install missing libraries
sudo apt-get install libqt6core6 libqt6gui6 libqt6widgets6

# Set library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

**Qt Display Issues (Headless Server)**
```bash
# Use offscreen platform
QT_QPA_PLATFORM=offscreen ./src/configgui config.json

# Or use virtual display
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99
./src/configgui
```

**Performance Issues**
```bash
# Profile execution
perf record ./src/configgui config.json
perf report

# Memory profiling
valgrind --leak-check=full ./src/configgui config.json
```

---

## Performance Tuning

### Build Performance

**Parallel Build**:
```bash
cmake --build . --parallel $(nproc)
```

**Precompiled Headers** (if enabled):
```cmake
target_precompile_headers(configgui PRIVATE <vector> <string> <map>)
```

**Link-Time Optimization**:
```bash
cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
```

### Runtime Performance

**Release Build**:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

**Optimization Flags**:
```bash
cmake -DCMAKE_CXX_FLAGS="-O3 -march=native" ..
```

---

## Security Considerations

### Source Validation
```bash
# Verify GPG signature (when available)
gpg --verify configgui-1.0.0.tar.gz.asc configgui-1.0.0.tar.gz

# Check file integrity
sha256sum -c CHECKSUMS.txt
```

### Secure Configuration
```bash
# Restrict file permissions
chmod 700 configgui
chmod 600 config.json

# Run with minimal privileges
./configgui config.json --no-root-required
```

---

## Additional Resources

### Documentation
- **README.md** - Project overview
- **SPECIFICATION.md** - Feature specification
- **API_REFERENCE.md** - API documentation
- **TROUBLESHOOTING.md** - Troubleshooting guide (this file)

### Community
- **GitHub Issues**: Report bugs
- **GitHub Discussions**: Ask questions
- **Contributing.md**: Contribution guidelines

### Related Tools
- CMake: https://cmake.org/
- Qt 6: https://www.qt.io/
- nlohmann/json: https://github.com/nlohmann/json
- libyaml: https://pyyaml.org/wiki/LibYAML

---

**Last Updated**: 2025-10-20  
**Version**: 1.0.0  
**Status**: Release Ready

