#include "server/router.h"
#include "logger/logger.h"
#include <sstream>

Router::Router() {
    // 设置默认 404 处理函数
    m_notFoundHandler = defaultNotFoundHandler;
}

void Router::addRoute(const std::string& method, const std::string& path, RouteHandler handler) {
    std::string key = makeRouteKey(method, path);
    m_routes[key] = handler;
    LOG_INFO("Route added: " + method + " " + path);
}

HttpResponse Router::handleRequest(const HttpRequest& request) {
    // 检查请求是否有效
    if (!request.isValid()) {
        LOG_WARN("Invalid HTTP request");
        HttpResponse response;
        response.statusCode = 400;
        response.statusText = "Bad Request";
        response.body = "Invalid HTTP request";
        response.headers["Content-Type"] = "text/plain";
        response.headers["Content-Length"] = std::to_string(response.body.length());
        return response;
    }
    
    // 查找路由
    std::string key = makeRouteKey(request.method, request.path);
    auto it = m_routes.find(key);
    
    if (it != m_routes.end()) {
        // 找到路由，执行处理函数
        LOG_INFO("Handling request: " + request.method + " " + request.path);
        try {
            return it->second(request);
        } catch (const std::exception& e) {
            LOG_ERROR("Error handling request: " + std::string(e.what()));
            HttpResponse response;
            response.statusCode = 500;
            response.statusText = "Internal Server Error";
            response.body = "Internal Server Error";
            response.headers["Content-Type"] = "text/plain";
            response.headers["Content-Length"] = std::to_string(response.body.length());
            return response;
        }
    } else {
        // 未找到路由，返回 404
        LOG_WARN("Route not found: " + request.method + " " + request.path);
        return m_notFoundHandler(request);
    }
}

void Router::setNotFoundHandler(RouteHandler handler) {
    m_notFoundHandler = handler;
}

std::string Router::makeRouteKey(const std::string& method, const std::string& path) const {
    return method + ":" + path;
}

HttpResponse Router::defaultNotFoundHandler(const HttpRequest& request) {
    HttpResponse response;
    response.statusCode = 404;
    response.statusText = "Not Found";
    
    std::ostringstream body;
    body << "404 Not Found\n";
    body << "Path: " << request.path << "\n";
    body << "Method: " << request.method;
    
    response.body = body.str();
    response.headers["Content-Type"] = "text/plain";
    response.headers["Content-Length"] = std::to_string(response.body.length());
    
    return response;
}
