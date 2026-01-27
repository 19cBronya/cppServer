#include "server/server.h"
#include "logger/logger.h"
#include "database/database.h"
#include "utils/signal_handler.h"
#include "utils/json_helper.h"
#include "http_parser.h"
#include <iostream>
#include <sstream>
#include <random>
#include <iomanip>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

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
    body << "  \"body\": \"" << JsonHelper::escape(req.body) << "\"\n";
    body << "}";
    
    HttpResponse response;
    response.statusCode = 200;
    response.statusText = "OK";
    response.body = body.str();
    response.headers["Content-Type"] = "application/json";
    response.headers["Content-Length"] = std::to_string(response.body.length());
    return response;
}

// 生成随机会话 ID
std::string generateSessionId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    const char* hex = "0123456789abcdef";
    std::string id;
    for (int i = 0; i < 16; ++i) {
        id += hex[dis(gen)];
    }
    return id;
}

// 聊天路由
HttpResponse handleChat(const HttpRequest& req) {
    HttpResponse response;
    response.headers["Content-Type"] = "application/json";
    
    // 解析请求
    auto data = JsonHelper::parseSimpleObject(req.body);
    std::string message = data["message"];
    std::string sessionId = data["session_id"];
    
    if (message.empty()) {
        response.statusCode = 400;
        response.statusText = "Bad Request";
        response.body = "{\"error\":\"Message is required\"}";
        response.headers["Content-Length"] = std::to_string(response.body.length());
        return response;
    }
    
    // 如果没有会话ID，创建新会话
    if (sessionId.empty()) {
        sessionId = generateSessionId();
        Database::getInstance().createSession(sessionId);
        LOG_INFO("Created new session: " + sessionId);
    } else if (!Database::getInstance().sessionExists(sessionId)) {
        Database::getInstance().createSession(sessionId);
        LOG_INFO("Created session: " + sessionId);
    }
    
    // 保存用户消息
    Database::getInstance().addMessage(sessionId, "user", message);
    LOG_INFO("User message saved to session " + sessionId);
    
    // 生成简单的回复（模拟 ChatGPT）
    std::string reply = "You said: " + message + ". This is a mock response from the server. In a real implementation, this would call an LLM API.";
    
    // 保存助手回复
    Database::getInstance().addMessage(sessionId, "assistant", reply);
    LOG_INFO("Assistant reply saved to session " + sessionId);
    
    // 构建响应
    std::ostringstream body;
    body << "{\n";
    body << "  \"session_id\": \"" << sessionId << "\",\n";
    body << "  \"message\": \"" << JsonHelper::escape(message) << "\",\n";
    body << "  \"reply\": \"" << JsonHelper::escape(reply) << "\"\n";
    body << "}";
    
    response.statusCode = 200;
    response.statusText = "OK";
    response.body = body.str();
    response.headers["Content-Length"] = std::to_string(response.body.length());
    return response;
}

// 获取会话历史
HttpResponse handleHistory(const HttpRequest& req) {
    HttpResponse response;
    response.headers["Content-Type"] = "application/json";
    
    // 从路径中提取 session_id
    // 格式: /history?session_id=xxx
    std::string query = req.path.substr(req.path.find('?') + 1);
    std::string sessionId;
    
    if (query.find("session_id=") == 0) {
        sessionId = query.substr(11);  // "session_id=" 长度为 11
    }
    
    if (sessionId.empty()) {
        response.statusCode = 400;
        response.statusText = "Bad Request";
        response.body = "{\"error\":\"session_id is required\"}";
        response.headers["Content-Length"] = std::to_string(response.body.length());
        return response;
    }
    
    // 获取消息历史
    auto messages = Database::getInstance().getMessages(sessionId);
    
    std::ostringstream body;
    body << "{\n";
    body << "  \"session_id\": \"" << sessionId << "\",\n";
    body << "  \"messages\": [\n";
    
    for (size_t i = 0; i < messages.size(); ++i) {
        body << "    {\n";
        body << "      \"role\": \"" << messages[i].role << "\",\n";
        body << "      \"content\": \"" << JsonHelper::escape(messages[i].content) << "\",\n";
        body << "      \"timestamp\": \"" << messages[i].timestamp << "\"\n";
        body << "    }";
        if (i < messages.size() - 1) {
            body << ",";
        }
        body << "\n";
    }
    
    body << "  ]\n";
    body << "}";
    
    response.statusCode = 200;
    response.statusText = "OK";
    response.body = body.str();
    response.headers["Content-Length"] = std::to_string(response.body.length());
    return response;
}

// Metrics 路由
HttpResponse handleMetrics(const HttpRequest& req) {
    Database& db = Database::getInstance();
    
    int totalSessions = db.getTotalSessions();
    int totalMessages = db.getTotalMessages();
    
    std::ostringstream body;
    body << "{\n";
    body << "  \"service\": \"ChatGPT Server\",\n";
    body << "  \"total_sessions\": " << totalSessions << ",\n";
    body << "  \"total_messages\": " << totalMessages << ",\n";
    body << "  \"uptime\": \"N/A\"\n";
    body << "}";
    
    HttpResponse response;
    response.statusCode = 200;
    response.statusText = "OK";
    response.body = body.str();
    response.headers["Content-Type"] = "application/json";
    response.headers["Content-Length"] = std::to_string(response.body.length());
    return response;
}

// 静态文件服务
HttpResponse serveStaticFile(const std::string& filepath) {
    HttpResponse response;
    
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        response.statusCode = 404;
        response.statusText = "Not Found";
        response.body = "File not found";
        response.headers["Content-Type"] = "text/plain";
        response.headers["Content-Length"] = std::to_string(response.body.length());
        return response;
    }
    
    // 读取文件内容
    std::ostringstream ss;
    ss << file.rdbuf();
    response.body = ss.str();
    
    // 设置 Content-Type
    std::string ext = filepath.substr(filepath.find_last_of('.') + 1);
    if (ext == "html") {
        response.headers["Content-Type"] = "text/html";
    } else if (ext == "css") {
        response.headers["Content-Type"] = "text/css";
    } else if (ext == "js") {
        response.headers["Content-Type"] = "application/javascript";
    } else {
        response.headers["Content-Type"] = "application/octet-stream";
    }
    
    response.statusCode = 200;
    response.statusText = "OK";
    response.headers["Content-Length"] = std::to_string(response.body.length());
    return response;
}

// 根路径路由 - 提供聊天界面
HttpResponse handleRoot(const HttpRequest& req) {
    // 尝试提供 static/index.html
    if (fs::exists("static/index.html")) {
        return serveStaticFile("static/index.html");
    }
    
    // 如果没有静态文件，显示欢迎信息
    HttpResponse response;
    response.statusCode = 200;
    response.statusText = "OK";
    response.body = "Welcome to ChatGPT Server!\n\nAvailable endpoints:\n"
                   "  GET  /            - Chat UI (if static/index.html exists)\n"
                   "  GET  /health      - Health check\n"
                   "  POST /chat        - Send a message\n"
                   "  GET  /history     - Get session history\n"
                   "  GET  /metrics     - System metrics\n"
                   "  POST /echo        - Echo request data\n";
    response.headers["Content-Type"] = "text/plain";
    response.headers["Content-Length"] = std::to_string(response.body.length());
    return response;
}

int main(int argc, char* argv[]) {
    // 初始化日志
    Logger::getInstance().setLevel(LogLevel::INFO);
    Logger::getInstance().setLogFile("logs/server.log");
    Logger::getInstance().setMaxFileSize(10 * 1024 * 1024);  // 10MB
    Logger::getInstance().setMaxFiles(5);
    
    LOG_INFO("=== ChatGPT Server Starting ===");
    
    // 初始化数据库
    if (!Database::getInstance().initialize("data/chat.db")) {
        LOG_FATAL("Failed to initialize database");
        return 1;
    }
    LOG_INFO("Database initialized successfully");
    
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
    server.getRouter().addRoute("POST", "/chat", handleChat);
    server.getRouter().addRoute("GET", "/history", handleHistory);
    server.getRouter().addRoute("GET", "/metrics", handleMetrics);
    server.getRouter().addRoute("POST", "/echo", handleEcho);
    server.getRouter().addRoute("GET", "/echo", handleEcho);
    
    if (!server.start()) {
        LOG_FATAL("Failed to start server");
        return 1;
    }
    
    LOG_INFO("Routes registered successfully");
    LOG_INFO("Available endpoints:");
    LOG_INFO("  GET  /            - Chat UI (if static/index.html exists)");
    LOG_INFO("  GET  /health      - Health check");
    LOG_INFO("  POST /chat        - Send a message");
    LOG_INFO("  GET  /history     - Get session history");
    LOG_INFO("  GET  /metrics     - System metrics");
    LOG_INFO("  POST /echo        - Echo request data");
    
    // 运行服务器
    server.run();
    
    LOG_INFO("=== ChatGPT Server Stopped ===");
    Database::getInstance().close();
    return 0;
}

