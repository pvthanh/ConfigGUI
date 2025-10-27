#include "http_server.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <regex>

namespace configgui::html {

HttpServer::HttpServer(int port, const std::string& host)
    : port_(port), host_(host), server_(std::make_unique<httplib::Server>()), running_(false) {
    setupDefaultHandlers();
    setupLogging();
    setupCORS();
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::get(const std::string& path, RequestHandler handler) {
    if (server_) {
        server_->Get(path, [handler](const httplib::Request& req, httplib::Response& res) {
            handler(req, res);
        });
    }
}

void HttpServer::post(const std::string& path, RequestHandler handler) {
    if (server_) {
        server_->Post(path, [handler](const httplib::Request& req, httplib::Response& res) {
            handler(req, res);
        });
    }
}

void HttpServer::put(const std::string& path, RequestHandler handler) {
    if (server_) {
        server_->Put(path, [handler](const httplib::Request& req, httplib::Response& res) {
            handler(req, res);
        });
    }
}

void HttpServer::del(const std::string& path, RequestHandler handler) {
    if (server_) {
        server_->Delete(path, [handler](const httplib::Request& req, httplib::Response& res) {
            handler(req, res);
        });
    }
}

void HttpServer::get_regex(const std::string& pattern, RequestHandler handler) {
    if (server_) {
        // For cpp-httplib v0.11, we use a workaround - register routes manually in main.cpp
        // This function is kept for API compatibility but delegates to main.cpp to handle
        (void)pattern;
        (void)handler;
    }
}

void HttpServer::setErrorHandler(ErrorHandler handler) {
    if (server_) {
        server_->set_error_handler([handler](const httplib::Request& req, httplib::Response& res) {
            handler(req, res);
        });
    }
}

bool HttpServer::start() {
    if (!server_) {
        std::cerr << "❌ Server not initialized" << std::endl;
        return false;
    }

    running_ = true;

    // Log startup information
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::cout << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
    std::cout << "🚀 Starting ConfigGUI HTML Server..." << std::endl;
    std::cout << "   Listening on http://" << host_ << ":" << port_ << std::endl;

    // Bind and listen (blocking)
    bool success = server_->listen(host_.c_str(), port_);

    if (!success) {
        std::cerr << "❌ Failed to start server on " << host_ << ":" << port_ << std::endl;
        running_ = false;
        return false;
    }

    running_ = true;
    std::cout << "✅ Server started successfully" << std::endl;

    return true;
}

void HttpServer::stop() {
    if (server_ && running_) {
        std::cout << "🛑 Stopping HTTP server..." << std::endl;
        server_->stop();
        running_ = false;
        std::cout << "✅ Server stopped" << std::endl;
    }
}

bool HttpServer::isRunning() const {
    return running_;
}

void HttpServer::setupDefaultHandlers() {
    if (!server_) return;

    // Default 404 handler
    server_->set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        res.status = 404;
        res.set_content(R"({"error":"Not Found","path":")" + req.path + R"("})", "application/json");
    });

    // Default 500 handler
    server_->set_exception_handler([](const httplib::Request& req, httplib::Response& res, std::exception_ptr ep) {
        res.status = 500;
        res.set_content(R"({"error":"Internal Server Error"})", "application/json");
    });
}

void HttpServer::setupLogging() {
    if (!server_) return;

    server_->set_post_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::cout << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] ";
        std::cout << req.method << " " << req.path << " → " << res.status << std::endl;
    });
}

void HttpServer::setupCORS() {
    if (!server_) return;

    // Set CORS headers for all responses
    server_->set_post_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Accept");
    });

    // Handle OPTIONS requests
    server_->Options("/.*", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Accept");
        res.status = 200;
    });
}

} // namespace configgui::html
