// SPDX-License-Identifier: MIT
// ConfigurationData - Implementation

#include "configuration_data.h"

namespace configgui {
namespace core {

const std::vector<ValidationError> ConfigurationData::empty_errors_;

ConfigurationData::ConfigurationData(const json& data) : data_(data) {}

QVariant ConfigurationData::get_value(const QString& field_name) const
{
    try
    {
        const auto& value = data_[field_name.toStdString()];
        if (value.is_string())
        {
            return QVariant(QString::fromStdString(value.get<std::string>()));
        }
        if (value.is_number_integer())
        {
            return QVariant(value.get<int>());
        }
        if (value.is_number_float())
        {
            return QVariant(value.get<double>());
        }
        if (value.is_boolean())
        {
            return QVariant(value.get<bool>());
        }
    }
    catch (const std::exception& /*e*/)
    {
        // Field doesn't exist or type conversion failed
    }
    return QVariant();
}

void ConfigurationData::set_value(const QString& field_name, const QVariant& value)
{
    mark_dirty(field_name);

    const auto key = field_name.toStdString();
    if (value.type() == QVariant::String)
    {
        data_[key] = value.toString().toStdString();
    }
    else if (value.type() == QVariant::Int)
    {
        data_[key] = value.toInt();
    }
    else if (value.type() == QVariant::Double)
    {
        data_[key] = value.toDouble();
    }
    else if (value.type() == QVariant::Bool)
    {
        data_[key] = value.toBool();
    }
}

void ConfigurationData::mark_dirty(const QString& field_name)
{
    field_states_[field_name].is_dirty = true;
}

void ConfigurationData::mark_clean(const QString& field_name)
{
    field_states_[field_name].is_dirty = false;
}

bool ConfigurationData::is_dirty(const QString& field_name) const
{
    const auto it = field_states_.find(field_name);
    if (it != field_states_.end())
    {
        return it->second.is_dirty;
    }
    return false;
}

void ConfigurationData::set_focused(const QString& field_name, bool focused)
{
    field_states_[field_name].is_focused = focused;
}

void ConfigurationData::add_error(const QString& field_name, const ValidationError& error)
{
    field_states_[field_name].errors.push_back(error);
}

void ConfigurationData::clear_errors(const QString& field_name)
{
    field_states_[field_name].errors.clear();
}

const std::vector<ValidationError>& ConfigurationData::get_errors(const QString& field_name) const
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

bool ConfigurationData::has_errors(const QString& field_name) const
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
