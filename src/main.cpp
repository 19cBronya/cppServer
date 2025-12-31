#include "server/server.h"
#include "logger/logger.h"
#include "utils/signal_handler.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // 初始化日志
    Logger::getInstance().setLevel(LogLevel::INFO);
    Logger::getInstance().setLogFile("logs/server.log");
    
    LOG_INFO("=== ChatGPT Server Starting ===");
    
    // 创建服务器实例
    uint16_t port = 8080;
    if (argc > 1) {
        port = static_cast<uint16_t>(std::stoi(argv[1]));
    }
    
    Server server(port);
    
    if (!server.start()) {
        LOG_FATAL("Failed to start server");
        return 1;
    }
    
    // 运行服务器
    server.run();
    
    LOG_INFO("=== ChatGPT Server Stopped ===");
    return 0;
}

