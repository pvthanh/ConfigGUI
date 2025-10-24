# src/core/CMakeLists.txt - Shared core library
# T003: Core abstraction library (no Qt dependencies)
# The core module contains all schema, validation, and I/O logic
# It will be linked as a static library by both Qt and HTML applications

# Collect core source files
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/schema.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/schema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/configuration_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/configuration_data.h
    ${CMAKE_CURRENT_SOURCE_DIR}/validation_error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/validation_error.h
    ${CMAKE_CURRENT_SOURCE_DIR}/form_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/form_state.h
    ${CMAKE_CURRENT_SOURCE_DIR}/result.h
    ${CMAKE_CURRENT_SOURCE_DIR}/error_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schema_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/schema_loader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schema_validator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/schema_validator.h
)

# Create core static library (no Qt dependencies)
add_library(ConfigGUICore STATIC ${CORE_SOURCES})

# Set include paths
target_include_directories(ConfigGUICore 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/core>
)

# Link dependencies (no Qt - pure C++)
target_link_libraries(ConfigGUICore
    PUBLIC
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator
        yaml-cpp
)

# C++17 requirement
target_compile_features(ConfigGUICore PUBLIC cxx_std_17)

# MISRA C++ compliance flags
if(MSVC)
    target_compile_options(ConfigGUICore PRIVATE /W4 /WX)
else()
    target_compile_options(ConfigGUICore PRIVATE
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

# Make ConfigGUICore available to parent scope
set(CONFIGGUI_CORE_LIB ConfigGUICore PARENT_SCOPE)
