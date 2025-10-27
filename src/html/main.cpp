#include "server/http_server.h"
#include "handlers/request_handler.h"
#include "handlers/schema_service.h"
#include "handlers/file_handler.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <csignal>
#include <thread>
#include <regex>

using namespace configgui::html;

// Global server instance for signal handling
std::unique_ptr<HttpServer> g_server;

/**
 * Signal handler for graceful shutdown
 */
void signalHandler(int signal) {
    std::cout << "\n🛑 Received signal " << signal << ", shutting down gracefully..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

/**
 * Print usage information
 */
void printUsage(const char* programName) {
    std::cout << "ConfigGUI HTML Server - Web-based Configuration Manager\n\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --port PORT              Port to listen on (default: 8080)\n";
    std::cout << "  --host HOST              Host to bind to (default: localhost)\n";
    std::cout << "  --schema-dir DIR         Schema directory (default: resources/schemas)\n";
    std::cout << "  --config-dir DIR         Config directory (default: resources/configs)\n";
    std::cout << "  --help                   Show this help message\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << programName << " --port 8080 --host 0.0.0.0\n";
}

/**
 * Parse command line arguments
 */
struct Config {
    int port = 8080;
    std::string host = "localhost";
    std::string schemaDir = "resources/schemas";
    std::string configDir = "resources/configs";
};

Config parseArgs(int argc, char* argv[]) {
    Config config;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            printUsage(argv[0]);
            exit(0);
        } else if (arg == "--port" && i + 1 < argc) {
            config.port = std::atoi(argv[++i]);
        } else if (arg == "--host" && i + 1 < argc) {
            config.host = argv[++i];
        } else if (arg == "--schema-dir" && i + 1 < argc) {
            config.schemaDir = argv[++i];
        } else if (arg == "--config-dir" && i + 1 < argc) {
            config.configDir = argv[++i];
        } else if (arg[0] == '-') {
            std::cerr << "❌ Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            exit(1);
        }
    }

    return config;
}

/**
 * Try to load HTML file from multiple possible locations
 */
std::string getIndexHtmlPath() {
    // Try multiple possible locations
    const std::vector<std::string> possibilities = {
        "src/html/assets/index.html",
        "../src/html/assets/index.html",
        "../../src/html/assets/index.html",
        "./assets/index.html",
        "/src/html/assets/index.html"
    };

    for (const auto& path : possibilities) {
        std::ifstream file(path);
        if (file.good()) {
            return path;
        }
    }
    
    return "";  // Not found
}

/**
 * Load HTML file content
 */
std::string loadHtmlFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * Main entry point
 */
int main(int argc, char* argv[]) {
    // Parse command line arguments
    Config config = parseArgs(argc, argv);

    // Print banner
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                           ║\n";
    std::cout << "║       ConfigGUI - HTML Form Generation Server             ║\n";
    std::cout << "║                                                           ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";

    // Create HTTP server
    g_server = std::make_unique<HttpServer>(config.port, config.host);

    std::cout << "📋 Configuration:\n";
    std::cout << "   Host: " << config.host << "\n";
    std::cout << "   Port: " << config.port << "\n";
    std::cout << "   Schema Directory: " << config.schemaDir << "\n";
    std::cout << "   Config Directory: " << config.configDir << "\n\n";

    // Register signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);   // Ctrl+C
    std::signal(SIGTERM, signalHandler);  // Termination signal

    // Try to load index.html
    std::string indexHtmlPath = getIndexHtmlPath();
    std::string indexHtmlContent;
    
    if (!indexHtmlPath.empty()) {
        indexHtmlContent = loadHtmlFile(indexHtmlPath);
        if (!indexHtmlContent.empty()) {
            std::cout << "📄 Loaded index.html from: " << indexHtmlPath << "\n\n";
        } else {
            std::cout << "⚠️  Found index.html but failed to read it\n\n";
        }
    } else {
        std::cout << "⚠️  index.html not found, using fallback landing page\n\n";
    }

    // Register basic routes
    g_server->get("/", [indexHtmlContent](const httplib::Request& req, httplib::Response& res) {
        // If we successfully loaded index.html, serve it
        if (!indexHtmlContent.empty()) {
            RequestHandler::sendHtml(res, indexHtmlContent);
            return;
        }
        
        // Fallback to landing page if index.html not available
        std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ConfigGUI - Web Configuration Manager</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container {
            background: white;
            border-radius: 8px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
            padding: 40px;
            max-width: 600px;
            text-align: center;
        }
        h1 {
            color: #333;
            margin-top: 0;
        }
        p {
            color: #666;
            line-height: 1.6;
        }
        .status {
            background: #e8f5e9;
            color: #2e7d32;
            padding: 15px;
            border-radius: 4px;
            margin: 20px 0;
        }
        .links {
            margin-top: 30px;
        }
        a {
            display: inline-block;
            margin: 10px;
            padding: 10px 20px;
            background: #667eea;
            color: white;
            text-decoration: none;
            border-radius: 4px;
            transition: background 0.3s;
        }
        a:hover {
            background: #764ba2;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎯 ConfigGUI</h1>
        <p>Web-based Configuration Management System</p>
        <div class="status">
            <strong>✅ Server is Running</strong><br>
            Ready to generate configuration forms from schemas
        </div>
        <div class="links">
            <a href="/api/schemas">📋 View Schemas</a>
            <a href="/main">🔧 Configuration Manager</a>
        </div>
    </div>
</body>
</html>
        )";
        RequestHandler::sendHtml(res, html);
    });

    // Health check endpoint
    g_server->get("/health", [](const httplib::Request& req, httplib::Response& res) {
        json response = {
            {"status", "healthy"},
            {"service", "ConfigGUI HTML Server"},
            {"version", "3.0"}
        };
        RequestHandler::sendJson(res, response);
    });

    // Schema discovery endpoint (real schema scanning)
    g_server->get("/api/schemas", [config](const httplib::Request& /*req*/, httplib::Response& res) {
        // Initialize schema service with configured directory
        SchemaService schemaService;
        if (!schemaService.initialize(config.schemaDir)) {
            // If schema directory doesn't exist, return empty list with warning
            json response = json::array();
            RequestHandler::sendJson(res, response, 200);
            return;
        }
        
        // Get list of all discovered schemas
        auto schemas = schemaService.listSchemas();
        
        // Convert to JSON array
        json response = json::array();
        for (const auto& schema : schemas) {
            response.push_back(schema.toJson());
        }
        
        RequestHandler::sendJson(res, response);
    });

    // Individual schema endpoint (get full schema by ID)
    // We'll handle this through query parameter for compatibility
    g_server->get("/api/schemas/get", [config](const httplib::Request& req, httplib::Response& res) {
        // Get schema ID from query parameter
        std::string schemaId;
        if (req.has_param("id")) {
            schemaId = req.get_param_value("id");
        } else {
            json errorResponse = SchemaService::createError("Missing 'id' parameter");
            RequestHandler::sendJson(res, errorResponse, 400);
            return;
        }
        
        // Initialize schema service with configured directory
        SchemaService schemaService;
        if (!schemaService.initialize(config.schemaDir)) {
            // Schema directory doesn't exist
            json errorResponse = SchemaService::createError("Schema directory not accessible");
            RequestHandler::sendJson(res, errorResponse, 404);
            return;
        }
        
        // Try to load the full schema
        json schema = schemaService.getSchema(schemaId);
        
        if (SchemaService::isError(schema)) {
            // Schema not found or parse error
            RequestHandler::sendJson(res, schema, 404);
            return;
        }
        
        // Success - return full schema
        RequestHandler::sendJson(res, schema);
    });

    // Static asset serving for CSS and JavaScript
    g_server->get("/main.css", [](const httplib::Request& /*req*/, httplib::Response& res) {
        const std::vector<std::string> cssPaths = {
            "src/html/assets/css/main.css",
            "../src/html/assets/css/main.css",
            "../../src/html/assets/css/main.css",
            "./assets/css/main.css"
        };
        
        for (const auto& path : cssPaths) {
            std::ifstream file(path);
            if (file.good()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                RequestHandler::sendCss(res, buffer.str());
                return;
            }
        }
        
        // Fallback: minimal CSS
        RequestHandler::sendCss(res, "body { font-family: sans-serif; padding: 20px; }");
    });

    g_server->get("/main.js", [](const httplib::Request& /*req*/, httplib::Response& res) {
        const std::vector<std::string> jsPaths = {
            "src/html/assets/js/main.js",
            "../src/html/assets/js/main.js",
            "../../src/html/assets/js/main.js",
            "./assets/js/main.js"
        };
        
        for (const auto& path : jsPaths) {
            std::ifstream file(path);
            if (file.good()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                RequestHandler::sendJavaScript(res, buffer.str());
                return;
            }
        }
        
        // Fallback: minimal JS console message
        RequestHandler::sendJavaScript(res, "console.log('main.js not found, using fallback');");
    });

    // ========== File Handler Routes (Hybrid Storage) ==========
    
    // POST /api/config/save - Save configuration file to server
    g_server->post("/api/config/save", [](const httplib::Request& req, httplib::Response& res) {
        configgui::handlers::FileHandler::handle_save_config(req, res);
    });
    
    // GET /api/config/list - List saved configurations on server
    g_server->get("/api/config/list", [](const httplib::Request& req, httplib::Response& res) {
        configgui::handlers::FileHandler::handle_list_configs(req, res);
    });
    
    // GET /api/config/download/{filename} - Download configuration from server
    // Use raw server pointer to register a catch-all handler using standard library regex
    auto* raw_server = g_server->getServer();
    if (raw_server) {
        // Register a handler that catches paths starting with /api/config/download/
        raw_server->Get(R"(/api/config/download/.*)", [](const httplib::Request& req, httplib::Response& res) {
            // Extract filename from path: /api/config/download/filename.json -> filename.json
            std::string path = req.path;
            std::string prefix = "/api/config/download/";
            if (path.find(prefix) == 0 && path.length() > prefix.length()) {
                std::string filename = path.substr(prefix.length());
                configgui::handlers::FileHandler::handle_download_config(req, res, filename);
                return;
            }
            res.status = 404;
            res.set_content(R"({"error":"Not Found"})", "application/json");
        });
        
        // DELETE /api/config/{filename} - Delete configuration from server
        raw_server->Delete(R"(/api/config/.*)", [](const httplib::Request& req, httplib::Response& res) {
            // Extract filename from path: /api/config/filename.json -> filename.json
            // Exclude /api/config/list as that's a GET endpoint
            std::string path = req.path;
            std::string prefix = "/api/config/";
            if (path.find(prefix) == 0 && path.length() > prefix.length()) {
                std::string filename = path.substr(prefix.length());
                // Don't allow deleting "list"
                if (filename == "list") {
                    res.status = 404;
                    res.set_content(R"({"error":"Not Found"})", "application/json");
                    return;
                }
                configgui::handlers::FileHandler::handle_delete_config(req, res, filename);
                return;
            }
            res.status = 404;
            res.set_content(R"({"error":"Not Found"})", "application/json");
        });
    }

    // Start server (blocking)
    if (!g_server->start()) {
        std::cerr << "❌ Failed to start server" << std::endl;
        return 1;
    }

    return 0;
}
