// Test to verify schema properties order is used (not required array order)
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::ordered_json;

int main()
{
    // Load schema
    std::ifstream schema_file("resources/schemas/config.schema.json");
    json schema;
    schema_file >> schema;
    schema_file.close();

    std::cout << "Root level properties order (from schema definition, NOT required array):" << std::endl;
    const auto& root_props = schema["properties"];
    size_t idx = 0;
    for (auto it = root_props.begin(); it != root_props.end(); ++it, ++idx) {
        std::cout << "  " << (idx+1) << ". " << it.key() << std::endl;
    }

    std::cout << "\nconfigureRules properties order (from schema definition, NOT required array):" << std::endl;
    const auto& configure_rules_props = schema["properties"]["configureRules"]["properties"];
    idx = 0;
    for (auto it = configure_rules_props.begin(); it != configure_rules_props.end(); ++it, ++idx) {
        std::cout << "  " << (idx+1) << ". " << it.key() << std::endl;
    }

    std::cout << "\nconfigureRules required array (for reference, NOT used for ordering):" << std::endl;
    const auto& required = schema["properties"]["configureRules"]["required"];
    for (size_t i = 0; i < required.size(); ++i) {
        std::cout << "  " << (i+1) << ". " << required[i].get<std::string>() << std::endl;
    }

    return 0;
}
