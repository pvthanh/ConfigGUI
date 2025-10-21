// SPDX-License-Identifier: MIT
// FormState - Implementation

#include "form_state.h"

namespace configgui {
namespace core {

FormState::FormState(const ConfigurationData& config_data)
{
    has_validation_errors_ = config_data.has_errors();
    has_unsaved_changes_ = config_data.is_dirty();
}

} // namespace core
} // namespace configgui
