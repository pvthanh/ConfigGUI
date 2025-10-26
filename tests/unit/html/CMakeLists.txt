# tests/unit/html/CMakeLists.txt - HTML server unit tests

# HTML server unit test executable
add_executable(test_html_server
    test_schema_service.cpp
    ${CMAKE_SOURCE_DIR}/src/html/handlers/schema_service.cpp
)

# Link against gtest, core library, and HTML components
target_link_libraries(test_html_server
    PRIVATE
        GTest::GTest
        GTest::Main
        ${CONFIGGUI_CORE_LIB}
        nlohmann_json::nlohmann_json
        yaml-cpp
)

# Set C++ standard
set_target_properties(test_html_server PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Include paths for schema_service header
target_include_directories(test_html_server
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/html/handlers
        ${CMAKE_SOURCE_DIR}/src/html/server
        ${CMAKE_SOURCE_DIR}/src/core
        ${cpp-httplib_SOURCE_DIR}
)

# Compiler flags
if(UNIX)
    target_compile_options(test_html_server PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-error=unused-parameter
        -Wno-error=unused-variable
    )
elseif(MSVC)
    target_compile_options(test_html_server PRIVATE /W4)
endif()

# Register test
add_test(NAME SchemaServiceTests COMMAND test_html_server)
