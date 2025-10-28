# tests/integration/CMakeLists.txt
# Integration tests for HTML form generation feature and multi-format save

# JSON save workflow integration tests (Phase 3 T024)
add_executable(test_json_save_workflow
    test_json_save_workflow.cpp
)

target_link_libraries(test_json_save_workflow
    PRIVATE
        GTest::gtest_main
        GTest::gtest
        ConfigGUICore
        nlohmann_json::nlohmann_json
)

target_include_directories(test_json_save_workflow PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

set_target_properties(test_json_save_workflow PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(UNIX)
    target_compile_options(test_json_save_workflow PRIVATE
        -Wall -Wextra -Wpedantic -Werror
        -Wno-error=unused-parameter
        -Wno-error=unused-variable
    )
endif()

gtest_discover_tests(test_json_save_workflow)

# INI save workflow integration tests (Phase 4)
add_executable(test_ini_save_workflow
    test_ini_save_workflow.cpp
)

target_link_libraries(test_ini_save_workflow
    PRIVATE
        GTest::gtest_main
        GTest::gtest
        ConfigGUICore
        nlohmann_json::nlohmann_json
)

target_include_directories(test_ini_save_workflow PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

set_target_properties(test_ini_save_workflow PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(UNIX)
    target_compile_options(test_ini_save_workflow PRIVATE
        -Wall -Wextra -Wpedantic -Werror
        -Wno-error=unused-parameter
        -Wno-error=unused-variable
    )
endif()

gtest_discover_tests(test_ini_save_workflow)

# Main form serving integration tests
add_executable(test_main_form
    test_main_form.cpp
)

target_link_libraries(test_main_form
    PRIVATE
        GTest::gtest_main
        GTest::gtest
)

# Configure test properties
set_target_properties(test_main_form PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add compiler flags
if(UNIX)
    target_compile_options(test_main_form PRIVATE
        -Wall -Wextra -Wpedantic -Werror
        -Wno-error=unused-parameter
        -Wno-error=unused-variable
    )
endif()

# Register tests with CTest
gtest_discover_tests(test_main_form)

message(STATUS "✅ Integration Tests: test_json_save_workflow (Phase 3 T024)")
message(STATUS "✅ Integration Tests: test_ini_save_workflow (Phase 4)")
message(STATUS "✅ Integration Tests (Phase 3 T083-T086): test_main_form")
