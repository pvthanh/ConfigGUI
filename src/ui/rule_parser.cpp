// SPDX-License-Identifier: MIT
// RuleParser - Implementation

#include "rule_parser.h"
#include <regex>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cmath>

namespace configgui {
namespace ui {

// MISRA C++ compliant float comparison
static bool isFloatEqual(double a, double b)
{
    return std::fabs(a - b) < 1e-9;
}

RuleDefinition RuleParser::parseShorthand(const std::string& field_name, const std::string& shorthand)
{
    RuleDefinition rule;
    rule.name = field_name;

    std::string working = shorthand;

    // Step 1: Parse type and optional marker (e.g., "string?" or "integer[0,5]?")
    std::string type_part;
    bool is_optional = false;

    parseTypeAndOptional(working, type_part, is_optional);
    rule.type = type_part;
    rule.required = !is_optional;

    // Step 2: Parse range constraints if type is numeric (e.g., "[0,5]" or "[-90,90]")
    if ((type_part == "integer" || type_part == "float") && working.find('[') != std::string::npos)
    {
        parseRangeConstraints(working, rule.minimum, rule.maximum);
    }

    // Step 3: Parse modifiers in curly braces (e.g., "{required,enum:A|B}" or "{optional,pattern:.*}")
    if (working.find('{') != std::string::npos)
    {
        parseModifiers(working, rule.required, rule.enum_values, rule.pattern);
    }

    // Set allow_empty based on required flag
    rule.allow_empty = !rule.required;

    return rule;
}

std::string RuleParser::toShorthand(const RuleDefinition& rule)
{
    std::string result = rule.type;

    // Add range constraints for numeric types
    if (rule.type == "integer" || rule.type == "float")
    {
        if (!isFloatEqual(rule.minimum, std::numeric_limits<double>::lowest()) ||
            !isFloatEqual(rule.maximum, std::numeric_limits<double>::max()))
        {
            result += "[";
            if (!isFloatEqual(rule.minimum, std::numeric_limits<double>::lowest()))
            {
                result += std::to_string(static_cast<int>(rule.minimum));
            }
            result += ",";
            if (!isFloatEqual(rule.maximum, std::numeric_limits<double>::max()))
            {
                result += std::to_string(static_cast<int>(rule.maximum));
            }
            result += "]";
        }
    }

    // Add modifiers if needed
    bool has_enum = !rule.enum_values.empty();
    bool has_pattern = !rule.pattern.empty();
    bool has_modifiers = has_enum || has_pattern || !rule.required;

    if (has_modifiers)
    {
        result += "{";
        result += rule.required ? "required" : "optional";

        if (has_enum)
        {
            result += ",enum:";
            for (size_t i = 0; i < rule.enum_values.size(); ++i)
            {
                if (i > 0) result += "|";
                result += rule.enum_values[i];
            }
        }

        if (has_pattern)
        {
            result += ",pattern:" + rule.pattern;
        }

        result += "}";
    }
    else if (!rule.required)
    {
        result += "?";
    }

    return result;
}

json RuleParser::convertOldFormatToNew(const json& old_rules)
{
    json new_format = json::object();

    if (!old_rules.is_array())
    {
        return new_format;
    }

    for (const auto& rule_obj : old_rules)
    {
        if (!rule_obj.is_object() || !rule_obj.contains("name"))
        {
            continue;
        }

        std::string field_name = rule_obj["name"].get<std::string>();

        RuleDefinition rule;
        rule.name = field_name;

        // Extract type
        if (rule_obj.contains("type"))
        {
            rule.type = rule_obj["type"].get<std::string>();
        }

        // Extract constraints
        if (rule_obj.contains("minimum"))
        {
            rule.minimum = rule_obj["minimum"].get<double>();
        }
        if (rule_obj.contains("maximum"))
        {
            rule.maximum = rule_obj["maximum"].get<double>();
        }

        // Extract allowEmpty (inverse of required)
        if (rule_obj.contains("allowEmpty"))
        {
            rule.allow_empty = rule_obj["allowEmpty"].get<bool>();
            rule.required = !rule.allow_empty;
        }

        // Extract enum
        if (rule_obj.contains("enum") && rule_obj["enum"].is_array())
        {
            for (const auto& enum_item : rule_obj["enum"])
            {
                if (enum_item.is_string())
                {
                    rule.enum_values.push_back(enum_item.get<std::string>());
                }
            }
        }

        // Extract pattern
        if (rule_obj.contains("pattern"))
        {
            rule.pattern = rule_obj["pattern"].get<std::string>();
        }

        // Convert to shorthand
        new_format[field_name] = toShorthand(rule);
    }

    return new_format;
}

json RuleParser::convertNewFormatToOld(const json& rules_obj)
{
    json old_format = json::array();

    if (!rules_obj.is_object())
    {
        return old_format;
    }

    for (auto it = rules_obj.begin(); it != rules_obj.end(); ++it)
    {
        std::string field_name = it.key();
        std::string shorthand;

        if (it.value().is_string())
        {
            shorthand = it.value().get<std::string>();
        }
        else
        {
            continue;
        }

        // Parse shorthand to RuleDefinition
        RuleDefinition rule = parseShorthand(field_name, shorthand);

        // Convert to old format object
        json old_rule = json::object();
        old_rule["name"] = rule.name;
        old_rule["type"] = rule.type;

        if (!isFloatEqual(rule.minimum, std::numeric_limits<double>::lowest()))
        {
            old_rule["minimum"] = rule.minimum;
        }
        if (!isFloatEqual(rule.maximum, std::numeric_limits<double>::max()))
        {
            old_rule["maximum"] = rule.maximum;
        }

        old_rule["allowEmpty"] = rule.allow_empty;

        if (!rule.enum_values.empty())
        {
            old_rule["enum"] = rule.enum_values;
        }
        if (!rule.pattern.empty())
        {
            old_rule["pattern"] = rule.pattern;
        }

        old_format.push_back(old_rule);
    }

    return old_format;
}

void RuleParser::parseTypeAndOptional(const std::string& input, std::string& type, bool& optional)
{
    // Remove trailing '?' to detect optional
    std::string working = input;
    optional = false;

    // Extract type (before any special characters like '[' or '{')
    size_t bracket_pos = working.find('[');
    size_t brace_pos = working.find('{');
    size_t question_pos = working.find('?');

    size_t end_pos = std::string::npos;
    if (bracket_pos != std::string::npos) end_pos = std::min(end_pos, bracket_pos);
    if (brace_pos != std::string::npos) end_pos = std::min(end_pos, brace_pos);
    if (question_pos != std::string::npos)
    {
        optional = true;
        end_pos = std::min(end_pos, question_pos);
    }

    if (end_pos != std::string::npos)
    {
        type = working.substr(0, end_pos);
    }
    else
    {
        type = working;
    }
}

void RuleParser::parseRangeConstraints(const std::string& input, double& min, double& max)
{
    // Extract content between '[' and ']'
    size_t start = input.find('[');
    size_t end = input.find(']');

    if (start == std::string::npos || end == std::string::npos)
    {
        return;
    }

    std::string range_str = input.substr(start + 1, end - start - 1);
    size_t comma_pos = range_str.find(',');

    if (comma_pos == std::string::npos)
    {
        return;
    }

    std::string min_str = range_str.substr(0, comma_pos);
    std::string max_str = range_str.substr(comma_pos + 1);

    // Trim whitespace
    min_str.erase(0, min_str.find_first_not_of(" \t"));
    min_str.erase(min_str.find_last_not_of(" \t") + 1);
    max_str.erase(0, max_str.find_first_not_of(" \t"));
    max_str.erase(max_str.find_last_not_of(" \t") + 1);

    // Parse values
    if (!min_str.empty() && min_str != "+")
    {
        try
        {
            min = std::stod(min_str);
        }
        catch (...)
        {
        }
    }
    if (!max_str.empty() && max_str != "+")
    {
        try
        {
            max = std::stod(max_str);
        }
        catch (...)
        {
        }
    }
}

void RuleParser::parseModifiers(const std::string& input, bool& required, std::vector<std::string>& enums,
                                std::string& pattern)
{
    // Extract content between '{' and '}'
    size_t start = input.find('{');
    size_t end = input.find('}');

    if (start == std::string::npos || end == std::string::npos)
    {
        return;
    }

    std::string mods_str = input.substr(start + 1, end - start - 1);

    // Split by comma to get individual modifiers
    std::istringstream iss(mods_str);
    std::string mod;

    while (std::getline(iss, mod, ','))
    {
        // Trim whitespace
        mod.erase(0, mod.find_first_not_of(" \t"));
        mod.erase(mod.find_last_not_of(" \t") + 1);

        if (mod == "required")
        {
            required = true;
        }
        else if (mod == "optional")
        {
            required = false;
        }
        else if (mod.find("enum:") == 0)
        {
            // Extract enum values: "enum:val1|val2|val3"
            std::string enum_str = mod.substr(5);  // Skip "enum:"
            std::istringstream enum_iss(enum_str);
            std::string enum_val;

            while (std::getline(enum_iss, enum_val, '|'))
            {
                enum_val.erase(0, enum_val.find_first_not_of(" \t"));
                enum_val.erase(enum_val.find_last_not_of(" \t") + 1);
                if (!enum_val.empty())
                {
                    enums.push_back(enum_val);
                }
            }
        }
        else if (mod.find("pattern:") == 0)
        {
            pattern = mod.substr(8);  // Skip "pattern:"
        }
    }
}

} // namespace ui
} // namespace configgui
