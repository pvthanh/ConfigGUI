// SPDX-License-Identifier: MIT
// JSONSchema - Implementation

#include "schema.h"
#include <QString>
#include <vector>

namespace configgui {
namespace core {

JSONSchema::JSONSchema(const json& schema_json) : schema_(schema_json)
{
    buildIndex();
}

JSONSchema::JSONSchema(const json& schema_json, std::shared_ptr<SchemaValidator> validator)
    : schema_(schema_json)
    , validator_(std::move(validator))
{
    buildIndex();
}

void JSONSchema::buildIndex()
{
    // OPTIMIZATION: Pre-build index of property names for O(1) lookups
    property_index_.clear();
    
    if (schema_.contains("properties"))
    {
        const auto& props = schema_["properties"];
        if (props.is_object())
        {
            // Iterate once and store all property names in hash set
            for (auto it = props.begin(); it != props.end(); ++it)
            {
                property_index_.insert(it.key());
            }
        }
    }
}

QString JSONSchema::title() const
{
    if (schema_.contains("title"))
    {
        return QString::fromStdString(schema_["title"].get<std::string>());
    }
    return "";
}

QString JSONSchema::description() const
{
    if (schema_.contains("description"))
    {
        return QString::fromStdString(schema_["description"].get<std::string>());
    }
    return "";
}

std::vector<QString> JSONSchema::required_fields() const
{
    std::vector<QString> required;
    if (schema_.contains("required"))
    {
        const auto& req = schema_["required"];
        if (req.is_array())
        {
            for (const auto& field : req)
            {
                if (field.is_string())
                {
                    required.push_back(QString::fromStdString(field.get<std::string>()));
                }
            }
        }
    }
    return required;
}

const json& JSONSchema::properties() const
{
    static const json empty_object = json::object();
    if (schema_.contains("properties"))
    {
        return schema_["properties"];
    }
    return empty_object;
}

bool JSONSchema::hasProperty(const std::string& name) const
{
    // OPTIMIZATION: O(1) lookup using hash set index
    // Instead of: properties().contains(name) which requires JSON traversal
    return property_index_.find(name) != property_index_.end();
}

const json* JSONSchema::getProperty(const std::string& name) const
{
    // OPTIMIZATION: Only access JSON if property exists in index
    // Prevents expensive JSON operations for non-existent properties
    if (hasProperty(name) && schema_.contains("properties"))
    {
        const auto& props = schema_["properties"];
        if (props.contains(name))
        {
            return &props[name];
        }
    }
    return nullptr;
}

} // namespace core
} // namespace configgui
