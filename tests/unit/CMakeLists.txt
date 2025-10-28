# tests/unit/CMakeLists.txt
# Unit tests organization

# Add subdirectories with tests
add_subdirectory(core)
add_subdirectory(html)
add_subdirectory(io)
add_subdirectory(validators)
add_subdirectory(utils)

# Standalone unit test files (multi-format save feature Phase 3)
# JSON serializer unit tests (T022)
add_executable(test_json_serializer_unit
    test_json_serializer.cpp
)

target_link_libraries(test_json_serializer_unit
    PRIVATE
        GTest::gtest_main
        GTest::gtest
        ConfigGUICore
        nlohmann_json::nlohmann_json
)

target_include_directories(test_json_serializer_unit PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

set_target_properties(test_json_serializer_unit PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(UNIX)
    target_compile_options(test_json_serializer_unit PRIVATE
        -Wall -Wextra -Wpedantic -Werror
        -Wno-error=unused-parameter
        -Wno-error=unused-variable
    )
endif()

gtest_discover_tests(test_json_serializer_unit)

message(STATUS "✅ Unit Tests: test_json_serializer (Phase 3 T022)")
