cmake_minimum_required(VERSION 3.20)

project(ConfigGUI
    VERSION 1.0.0
    DESCRIPTION "Schema-Driven Configuration GUI"
    LANGUAGES CXX
)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build Options - Dual-Target Support (T001)
option(BUILD_QT_APP "Build Qt application" ON)
option(BUILD_HTML_SERVER "Build HTML server application" OFF)

if(NOT BUILD_QT_APP AND NOT BUILD_HTML_SERVER)
    message(FATAL_ERROR "At least one of BUILD_QT_APP or BUILD_HTML_SERVER must be ON")
endif()

# Qt Configuration (only needed if building Qt app)
if(BUILD_QT_APP)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

# CMake FetchContent for external dependencies
include(FetchContent)

# Find Qt6 (only required if building Qt app)
if(BUILD_QT_APP)
    find_package(Qt6 COMPONENTS 
        Core 
        Gui 
        Widgets 
        Test
        REQUIRED
    )
endif()

# Find nlohmann_json
find_package(nlohmann_json 3.11 REQUIRED)

# Find json-schema-validator
find_package(nlohmann_json_schema_validator REQUIRED)

# Find yaml-cpp
find_package(yaml-cpp REQUIRED)

# Fetch mINI library (header-only, for INI format support)
# Used by core library for INI serialization
FetchContent_Declare(mINI
    GIT_REPOSITORY https://github.com/pulzed/mINI.git
    GIT_TAG master
)
FetchContent_GetProperties(mINI)
if(NOT mini_POPULATED)
    FetchContent_Populate(mINI)
    # mINI is header-only, just add include directory
    set(mINI_INCLUDE_DIRS "${mini_SOURCE_DIR}/include")
endif()

# Fetch cpp-httplib (header-only, C++17)
# Only used by HTML server build (Phase 3)
FetchContent_Declare(cpp-httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.0
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Fetch cpp-httplib content (for HTML server module)
if(BUILD_HTML_SERVER)
    FetchContent_MakeAvailable(cpp-httplib)
endif()

# Add source subdirectories (modular organization)
add_subdirectory(src)

# Main application executable (Qt-only)
# (Defined in src/qt/CMakeLists.txt)

# MISRA C++ Compiler Flags (for Qt app if built)
# (Defined in src/core/CMakeLists.txt and src/qt/CMakeLists.txt)

# Enable testing
enable_testing()

# Find Google Test for unit testing
find_package(GTest REQUIRED)

# Unit Tests (Qt-specific and core tests)
add_subdirectory(tests)

# Integration Tests (Qt-specific)
# (Defined in tests/integration)

# clang-tidy target
if(CMAKE_CXX_CLANG_TIDY)
    add_custom_target(clang-tidy
        COMMAND clang-tidy src/**/*.cpp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# Print configuration
message(STATUS "ConfigGUI Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Qt App: ${BUILD_QT_APP}")
message(STATUS "  Build HTML Server: ${BUILD_HTML_SERVER}")
if(BUILD_QT_APP)
    message(STATUS "  Qt Version: ${Qt6_VERSION}")
endif()
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
