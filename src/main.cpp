#include "server/server.h"
#include "logger/logger.h"
#include "utils/signal_handler.h"
#include "http_parser.h"
#include <iostream>
#include <sstream>

// 健康检查路由
HttpResponse handleHealth(const HttpRequest& req) {
    HttpResponse response;
    response.statusCode = 200;
    response.statusText = "OK";
    response.body = "{\"status\":\"ok\",\"service\":\"ChatGPT Server\"}";
    response.headers["Content-Type"] = "application/json";
    response.headers["Content-Length"] = std::to_string(response.body.length());
    return response;
}

// Echo 路由（回显请求信息）
HttpResponse handleEcho(const HttpRequest& req) {
    std::ostringstream body;
    body << "{\n";
    body << "  \"method\": \"" << req.method << "\",\n";
    body << "  \"path\": \"" << req.path << "\",\n";
    body << "  \"body\": \"" << req.body << "\"\n";
    body << "}";
    
    HttpResponse response;
    response.statusCode = 200;
    response.statusText = "OK";
    response.body = body.str();
    response.headers["Content-Type"] = "application/json";
    response.headers["Content-Length"] = std::to_string(response.body.length());
    return response;
}

// 根路径路由
HttpResponse handleRoot(const HttpRequest& req) {
    HttpResponse response;
    response.statusCode = 200;
    response.statusText = "OK";
    response.body = "Welcome to ChatGPT Server!\n\nAvailable endpoints:\n"
                   "  GET  /         - This message\n"
                   "  GET  /health   - Health check\n"
                   "  POST /echo     - Echo request data\n";
    response.headers["Content-Type"] = "text/plain";
    response.headers["Content-Length"] = std::to_string(response.body.length());
    return response;
}

int main(int argc, char* argv[]) {
    // 初始化日志
    Logger::getInstance().setLevel(LogLevel::INFO);
    Logger::getInstance().setLogFile("logs/server.log");
    
    LOG_INFO("=== ChatGPT Server Starting ===");
    
    // 创建服务器实例
    uint16_t port = 8080;
    size_t numThreads = 0;  // 0 = 使用硬件并发数
    
    if (argc > 1) {
        port = static_cast<uint16_t>(std::stoi(argv[1]));
    }
    if (argc > 2) {
        numThreads = static_cast<size_t>(std::stoi(argv[2]));
    }
    
    Server server(port, numThreads);
    
    // 注册路由
    LOG_INFO("Registering routes...");
    server.getRouter().addRoute("GET", "/", handleRoot);
    server.getRouter().addRoute("GET", "/health", handleHealth);
    server.getRouter().addRoute("POST", "/echo", handleEcho);
    server.getRouter().addRoute("GET", "/echo", handleEcho);  // 也支持 GET
    
    if (!server.start()) {
        LOG_FATAL("Failed to start server");
        return 1;
    }
    
    LOG_INFO("Routes registered successfully");
    LOG_INFO("Available endpoints:");
    LOG_INFO("  GET  /         - Root endpoint");
    LOG_INFO("  GET  /health   - Health check");
    LOG_INFO("  POST /echo     - Echo request data");
    
    // 运行服务器
    server.run();
    
    LOG_INFO("=== ChatGPT Server Stopped ===");
    return 0;
}

