# src/html/CMakeLists.txt - HTML server application module
# T059-T065: HTTP Server Core implementation
# T075-T082: Static Asset Serving & Docker support
# Provides HTTP server with cpp-httplib for web-based ConfigGUI deployment

# Asset Configuration (T075)
# ASSET_MODE options:
#  - FILESYSTEM (default): Serve assets from src/html/assets/ at runtime (flexibility for development)
#  - EMBEDDED: Embed assets in binary (not implemented yet, requires xxd/bin2c)
set(ASSET_MODE "FILESYSTEM" CACHE STRING "Asset serving mode: FILESYSTEM or EMBEDDED")

# Define asset directories
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

# HTML server sources
set(HTML_SERVER_SOURCES
    main.cpp
    server/http_server.cpp
    server/http_server.h
)

set(HTML_HANDLERS_SOURCES
    handlers/request_handler.cpp
    handlers/request_handler.h
    handlers/schema_service.cpp
    handlers/schema_service.h
    handlers/file_handler.cpp
    handlers/file_handler.h
)

# Define asset resources (for reference and installation)
set(HTML_ASSETS
    "${ASSET_DIR}/index.html"
    "${ASSET_DIR}/css/main.css"
    "${ASSET_DIR}/js/main.js"
)

# Verify assets exist during configuration (T075)
if(NOT EXISTS "${ASSET_DIR}/index.html")
    message(WARNING "Asset not found: ${ASSET_DIR}/index.html (will fallback to embedded landing page)")
endif()
if(NOT EXISTS "${ASSET_DIR}/css/main.css")
    message(WARNING "Asset not found: ${ASSET_DIR}/css/main.css (will fallback to minimal CSS)")
endif()
if(NOT EXISTS "${ASSET_DIR}/js/main.js")
    message(WARNING "Asset not found: ${ASSET_DIR}/js/main.js (will fallback to minimal JS)")
endif()

# Create HTML server executable
add_executable(ConfigGUIServer
    ${HTML_SERVER_SOURCES}
    ${HTML_HANDLERS_SOURCES}
)

# Link against core library and dependencies
target_link_libraries(ConfigGUIServer
    PRIVATE
        ${CONFIGGUI_CORE_LIB}
        nlohmann_json::nlohmann_json
        yaml-cpp
)

# C++ standard and compiler flags
set_target_properties(ConfigGUIServer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add compiler flags for MISRA compliance
if(UNIX)
    target_compile_options(ConfigGUIServer PRIVATE
        -Wall -Wextra -Wpedantic -Werror
        -Wno-error=unused-parameter
        -Wno-error=unused-variable
    )
elseif(MSVC)
    target_compile_options(ConfigGUIServer PRIVATE
        /W4
    )
endif()

# Include paths (cpp-httplib is header-only, so include its source dir)
target_include_directories(ConfigGUIServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/server
        ${CMAKE_CURRENT_SOURCE_DIR}/handlers
        ${CMAKE_SOURCE_DIR}/src/core
        ${cpp-httplib_SOURCE_DIR}
)

# Define compile-time asset directory for fallback serving (T075)
target_compile_definitions(ConfigGUIServer
    PRIVATE
        CONFIGGUI_ASSET_DIR="${ASSET_DIR}"
        CONFIGGUI_ASSET_MODE="${ASSET_MODE}"
)

# Installation
install(TARGETS ConfigGUIServer
    RUNTIME DESTINATION bin
)

# Install HTML assets (T075)
install(FILES
    ${HTML_ASSETS}
    DESTINATION share/configgui-html/assets
)

install(DIRECTORY
    "${ASSET_DIR}/css"
    "${ASSET_DIR}/js"
    DESTINATION share/configgui-html/assets
)

message(STATUS "✅ HTML Server module (Phase 3 T059-T065): ConfigGUIServer")
message(STATUS "✅ Asset Serving Configuration (Phase 3 T075): ${ASSET_MODE} mode")

