// Test to verify configureRules save order is fixed
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::ordered_json;

int main()
{
    // Load the schema to verify the required array exists
    std::ifstream schema_file("resources/schemas/config.schema.json");
    if (!schema_file.is_open()) {
        std::cerr << "Failed to open schema file" << std::endl;
        return 1;
    }

    json schema;
    schema_file >> schema;
    schema_file.close();

    // Check if configureRules has required array
    if (!schema["properties"]["configureRules"].contains("required")) {
        std::cerr << "ERROR: configureRules does not have 'required' array!" << std::endl;
        return 1;
    }

    std::cout << "✓ configureRules has 'required' array" << std::endl;

    // Get the required array
    const auto& required = schema["properties"]["configureRules"]["required"];
    std::cout << "✓ configureRules required fields order:" << std::endl;
    for (size_t i = 0; i < required.size(); ++i) {
        std::cout << "  " << (i+1) << ". " << required[i].get<std::string>() << std::endl;
    }

    // Load the config file to verify order is preserved
    std::ifstream config_file("resources/configs/config_new.json");
    if (!config_file.is_open()) {
        std::cerr << "Failed to open config file" << std::endl;
        return 1;
    }

    json config;
    config_file >> config;
    config_file.close();

    std::cout << "\n✓ config_new.json configureRules order:" << std::endl;
    const auto& config_rules = config["configureRules"];
    size_t idx = 0;
    for (auto it = config_rules.begin(); it != config_rules.end(); ++it, ++idx) {
        std::cout << "  " << (idx+1) << ". " << it.key() << std::endl;
    }

    // Verify they match
    std::cout << "\n✓ Verifying order matches..." << std::endl;
    bool order_matches = true;
    for (size_t i = 0; i < required.size() && i < idx; ++i) {
        std::string req_key = required[i].get<std::string>();
        
        // Get actual key from config
        auto config_it = config_rules.begin();
        std::advance(config_it, i);
        std::string actual_key = config_it.key();

        if (req_key != actual_key) {
            std::cerr << "  MISMATCH at position " << (i+1) << ": expected '" << req_key 
                      << "', got '" << actual_key << "'" << std::endl;
            order_matches = false;
        }
    }

    if (order_matches) {
        std::cout << "✓ Order matches perfectly!" << std::endl;
        return 0;
    } else {
        std::cout << "✗ Order does not match" << std::endl;
        return 1;
    }
}
