#ifndef CONFIGGUI_HTML_HANDLERS_REQUEST_HANDLER_H
#define CONFIGGUI_HTML_HANDLERS_REQUEST_HANDLER_H

#include "httplib.h"
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace configgui::html {

using json = nlohmann::json;

/**
 * @brief Common request handler utilities
 * 
 * Provides helper functions for processing HTTP requests and generating
 * standardized JSON responses.
 */
class RequestHandler {
public:
    /**
     * @brief Send a successful JSON response
     * @param res HTTP response object
     * @param data JSON data to send
     * @param status HTTP status code (default: 200)
     */
    static void sendJson(httplib::Response& res, const json& data, int status = 200);

    /**
     * @brief Send an error JSON response
     * @param res HTTP response object
     * @param error Error message
     * @param status HTTP status code (default: 400)
     * @param details Optional additional error details
     */
    static void sendError(httplib::Response& res, const std::string& error, 
                         int status = 400, const json& details = json::object());

    /**
     * @brief Send HTML content
     * @param res HTTP response object
     * @param content HTML content to send
     * @param status HTTP status code (default: 200)
     */
    static void sendHtml(httplib::Response& res, const std::string& content, int status = 200);

    /**
     * @brief Send CSS content
     * @param res HTTP response object
     * @param content CSS content to send
     */
    static void sendCss(httplib::Response& res, const std::string& content);

    /**
     * @brief Send JavaScript content
     * @param res HTTP response object
     * @param content JavaScript content to send
     */
    static void sendJavaScript(httplib::Response& res, const std::string& content);

    /**
     * @brief Parse JSON from request body
     * @param req HTTP request object
     * @return Parsed JSON object, or null if parsing fails
     */
    static json parseJsonBody(const httplib::Request& req);

    /**
     * @brief Get content type from file extension
     * @param path File path
     * @return MIME type string
     */
    static std::string getMimeType(const std::string& path);

    /**
     * @brief Check if request has required JSON body
     * @param req HTTP request object
     * @param res HTTP response object
     * @return true if body exists and is valid JSON
     */
    static bool validateJsonBody(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Log request information
     * @param method HTTP method
     * @param path Request path
     * @param status Response status
     */
    static void logRequest(const std::string& method, const std::string& path, int status);
};

} // namespace configgui::html

#endif // CONFIGGUI_HTML_HANDLERS_REQUEST_HANDLER_H
