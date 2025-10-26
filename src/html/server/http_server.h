#ifndef CONFIGGUI_HTML_SERVER_HTTP_SERVER_H
#define CONFIGGUI_HTML_SERVER_HTTP_SERVER_H

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include "httplib.h"

namespace configgui::html {

/**
 * @brief HTTP Server for ConfigGUI HTML interface
 * 
 * Provides RESTful API endpoints and static asset serving for the web-based
 * ConfigGUI application. Uses cpp-httplib for HTTP server implementation.
 */
class HttpServer {
public:
    using RequestHandler = std::function<void(const httplib::Request&, httplib::Response&)>;
    using ErrorHandler = std::function<void(const httplib::Request&, httplib::Response&)>;

    /**
     * @brief Constructor
     * @param port Port number to listen on (default: 8080)
     * @param host Host to bind to (default: "localhost")
     */
    explicit HttpServer(int port = 8080, const std::string& host = "localhost");

    /**
     * @brief Destructor - ensures graceful shutdown
     */
    ~HttpServer();

    /**
     * @brief Register a GET endpoint
     * @param path URL path pattern
     * @param handler Request handler function
     */
    void get(const std::string& path, RequestHandler handler);

    /**
     * @brief Register a POST endpoint
     * @param path URL path pattern
     * @param handler Request handler function
     */
    void post(const std::string& path, RequestHandler handler);

    /**
     * @brief Register a PUT endpoint
     * @param path URL path pattern
     * @param handler Request handler function
     */
    void put(const std::string& path, RequestHandler handler);

    /**
     * @brief Register a DELETE endpoint
     * @param path URL path pattern
     * @param handler Request handler function
     */
    void del(const std::string& path, RequestHandler handler);

    /**
     * @brief Register a GET endpoint with regex pattern matching
     * @param pattern Regex pattern for URL path
     * @param handler Request handler function
     */
    void get_regex(const std::string& pattern, RequestHandler handler);

    /**
     * @brief Set custom error handler
     * @param handler Error handler function
     */
    void setErrorHandler(ErrorHandler handler);

    /**
     * @brief Start the HTTP server (blocking call)
     * @return true if server started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Stop the HTTP server gracefully
     */
    void stop();

    /**
     * @brief Check if server is running
     * @return true if server is currently running
     */
    bool isRunning() const;

    /**
     * @brief Get the port the server is listening on
     * @return Port number
     */
    int getPort() const { return port_; }

    /**
     * @brief Get the host the server is bound to
     * @return Host address
     */
    const std::string& getHost() const { return host_; }

private:
    int port_;
    std::string host_;
    std::unique_ptr<httplib::Server> server_;
    bool running_;

    /**
     * @brief Setup default error handlers and middleware
     */
    void setupDefaultHandlers();

    /**
     * @brief Setup logging middleware
     */
    void setupLogging();

    /**
     * @brief Setup CORS headers middleware
     */
    void setupCORS();
};

} // namespace configgui::html

#endif // CONFIGGUI_HTML_SERVER_HTTP_SERVER_H
