// SPDX-License-Identifier: MIT
// ConfigurationData - Implementation

#include "configuration_data.h"

namespace configgui {
namespace core {

const std::vector<ValidationError> ConfigurationData::empty_errors_;

ConfigurationData::ConfigurationData(const json& data) : data_(data) {}

json ConfigurationData::get_value(const std::string& field_name) const
{
    try
    {
        if (data_.contains(field_name))
        {
            return data_[field_name];
        }
    }
    catch (const std::exception& /*e*/)
    {
        // Field doesn't exist or type conversion failed
    }
    return json();
}

void ConfigurationData::set_value(const std::string& field_name, const json& value)
{
    mark_dirty(field_name);
    data_[field_name] = value;
}

void ConfigurationData::mark_dirty(const std::string& field_name)
{
    field_states_[field_name].is_dirty = true;
}

void ConfigurationData::mark_clean(const std::string& field_name)
{
    field_states_[field_name].is_dirty = false;
}

bool ConfigurationData::is_dirty(const std::string& field_name) const
{
    const auto it = field_states_.find(field_name);
    if (it != field_states_.end())
    {
        return it->second.is_dirty;
    }
    return false;
}

void ConfigurationData::set_focused(const std::string& field_name, bool focused)
{
    field_states_[field_name].is_focused = focused;
}

void ConfigurationData::add_error(const std::string& field_name, const ValidationError& error)
{
    field_states_[field_name].errors.push_back(error);
}

void ConfigurationData::clear_errors(const std::string& field_name)
{
    field_states_[field_name].errors.clear();
}

const std::vector<ValidationError>& ConfigurationData::get_errors(const std::string& field_name) const
{
    const auto it = field_states_.find(field_name);
    if (it != field_states_.end())
    {
        return it->second.errors;
    }
    return empty_errors_;
}

ValidationErrors ConfigurationData::all_errors() const
{
    ValidationErrors all;
    for (const auto& [field, state] : field_states_)
    {
        all.insert(all.end(), state.errors.begin(), state.errors.end());
    }
    return all;
}

bool ConfigurationData::has_errors() const
{
    for (const auto& [field, state] : field_states_)
    {
        if (!state.errors.empty())
        {
            return true;
        }
    }
    return false;
}

bool ConfigurationData::has_errors(const std::string& field_name) const
{
    const auto it = field_states_.find(field_name);
    if (it != field_states_.end())
    {
        return !it->second.errors.empty();
    }
    return false;
}

bool ConfigurationData::is_dirty() const
{
    for (const auto& [field, state] : field_states_)
    {
        if (state.is_dirty)
        {
            return true;
        }
    }
    return false;
}

void ConfigurationData::reset()
{
    field_states_.clear();
}

std::string ConfigurationData::to_json_string() const
{
    return data_.dump(2);
}

} // namespace core
} // namespace configgui
