// SPDX-License-Identifier: MIT
// RuleParser - Header for parsing shorthand rule format

#ifndef CONFIGGUI_UI_RULE_PARSER_H
#define CONFIGGUI_UI_RULE_PARSER_H

#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

namespace configgui {
namespace ui {

/**
 * @struct RuleDefinition
 * @brief Represents a parsed rule in structured format
 */
struct RuleDefinition
{
    std::string name;           // Field name
    std::string type;           // "string", "integer", "float", "boolean"
    bool required = true;       // false if "?"
    bool allow_empty = true;    // false if {required}
    
    // Constraints
    double minimum = std::numeric_limits<double>::lowest();
    double maximum = std::numeric_limits<double>::max();
    std::vector<std::string> enum_values;
    std::string pattern;        // Regex pattern
};

/**
 * @class RuleParser
 * @brief Parses shorthand rule format to/from structured format
 */
class RuleParser
{
public:
    /**
     * @brief Parse shorthand format string to RuleDefinition
     * @param field_name The field name (key)
     * @param shorthand The shorthand value (e.g., "string?", "integer[0,5]")
     * @return RuleDefinition with parsed values
     */
    static RuleDefinition parseShorthand(const std::string& field_name, const std::string& shorthand);

    /**
     * @brief Convert RuleDefinition back to shorthand format
     * @param rule The rule to serialize
     * @return Shorthand string representation
     */
    static std::string toShorthand(const RuleDefinition& rule);

    /**
     * @brief Convert old verbose format (array of objects) to new simplified format
     * @param old_rules Array of rule objects with name, type, etc.
     * @return Object with field_name: shorthand_value pairs
     */
    static json convertOldFormatToNew(const json& old_rules);

    /**
     * @brief Convert new simplified format back to old verbose format
     * @param rules_obj Object with field_name: shorthand_value pairs
     * @return Array of rule objects
     */
    static json convertNewFormatToOld(const json& rules_obj);

private:
    static void parseTypeAndOptional(const std::string& input, std::string& type, bool& optional);
    static void parseRangeConstraints(const std::string& input, double& min, double& max);
    static void parseModifiers(const std::string& input, bool& required, std::vector<std::string>& enums, std::string& pattern);
};

} // namespace ui
} // namespace configgui

#endif // CONFIGGUI_UI_RULE_PARSER_H
