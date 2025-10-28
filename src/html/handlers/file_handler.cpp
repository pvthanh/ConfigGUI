#include "file_handler.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace configgui {
namespace handlers {

std::string FileHandler::get_storage_directory() {
    // Store configs in ~/.configgui/configs
    const char* home = std::getenv("HOME");
    if (!home) {
        home = "/tmp";
    }
    
    return std::string(home) + "/.configgui/configs";
}

std::string FileHandler::sanitize_filename(const std::string& filename) {
    // Only allow alphanumeric, dash, underscore, dot, and .json/.yaml/.yml/.ini extension
    std::regex valid_filename(R"(^[a-zA-Z0-9._\-]+\.(json|yaml|yml|ini)$)");
    
    if (filename.empty()) {
        return "Filename cannot be empty";
    }
    
    if (!std::regex_match(filename, valid_filename)) {
        return "Invalid filename. Only alphanumeric, dots, dashes, underscores allowed. Must end with .json, .yaml, .yml, or .ini";
    }
    
    // Additional check for path traversal
    if (filename.find("..") != std::string::npos || 
        filename.find("/") != std::string::npos ||
        filename.find("\\") != std::string::npos) {
        return "Filename cannot contain path separators";
    }
    
    return "";  // Empty string means success
}

bool FileHandler::file_exists(const std::string& path) {
    return fs::exists(path);
}

std::string FileHandler::ensure_directory(const std::string& path) {
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);
        }
        return "";  // Empty string means success
    } catch (const std::exception& e) {
        return std::string("Failed to create directory: ") + e.what();
    }
}

void FileHandler::handle_save_config(const httplib::Request& req, httplib::Response& res) {
    try {
        std::cout << "[FILE] ============================================" << std::endl;
        std::cout << "[FILE] SAVE CONFIG HANDLER CALLED" << std::endl;
        std::cout << "[FILE] Content-Type: " << (req.has_header("Content-Type") ? req.get_header_value("Content-Type") : "NOT SET") << std::endl;
        std::cout << "[FILE] Body Length: " << req.body.length() << std::endl;
        std::cout << "[FILE] ============================================" << std::endl;
        
        if (req.body.empty()) {
            json response;
            response["success"] = false;
            response["error"] = "Request body is empty";
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        std::cout << "[FILE] Received POST /api/config/save" << std::endl;
        
        // Parse request JSON
        auto request_data = json::parse(req.body);
        
        if (!request_data.contains("filename") || !request_data.contains("data")) {
            json response;
            response["success"] = false;
            response["error"] = "Request must contain 'filename' and 'data' fields";
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        std::string filename = request_data["filename"].get<std::string>();
        std::string format = request_data.contains("format") ? 
                            request_data["format"].get<std::string>() : "json";
        
        std::cout << "[FILE] Filename: " << filename << std::endl;
        std::cout << "[FILE] Format: " << format << std::endl;
        
        // Prefer 'content' field if provided (already formatted), else use 'data' field
        std::string file_content;
        if (request_data.contains("content") && !request_data["content"].is_null()) {
            // Content is already formatted (INI or JSON string)
            file_content = request_data["content"].get<std::string>();
            std::cout << "[FILE] Using pre-formatted content (format: " << format << ")" << std::endl;
        } else {
            // Fall back to JSON data
            json config_data = request_data["data"];
            file_content = config_data.dump(2);  // Pretty-print JSON
            std::cout << "[FILE] Using JSON formatted data" << std::endl;
        }
        
        std::cout << "[FILE] File content length: " << file_content.length() << " bytes" << std::endl;
        
        // Sanitize filename
        std::string sanitize_err = sanitize_filename(filename);
        if (!sanitize_err.empty()) {
            std::cout << "[FILE] Sanitize error: " << sanitize_err << std::endl;
            json response;
            response["success"] = false;
            response["error"] = sanitize_err;
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        std::cout << "[FILE] Filename sanitized successfully" << std::endl;
        
        // Ensure storage directory exists
        std::string storage_dir = get_storage_directory();
        std::string dir_err = ensure_directory(storage_dir);
        if (!dir_err.empty()) {
            std::cout << "[FILE] Directory error: " << dir_err << std::endl;
            json response;
            response["success"] = false;
            response["error"] = dir_err;
            res.set_content(response.dump(), "application/json");
            res.status = 500;
            return;
        }
        std::cout << "[FILE] Storage directory: " << storage_dir << std::endl;
        
        // Write file
        std::string full_path = storage_dir + "/" + filename;
        std::cout << "[FILE] Full path: " << full_path << std::endl;
        std::ofstream outfile(full_path);
        if (!outfile.is_open()) {
            std::cout << "[FILE] Failed to open file for writing" << std::endl;
            json response;
            response["success"] = false;
            response["error"] = "Failed to open file for writing";
            res.set_content(response.dump(), "application/json");
            res.status = 500;
            return;
        }
        std::cout << "[FILE] File opened successfully for writing" << std::endl;
        
        outfile << file_content;
        outfile.close();
        std::cout << "[FILE] File written and closed" << std::endl;
        
        std::cout << "[FILE] Saved configuration to: " << full_path << " (format: " << format << ")" << std::endl;
        
        json response;
        response["success"] = true;
        response["path"] = full_path;
        response["filename"] = filename;
        response["format"] = format;
        response["message"] = "Configuration saved to server backup";
        
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        
    } catch (const json::exception& e) {
        std::cerr << "[FILE] JSON Parse Error: " << e.what() << std::endl;
        std::cout << "[FILE] JSON Parse Error caught: " << e.what() << std::endl;
        
        json response;
        response["success"] = false;
        response["error"] = std::string("JSON parse error: ") + e.what();
        
        res.set_content(response.dump(), "application/json");
        res.status = 400;
        
    } catch (const std::exception& e) {
        std::cerr << "[FILE] Error: " << e.what() << std::endl;
        std::cout << "[FILE] Exception caught: " << e.what() << std::endl;
        
        json response;
        response["success"] = false;
        response["error"] = e.what();
        
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    } catch (...) {
        std::cerr << "[FILE] Unknown error occurred" << std::endl;
        std::cout << "[FILE] Unknown exception caught" << std::endl;
        
        json response;
        response["success"] = false;
        response["error"] = "Unknown error occurred";
        
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

void FileHandler::handle_list_configs(const httplib::Request& /*req*/, httplib::Response& res) {
    try {
        std::cout << "[FILE] Received GET /api/config/list" << std::endl;
        
        std::string storage_dir = get_storage_directory();
        
        json response;
        response["files"] = json::array();
        response["count"] = 0;
        
        // Check if directory exists
        if (!fs::exists(storage_dir)) {
            res.set_content(response.dump(), "application/json");
            res.status = 200;
            return;
        }
        
        // List all config files in directory
        try {
            for (const auto& entry : fs::directory_iterator(storage_dir)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    
                    // Include JSON/YAML/INI files
                    size_t pos = filename.rfind('.');
                    if (pos != std::string::npos) {
                        std::string ext = filename.substr(pos);
                        if (ext == ".json" || ext == ".yaml" || ext == ".yml" || ext == ".ini") {
                            json file_info;
                            file_info["filename"] = filename;
                            file_info["size"] = fs::file_size(entry);
                            response["files"].push_back(file_info);
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[FILE] Directory iteration error: " << e.what() << std::endl;
        }
        
        response["count"] = response["files"].size();
        
        std::cout << "[FILE] Listed " << response["count"] << " config files" << std::endl;
        
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        
    } catch (const std::exception& e) {
        std::cerr << "[FILE] Error: " << e.what() << std::endl;
        
        json response;
        response["success"] = false;
        response["error"] = e.what();
        
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

void FileHandler::handle_download_config(const httplib::Request& /*req*/,
                                         httplib::Response& res,
                                         const std::string& filename) {
    try {
        std::cout << "[FILE] Received GET /api/config/download/" << filename << std::endl;
        
        // Sanitize filename
        std::string sanitize_err = sanitize_filename(filename);
        if (!sanitize_err.empty()) {
            res.set_content("Invalid filename", "text/plain");
            res.status = 400;
            return;
        }
        
        std::string storage_dir = get_storage_directory();
        std::string full_path = storage_dir + "/" + filename;
        
        // Check if file exists
        if (!file_exists(full_path)) {
            json response;
            response["error"] = "File not found";
            res.set_content(response.dump(), "application/json");
            res.status = 404;
            return;
        }
        
        // Read file
        std::ifstream file(full_path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        std::cout << "[FILE] Sending file: " << full_path << std::endl;
        
        res.set_content(buffer.str(), "application/json");
        res.set_header("Content-Disposition", 
                      "attachment; filename=\"" + filename + "\"");
        res.status = 200;
        
    } catch (const std::exception& e) {
        std::cerr << "[FILE] Error: " << e.what() << std::endl;
        
        json response;
        response["error"] = e.what();
        
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

void FileHandler::handle_delete_config(const httplib::Request& /*req*/,
                                       httplib::Response& res,
                                       const std::string& filename) {
    try {
        std::cout << "[FILE] Received DELETE /api/config/" << filename << std::endl;
        
        // Sanitize filename
        std::string sanitize_err = sanitize_filename(filename);
        if (!sanitize_err.empty()) {
            json response;
            response["success"] = false;
            response["error"] = sanitize_err;
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        std::string storage_dir = get_storage_directory();
        std::string full_path = storage_dir + "/" + filename;
        
        // Check if file exists
        if (!file_exists(full_path)) {
            json response;
            response["success"] = false;
            response["error"] = "File not found";
            res.set_content(response.dump(), "application/json");
            res.status = 404;
            return;
        }
        
        // Delete file
        fs::remove(full_path);
        
        std::cout << "[FILE] Deleted file: " << full_path << std::endl;
        
        json response;
        response["success"] = true;
        response["message"] = "File deleted successfully from server";
        
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        
    } catch (const std::exception& e) {
        std::cerr << "[FILE] Error: " << e.what() << std::endl;
        
        json response;
        response["success"] = false;
        response["error"] = e.what();
        
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

} // namespace handlers
} // namespace configgui
