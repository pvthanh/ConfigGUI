# tests/integration/CMakeLists.txt
# Integration tests for HTML form generation feature
# Task: T083-T086 (Phase 3 Block 4)

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

message(STATUS "✅ Integration Tests (Phase 3 T083-T086): test_main_form")
