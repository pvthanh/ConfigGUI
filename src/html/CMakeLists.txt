# src/html/CMakeLists.txt - HTML server application module
# T005: HTML + JavaScript server application build configuration
# This module depends on ConfigGUICore and cpp-httplib

# cpp-httplib is a header-only library, so just make include dir available
# include_directories(${cpp-httplib_SOURCE_DIR})

# HTML server sources (to be implemented in Phase 3)
set(HTML_SERVER_SOURCES
    # server/main.cpp
    # server/http_server.cpp
    # server/http_server.h
)

set(HTML_HANDLERS_SOURCES
    # handlers/schema_handler.cpp
    # handlers/schema_handler.h
    # handlers/form_handler.cpp
    # handlers/form_handler.h
    # handlers/validation_handler.cpp
    # handlers/validation_handler.h
)

# Create HTML server executable (placeholder - implementation in Phase 3)
# add_executable(ConfigGUIServer
#     ${HTML_SERVER_SOURCES}
#     ${HTML_HANDLERS_SOURCES}
# )
#
# target_link_libraries(ConfigGUIServer
#     PRIVATE
#         ${CONFIGGUI_CORE_LIB}
#         cpp-httplib
# )

message(STATUS "HTML Server module configured (implementation in Phase 3)")

