# src/core/CMakeLists.txt - Shared core library
# T003: Core abstraction library (no Qt dependencies)
# The core module contains all schema, validation, and I/O logic
# It will be linked as a static library by both Qt and HTML applications

# Collect core source files from subdirectories

# Core data model
set(CORE_DATA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/data/configuration_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/configuration_data.h
    ${CMAKE_CURRENT_SOURCE_DIR}/data/form_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data/form_state.h
)

# Multi-format save feature models (Phase 1)
set(CORE_MODELS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/models/format_type.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/format_type.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/serialization_context.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/serialization_result.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/serialization_result.cpp
)

# Multi-format save feature serializers (Phase 1-3)
set(CORE_SERIALIZERS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/serializers/format_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serializers/serializer_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serializers/serializer_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/serializers/json_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serializers/json_serializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/serializers/ini_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serializers/ini_serializer.cpp
)

# Core schema and validation
set(CORE_SCHEMA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/schema.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/schema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/schema_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/schema_loader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/schema_validator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/schema_validator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/validation_error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/validation_error.h
)

# Core I/O operations
set(CORE_IO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/io/json_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/json_reader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/json_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/json_writer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/yaml_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/yaml_reader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/yaml_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/yaml_writer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/ini_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/ini_parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/ini_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/ini_reader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/ini_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/ini_writer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/configuration_writer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/configuration_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/configuration_reader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/configuration_reader.cpp
)

# Core infrastructure (no subdirectory)
set(CORE_BASE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/result.h
    ${CMAKE_CURRENT_SOURCE_DIR}/error_types.h
)

# Combine all core sources
set(CORE_SOURCES
    ${CORE_BASE_SOURCES}
    ${CORE_DATA_SOURCES}
    ${CORE_SCHEMA_SOURCES}
    ${CORE_IO_SOURCES}
    ${CORE_MODELS_SOURCES}
    ${CORE_SERIALIZERS_SOURCES}
)

# Create core static library (no Qt dependencies)
add_library(ConfigGUICore STATIC ${CORE_SOURCES})

# Set include paths
target_include_directories(ConfigGUICore 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/core>
)

# Link dependencies
# Note: Qt is linked here when BUILD_QT_APP=ON (Phase 2 temporary)
# Will be removed in refactoring to make core truly Qt-free
target_link_libraries(ConfigGUICore
    PUBLIC
        $<$<BOOL:${BUILD_QT_APP}>:Qt6::Core>
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator::validator
        yaml-cpp::yaml-cpp
)

# Include mINI header-only library
target_include_directories(ConfigGUICore
    PUBLIC
        ${mINI_INCLUDE_DIRS}
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
