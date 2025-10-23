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

# Qt Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 COMPONENTS 
    Core 
    Gui 
    Widgets 
    Test
    REQUIRED
)

# Find nlohmann_json
find_package(nlohmann_json 3.11 REQUIRED)

# Find json-schema-validator
find_package(nlohmann_json_schema_validator REQUIRED)

# Find yaml-cpp
find_package(yaml-cpp REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(CORE_SOURCES
    src/core/schema.h
    src/core/schema.cpp
    src/core/configuration_data.h
    src/core/configuration_data.cpp
    src/core/validation_error.h
    src/core/validation_error.cpp
    src/core/form_state.h
    src/core/form_state.cpp
    src/core/result.h
    src/core/error_types.h
    src/core/schema_loader.h
    src/core/schema_loader.cpp
    src/core/schema_validator.h
    src/core/schema_validator.cpp
)

set(UI_SOURCES
    src/ui/main_window.h
    src/ui/main_window.cpp
    src/ui/widget_factory.h
    src/ui/widget_factory.cpp
    src/ui/form_generator.h
    src/ui/form_generator.cpp
    src/ui/array_widget.h
    src/ui/array_widget.cpp
    src/ui/range_widget.h
    src/ui/range_widget.cpp
    src/ui/dictionary_widget.h
    src/ui/dictionary_widget.cpp
    src/ui/object_array_widget.h
    src/ui/object_array_widget.cpp
    src/ui/path_selector_widget.h
    src/ui/path_selector_widget.cpp
    src/ui/rule_parser.h
    src/ui/rule_parser.cpp
    src/ui/rule_editor_widget.h
    src/ui/rule_editor_widget.cpp
    src/ui/validation_feedback_widget.h
    src/ui/validation_feedback_widget.cpp
)

set(VALIDATOR_SOURCES
    src/validators/ivalidator.h
    src/validators/required_validator.h
    src/validators/required_validator.cpp
    src/validators/type_validator.h
    src/validators/type_validator.cpp
    src/validators/range_validator.h
    src/validators/range_validator.cpp
    src/validators/pattern_validator.h
    src/validators/pattern_validator.cpp
    src/validators/enum_validator.h
    src/validators/enum_validator.cpp
)

set(IO_SOURCES
    src/io/json_reader.h
    src/io/json_reader.cpp
    src/io/json_writer.h
    src/io/json_writer.cpp
    src/io/yaml_reader.h
    src/io/yaml_reader.cpp
    src/io/yaml_writer.h
    src/io/yaml_writer.cpp
)

set(UTILS_SOURCES
    src/utils/logger.h
    src/utils/logger.cpp
    src/utils/string_utils.h
    src/utils/string_utils.cpp
    src/utils/file_utils.h
    src/utils/file_utils.cpp
    src/utils/crash_handler.h
    src/utils/crash_handler.cpp
)

# Main application executable
add_executable(ConfigGUI
    src/main.cpp
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${VALIDATOR_SOURCES}
    ${IO_SOURCES}
    ${UTILS_SOURCES}
)

# Link libraries
target_link_libraries(ConfigGUI
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    nlohmann_json::nlohmann_json
    nlohmann_json_schema_validator
    yaml-cpp
)

# MISRA C++ Compiler Flags
if(MSVC)
    target_compile_options(ConfigGUI PRIVATE /W4 /WX)
else()
    target_compile_options(ConfigGUI PRIVATE 
        -Wall 
        -Wextra 
        -Werror 
        -Wconversion 
        -Wsign-conversion 
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpedantic
        -Wfloat-equal
        -Wredundant-decls
        -Wdouble-promotion
    )
endif()

# Enable testing
enable_testing()

# Unit Tests
file(GLOB_RECURSE UNIT_TEST_SOURCES "tests/unit/*.cpp")
if(UNIT_TEST_SOURCES)
    add_executable(test_unit ${UNIT_TEST_SOURCES} ${CORE_SOURCES} ${VALIDATOR_SOURCES} ${IO_SOURCES} ${UTILS_SOURCES})
    target_link_libraries(test_unit
        Qt6::Core
        Qt6::Test
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator
        yaml-cpp
    )
    add_test(NAME UnitTests COMMAND test_unit)
endif()

# Integration Tests
file(GLOB_RECURSE INTEGRATION_TEST_SOURCES "tests/integration/*.cpp")
if(INTEGRATION_TEST_SOURCES)
    add_executable(test_integration ${INTEGRATION_TEST_SOURCES} ${CORE_SOURCES} ${UI_SOURCES} ${VALIDATOR_SOURCES} ${IO_SOURCES} ${UTILS_SOURCES})
    target_link_libraries(test_integration
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Test
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator
        yaml-cpp
    )
    add_test(NAME IntegrationTests COMMAND test_integration)
endif()

# clang-tidy target
if(CMAKE_CXX_CLANG_TIDY)
    add_custom_target(clang-tidy
        COMMAND clang-tidy ${CORE_SOURCES} ${UI_SOURCES} ${VALIDATOR_SOURCES} ${IO_SOURCES} ${UTILS_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# Print configuration
message(STATUS "ConfigGUI Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt Version: ${Qt6_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
