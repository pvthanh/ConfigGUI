#ifndef CONFIGGUI_FILE_HANDLER_H
#define CONFIGGUI_FILE_HANDLER_H

#include "httplib.h"
#include <string>
#include <vector>

namespace configgui {
namespace handlers {

class FileHandler {
public:
    /**
     * Handle POST /api/config/save - Save configuration to server
     * Request body: { "filename": "config.json", "data": {...} }
     * Response: { "success": true, "path": "/path/to/config.json", "message": "..." }
     */
    static void handle_save_config(const httplib::Request& req, httplib::Response& res);
    
    /**
     * Handle GET /api/config/list - List saved configurations
     * Response: { "files": ["config1.json", "config2.json"], "count": 2 }
     */
    static void handle_list_configs(const httplib::Request& req, httplib::Response& res);
    
    /**
     * Handle GET /api/config/download/{filename} - Download configuration
     * Response: File content (application/json)
     */
    static void handle_download_config(const httplib::Request& req, httplib::Response& res, const std::string& filename);
    
    /**
     * Handle DELETE /api/config/{filename} - Delete configuration from server
     * Response: { "success": true, "message": "..." }
     */
    static void handle_delete_config(const httplib::Request& req, httplib::Response& res, const std::string& filename);
    
private:
    // Get storage directory for configurations
    static std::string get_storage_directory();
    
    // Sanitize filename to prevent path traversal attacks
    // Returns error string if invalid, empty string if valid
    static std::string sanitize_filename(const std::string& filename);
    
    // Check if file exists
    static bool file_exists(const std::string& path);
    
    // Create directory if not exists
    // Returns error string if failed, empty string if success
    static std::string ensure_directory(const std::string& path);
};

} // namespace handlers
} // namespace configgui

#endif
