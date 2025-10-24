# src/qt/CMakeLists.txt - Qt-specific application module
# T004: Qt GUI application build configuration
# This module depends on ConfigGUICore and adds Qt-specific UI components

# Qt-specific UI sources
set(QT_UI_SOURCES
    ../ui/main_window.h
    ../ui/main_window.cpp
    ../ui/widget_factory.h
    ../ui/widget_factory.cpp
    ../ui/form_generator.h
    ../ui/form_generator.cpp
    ../ui/array_widget.h
    ../ui/array_widget.cpp
    ../ui/range_widget.h
    ../ui/range_widget.cpp
    ../ui/dictionary_widget.h
    ../ui/dictionary_widget.cpp
    ../ui/object_array_widget.h
    ../ui/object_array_widget.cpp
    ../ui/path_selector_widget.h
    ../ui/path_selector_widget.cpp
    ../ui/rule_parser.h
    ../ui/rule_parser.cpp
    ../ui/rule_editor_widget.h
    ../ui/rule_editor_widget.cpp
    ../ui/validation_feedback_widget.h
    ../ui/validation_feedback_widget.cpp
)

# Qt validators
set(QT_VALIDATOR_SOURCES
    ../validators/ivalidator.h
    ../validators/required_validator.h
    ../validators/required_validator.cpp
    ../validators/type_validator.h
    ../validators/type_validator.cpp
    ../validators/range_validator.h
    ../validators/range_validator.cpp
    ../validators/pattern_validator.h
    ../validators/pattern_validator.cpp
    ../validators/enum_validator.h
    ../validators/enum_validator.cpp
)

# Qt I/O sources
set(QT_IO_SOURCES
    ../io/json_reader.h
    ../io/json_reader.cpp
    ../io/json_writer.h
    ../io/json_writer.cpp
    ../io/yaml_reader.h
    ../io/yaml_reader.cpp
    ../io/yaml_writer.h
    ../io/yaml_writer.cpp
)

# Qt utilities
set(QT_UTILS_SOURCES
    ../utils/logger.h
    ../utils/logger.cpp
    ../utils/string_utils.h
    ../utils/string_utils.cpp
    ../utils/file_utils.h
    ../utils/file_utils.cpp
    ../utils/crash_handler.h
    ../utils/crash_handler.cpp
)

# Main Qt application executable
add_executable(ConfigGUI
    ../main.cpp
    ${QT_UI_SOURCES}
    ${QT_VALIDATOR_SOURCES}
    ${QT_IO_SOURCES}
    ${QT_UTILS_SOURCES}
)

# Link against core library and Qt
target_link_libraries(ConfigGUI
    PRIVATE
        ${CONFIGGUI_CORE_LIB}
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

# Include paths
target_include_directories(ConfigGUI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# C++17 requirement
target_compile_features(ConfigGUI PRIVATE cxx_std_17)

# MISRA C++ compliance flags
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
