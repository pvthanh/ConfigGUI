// Direct test of collectDataRecursive order logic
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>
#include <string>

using json = nlohmann::ordered_json;

// Simulate the ordering logic from collectDataRecursive
std::vector<std::string> getOrderedKeys(const json& schema)
{
    std::vector<std::string> field_order;
    
    if (!schema.contains("properties")) {
        return field_order;
    }

    const auto& properties = schema["properties"];
    
    if (schema.contains("required") && schema["required"].is_array())
    {
        // Use the required array order to match GUI display order
        for (const auto& required_field : schema["required"])
        {
            if (required_field.is_string())
            {
                field_order.push_back(required_field.get<std::string>());
            }
        }
    }
    else
    {
        // Fallback: iterate through properties in their definition order
        for (auto it = properties.begin(); it != properties.end(); ++it)
        {
            field_order.push_back(it.key());
        }
    }
    
    return field_order;
}

int main()
{
    // Load schema
    std::ifstream schema_file("resources/schemas/config.schema.json");
    json schema;
    schema_file >> schema;
    schema_file.close();

    // Test the root level
    std::cout << "Root level properties order (using 'required' array):" << std::endl;
    auto root_order = getOrderedKeys(schema);
    for (size_t i = 0; i < root_order.size(); ++i) {
        std::cout << "  " << (i+1) << ". " << root_order[i] << std::endl;
    }

    // Test configureRules level
    std::cout << "\nconfigureRules properties order (using 'required' array):" << std::endl;
    auto configure_rules_schema = schema["properties"]["configureRules"];
    auto configure_order = getOrderedKeys(configure_rules_schema);
    for (size_t i = 0; i < configure_order.size(); ++i) {
        std::cout << "  " << (i+1) << ". " << configure_order[i] << std::endl;
    }

    // Test individual rules (each should use required array within their properties)
    std::cout << "\nExample: LinksTableRule properties order:" << std::endl;
    auto links_table_rule = schema["properties"]["configureRules"]["properties"]["LinksTableRule"];
    auto links_order = getOrderedKeys(links_table_rule);
    for (size_t i = 0; i < links_order.size(); ++i) {
        std::cout << "  " << (i+1) << ". " << links_order[i] << std::endl;
    }

    return 0;
}
