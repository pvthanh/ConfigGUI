# SPDX-License-Identifier: MIT
# Unit tests for core module - CMakeLists.txt

# Enable testing
enable_testing()

# Core module unit test files
set(CORE_TEST_SOURCES
    test_schema_loader.cpp
    test_schema_validator.cpp
    test_json_io.cpp
    test_yaml_io.cpp
    test_ini_parser.cpp
    # test_configuration.cpp  # TODO: Fix API compatibility (uses camelCase methods)
)

# Create executable for each test file
foreach(test_file ${CORE_TEST_SOURCES})
    get_filename_component(test_name ${test_file} NAME_WE)
    
    add_executable(${test_name} ${test_file})
    
    # Link against Google Test
    target_link_libraries(${test_name} PRIVATE
        GTest::gtest_main
        GTest::gtest
    )
    
    # Link against core library
    target_link_libraries(${test_name} PRIVATE
        ConfigGUICore
    )
    
    # Link against external dependencies
    target_link_libraries(${test_name} PRIVATE
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator
        yaml-cpp
    )
    
    # Set include directories
    target_include_directories(${test_name} PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )
    
    # Add test to CMake's test registry
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # Set test properties for better output
    set_tests_properties(${test_name} PROPERTIES
        LABELS "core"
        TIMEOUT 30
    )
    
endforeach()

# Optional: Create a combined test runner target
add_custom_target(run-core-tests
    COMMAND ${CMAKE_CTEST_COMMAND} -V -L core
    DEPENDS ${CORE_TEST_SOURCES}
    COMMENT "Running all core module unit tests..."
)

# Optional: Add code coverage targets (if supported)
if(ENABLE_CODE_COVERAGE)
    # Coverage target for core tests
    add_custom_target(core-coverage
        COMMAND ${CMAKE_CTEST_COMMAND} -L core --output-on-failure
        COMMAND gcovr --root ${PROJECT_SOURCE_DIR} --print-summary
        COMMENT "Generating coverage report for core module tests..."
    )
endif()
