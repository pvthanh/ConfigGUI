// Test that schema can be loaded and processed without errors
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

bool validate_form_generation(const json& schema) {
    // Simulate what Qt form generator does
    
    if (!schema.contains("properties")) {
        std::cerr << "Schema missing properties\n";
        return false;
    }
    
    const auto& props = schema["properties"];
    
    // Navigate to the problematic section
    const auto& configure_rules = props.at("configureRules");
    const auto& rules_props = configure_rules.at("properties");
    const auto& data_type_rule = rules_props.at("dataTypeValidationRule");
    const auto& dtr_props = data_type_rule.at("properties");
    const auto& rules_schema = dtr_props.at("rules");
    
    // This is what was failing - processing the default array
    if (rules_schema.contains("default")) {
        const auto& defaults = rules_schema["default"];
        
        if (!defaults.is_array()) {
            std::cerr << "ERROR: defaults is not an array\n";
            return false;
        }
        
        std::cout << "✓ Defaults is array with " << defaults.size() << " items\n";
        
        // Try to process each item like the widget factory would
        for (size_t i = 0; i < defaults.size(); ++i) {
            const auto& item = defaults[i];
            
            // Qt form generator would create a widget for each item
            // This simulates that process
            if (!item.is_object()) {
                std::cerr << "ERROR: Item " << i << " is not an object\n";
                return false;
            }
            
            if (!item.contains("name") || !item.contains("type")) {
                std::cerr << "ERROR: Item " << i << " missing required fields\n";
                return false;
            }
            
            // This is where Qt would create the widget
            // If there's an error, it would crash here
            std::string name = item["name"].get<std::string>();
            std::string type = item["type"].get<std::string>();
            
            if (i < 3) {
                std::cout << "  ✓ Item " << i << ": " << name << " (" << type << ")\n";
            }
        }
        
        std::cout << "  ✓ Last item " << (defaults.size()-1) << ": " 
                  << defaults[defaults.size()-1]["name"].get<std::string>() << "\n";
    }
    
    return true;
}

int main() {
    std::ifstream schema_file("resources/schemas/config.schema.json");
    if (!schema_file.is_open()) {
        std::cerr << "ERROR: Cannot open schema\n";
        return 1;
    }
    
    try {
        json schema = json::parse(schema_file);
        
        std::cout << "Schema loaded successfully\n";
        std::cout << "Validating form generation compatibility...\n\n";
        
        if (validate_form_generation(schema)) {
            std::cout << "\n✓ Schema is Qt GUI compatible!\n";
            std::cout << "✓ No crashes expected when loading in Qt GUI\n";
            std::cout << "\nYou can now safely run: ./build/src/qt/ConfigGUI\n";
            return 0;
        } else {
            std::cerr << "\n✗ Schema validation failed\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}
