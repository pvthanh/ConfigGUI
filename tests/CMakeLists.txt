# CMakeLists.txt for Tests

cmake_minimum_required(VERSION 3.20)

# Include Google Test
find_package(GTest REQUIRED)

# Test executable
enable_testing()

# Common test utilities
add_library(test_common INTERFACE)
target_include_directories(test_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/src
)

# Unit Tests - Validators
add_executable(test_validators
    unit/validators/test_type_validator.cpp
    unit/validators/test_range_validator.cpp
    unit/validators/test_pattern_validator.cpp
    unit/validators/test_enum_validator.cpp
    unit/validators/test_required_validator.cpp
)

target_link_libraries(test_validators
    GTest::GTest
    GTest::Main
    test_common
    nlohmann_json::nlohmann_json
)

add_test(NAME Validators COMMAND test_validators)

# Unit Tests - File I/O
add_executable(test_file_io
    unit/io/test_json_reader_writer.cpp
    unit/io/test_yaml_reader_writer.cpp
    unit/io/test_file_io_errors.cpp
)

target_link_libraries(test_file_io
    GTest::GTest
    GTest::Main
    test_common
    nlohmann_json::nlohmann_json
    yaml-cpp
)

add_test(NAME FileIO COMMAND test_file_io)

# Unit Tests - Core Models
add_executable(test_core_models
    unit/core/test_schema.cpp
    unit/core/test_configuration_data.cpp
    unit/core/test_form_state.cpp
    unit/core/test_validation_error.cpp
    unit/core/test_result.cpp
)

target_link_libraries(test_core_models
    GTest::GTest
    GTest::Main
    test_common
    nlohmann_json::nlohmann_json
)

add_test(NAME CoreModels COMMAND test_core_models)

# Unit Tests - Utilities
add_executable(test_utilities
    unit/utils/test_string_utils.cpp
    unit/utils/test_file_utils.cpp
    unit/utils/test_logger.cpp
)

target_link_libraries(test_utilities
    GTest::GTest
    GTest::Main
    test_common
)

add_test(NAME Utilities COMMAND test_utilities)

# Integration Tests
add_executable(test_integration
    integration/test_schema_workflow.cpp
    integration/test_form_generation.cpp
    integration/test_config_cycle.cpp
    integration/test_file_formats.cpp
    integration/test_error_handling.cpp
)

target_link_libraries(test_integration
    GTest::GTest
    GTest::Main
    test_common
    nlohmann_json::nlohmann_json
    yaml-cpp
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

add_test(NAME Integration COMMAND test_integration)

# Run all tests with: ctest --output-on-failure
