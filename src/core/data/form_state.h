// SPDX-License-Identifier: MIT
// FormState - Tracks UI form state and rendering

#pragma once

#include "configuration_data.h"
#include <QString>
#include <map>

namespace configgui {
namespace core {

/// @brief Form-level state tracking
class FormState
{
public:
    FormState() = default;

    /// @brief Create form state from configuration
    explicit FormState(const ConfigurationData& config_data);

    /// @brief Check if form is currently valid
    [[nodiscard]] bool is_valid() const { return !has_validation_errors_; }

    /// @brief Mark form as having validation errors
    void set_validation_errors(bool has_errors) { has_validation_errors_ = has_errors; }

    /// @brief Get scroll position
    [[nodiscard]] int scroll_position() const { return scroll_position_; }

    /// @brief Set scroll position
    void set_scroll_position(int position) { scroll_position_ = position; }

    /// @brief Check if form has unsaved changes
    [[nodiscard]] bool has_unsaved_changes() const { return has_unsaved_changes_; }

    /// @brief Mark form as having unsaved changes
    void set_unsaved_changes(bool unsaved) { has_unsaved_changes_ = unsaved; }

    /// @brief Get active tab index (for tabbed forms)
    [[nodiscard]] int active_tab() const { return active_tab_index_; }

    /// @brief Set active tab index
    void set_active_tab(int index) { active_tab_index_ = index; }

    /// @brief Get field with focus
    [[nodiscard]] const QString& focused_field() const { return focused_field_; }

    /// @brief Set field with focus
    void set_focused_field(const QString& field) { focused_field_ = field; }

    /// @brief Check if form is loading data
    [[nodiscard]] bool is_loading() const { return is_loading_; }

    /// @brief Set loading state
    void set_loading(bool loading) { is_loading_ = loading; }

    /// @brief Reset to initial state
    void reset()
    {
        has_validation_errors_ = false;
        has_unsaved_changes_ = false;
        scroll_position_ = 0;
        active_tab_index_ = 0;
        focused_field_.clear();
        is_loading_ = false;
    }

private:
    bool has_validation_errors_ = false;
    bool has_unsaved_changes_ = false;
    int scroll_position_ = 0;
    int active_tab_index_ = 0;
    QString focused_field_;
    bool is_loading_ = false;
};

} // namespace core
} // namespace configgui
