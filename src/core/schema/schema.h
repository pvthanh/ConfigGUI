// SPDX-License-Identifier: MIT
// JSONSchema - Represents a parsed JSON schema

#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <QString>
#include <unordered_map>
#include <unordered_set>

using json = nlohmann::ordered_json;

namespace configgui {
namespace core {

// Forward declarations
class SchemaValidator;

/// @brief Represents a JSON Schema (Draft 7) with validation support
/// 
/// OPTIMIZATION: Builds field index on schema load for O(1) lookups
/// instead of O(n) iteration through properties.
class JSONSchema
{
public:
    /// @brief Create an empty schema
    JSONSchema() = default;

    /// @brief Create a schema from JSON
    explicit JSONSchema(const json& schema_json);

    /// @brief Create a schema with validator
    JSONSchema(const json& schema_json, std::shared_ptr<SchemaValidator> validator);

    /// @brief Get the underlying JSON schema
    [[nodiscard]] const json& raw_schema() const { return schema_; }

    /// @brief Get the title of the schema
    [[nodiscard]] QString title() const;

    /// @brief Get the description of the schema
    [[nodiscard]] QString description() const;

    /// @brief Check if schema is valid (Draft 7)
    [[nodiscard]] bool is_valid() const { return validator_ != nullptr; }

    /// @brief Get all required fields
    [[nodiscard]] std::vector<QString> required_fields() const;

    /// @brief Get properties in schema
    [[nodiscard]] const json& properties() const;

    /// @brief Check if property exists (O(1) with indexing)
    /// OPTIMIZATION: Uses hash map index instead of iteration
    [[nodiscard]] bool hasProperty(const std::string& name) const;

    /// @brief Get property by name (O(1) lookup)
    /// @param name Property name
    /// @return Pointer to property or nullptr if not found
    [[nodiscard]] const json* getProperty(const std::string& name) const;

    /// @brief Equality operator
    bool operator==(const JSONSchema& other) const { return schema_ == other.schema_; }

    /// @brief Inequality operator
    bool operator!=(const JSONSchema& other) const { return schema_ != other.schema_; }

    /// @brief Get the validator (for internal use)
    [[nodiscard]] std::shared_ptr<SchemaValidator> validator() const { return validator_; }

private:
    /// @brief Build index for fast property lookups
    /// Called automatically in constructors
    void buildIndex();

    json schema_;
    std::shared_ptr<SchemaValidator> validator_;
    
    // OPTIMIZATION: Hash set index for O(1) property existence checks
    std::unordered_set<std::string> property_index_;
};

} // namespace core
} // namespace configgui
