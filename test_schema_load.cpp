// Quick test to validate schema structure
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

int main() {
    // Read the schema
    std::ifstream schema_file("resources/schemas/config.schema.json");
    if (!schema_file.is_open()) {
        std::cerr << "ERROR: Cannot open schema file\n";
        return 1;
    }
    
    try {
        json schema;
        schema_file >> schema;
        
        // Check the dataTypeValidationRule.rules structure
        const auto& rules_schema = schema["properties"]["configureRules"]["properties"]["dataTypeValidationRule"]["properties"]["rules"];
        
        if (!rules_schema.contains("default")) {
            std::cerr << "ERROR: No default value for rules\n";
            return 1;
        }
        
        const auto& defaults = rules_schema["default"];
        
        std::cout << "✓ Schema parsed successfully\n";
        std::cout << "✓ dataTypeValidationRule.rules found\n";
        std::cout << "✓ Default value exists\n";
        std::cout << "✓ Default value type: " << (defaults.is_array() ? "array" : "not array") << "\n";
        std::cout << "✓ Default array size: " << defaults.size() << "\n";
        
        if (defaults.size() > 0) {
            const auto& first_item = defaults[0];
            std::cout << "✓ First item type: " << (first_item.is_object() ? "object" : "NOT OBJECT - BUG!") << "\n";
            
            if (first_item.is_object()) {
                std::cout << "✓ First item has 'name' field: " << (first_item.contains("name") ? "YES" : "NO") << "\n";
                if (first_item.contains("name")) {
                    std::cout << "  - name value: " << first_item["name"].get<std::string>() << "\n";
                }
            }
        }
        
        // Check if there's any nested array problem
        bool has_nested_array = false;
        for (size_t i = 0; i < defaults.size(); ++i) {
            if (defaults[i].is_array()) {
                has_nested_array = true;
                std::cout << "ERROR: Item " << i << " is an array (not object)! BUG IN SCHEMA!\n";
            }
        }
        
        if (!has_nested_array) {
            std::cout << "\n✓ ALL ITEMS ARE OBJECTS - SCHEMA FIXED!\n";
            return 0;
        } else {
            std::cout << "\n✗ SCHEMA STILL HAS NESTED ARRAYS - NOT FIXED\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}
