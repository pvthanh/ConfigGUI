#include "request_handler.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>

namespace configgui::html {

void RequestHandler::sendJson(httplib::Response& res, const json& data, int status) {
    res.status = status;
    res.set_content(data.dump(2), "application/json");
    res.set_header("Content-Type", "application/json; charset=utf-8");
}

void RequestHandler::sendError(httplib::Response& res, const std::string& error,
                               int status, const json& details) {
    json response = {
        {"error", error},
        {"status", status}
    };

    if (!details.empty()) {
        response["details"] = details;
    }

    sendJson(res, response, status);
}

void RequestHandler::sendHtml(httplib::Response& res, const std::string& content, int status) {
    res.status = status;
    res.set_content(content, "text/html");
    res.set_header("Content-Type", "text/html; charset=utf-8");
}

void RequestHandler::sendCss(httplib::Response& res, const std::string& content) {
    res.status = 200;
    res.set_content(content, "text/css");
    res.set_header("Content-Type", "text/css; charset=utf-8");
}

void RequestHandler::sendJavaScript(httplib::Response& res, const std::string& content) {
    res.status = 200;
    res.set_content(content, "application/javascript");
    res.set_header("Content-Type", "application/javascript; charset=utf-8");
}

json RequestHandler::parseJsonBody(const httplib::Request& req) {
    try {
        if (req.body.empty()) {
            return json();
        }
        return json::parse(req.body);
    } catch (const json::exception& e) {
        std::cerr << "❌ JSON parse error: " << e.what() << std::endl;
        return json();
    }
}

std::string RequestHandler::getMimeType(const std::string& path) {
    // Find the last dot
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream";
    }

    std::string ext = path.substr(dotPos);

    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), 
                   [](unsigned char c) { return std::tolower(c); });

    // Map extensions to MIME types
    static const std::unordered_map<std::string, std::string> mimeTypes = {
        // Web assets
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".svg", "image/svg+xml"},

        // Images
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".ico", "image/x-icon"},
        {".webp", "image/webp"},

        // Fonts
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"},
        {".eot", "application/vnd.ms-fontobject"},

        // Documents
        {".pdf", "application/pdf"},
        {".txt", "text/plain"},
        {".csv", "text/csv"},

        // Data
        {".yaml", "application/x-yaml"},
        {".yml", "application/x-yaml"},
    };

    auto it = mimeTypes.find(ext);
    return (it != mimeTypes.end()) ? it->second : "application/octet-stream";
}

bool RequestHandler::validateJsonBody(const httplib::Request& req, httplib::Response& res) {
    if (req.body.empty()) {
        sendError(res, "Request body is empty", 400);
        return false;
    }

    try {
        auto _ = json::parse(req.body);  // Suppress unused warning
        (void)_;  // Explicitly mark as intentionally unused
        return true;
    } catch (const json::exception& e) {
        sendError(res, "Invalid JSON in request body", 400,
                 json{{"parseError", e.what()}});
        return false;
    }
}

void RequestHandler::logRequest(const std::string& method, const std::string& path, int status) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::cout << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
    std::cout << method << " " << path << " → " << status;

    // Add status emoji
    if (status >= 200 && status < 300) {
        std::cout << " ✅";
    } else if (status >= 300 && status < 400) {
        std::cout << " ➡️";
    } else if (status >= 400 && status < 500) {
        std::cout << " ⚠️";
    } else if (status >= 500) {
        std::cout << " ❌";
    }

    std::cout << std::endl;
}

} // namespace configgui::html
