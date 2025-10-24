# src/CMakeLists.txt - Main source module organization
# T002: Modular source organization for dual-target build

# Core module - always built (shared by Qt and HTML)
add_subdirectory(core)

# Qt module - conditional on BUILD_QT_APP
if(BUILD_QT_APP)
    add_subdirectory(qt)
endif()

# HTML server module - conditional on BUILD_HTML_SERVER
# Note: cpp-httplib is made available in root CMakeLists.txt
if(BUILD_HTML_SERVER)
    add_subdirectory(html)
endif()
