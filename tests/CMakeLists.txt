# CMakeLists.txt for Tests

cmake_minimum_required(VERSION 3.20)

# Include Google Test
find_package(GTest REQUIRED)

# Test executable
enable_testing()

# Add core unit tests
add_subdirectory(unit/core)

# Add HTML server unit tests
add_subdirectory(unit/html)

# Add integration tests
add_subdirectory(integration)

