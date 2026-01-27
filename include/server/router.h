#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <map>
#include <functional>
#include "../http_parser.h"

// 路由处理函数类型
using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

// 路由系统
class Router {
public:
    Router();
    
    // 添加路由：方法 + 路径 -> 处理函数
    void addRoute(const std::string& method, const std::string& path, RouteHandler handler);
    
    // 处理请求：根据请求找到对应的处理函数
    HttpResponse handleRequest(const HttpRequest& request);
    
    // 默认路由（404）
    void setNotFoundHandler(RouteHandler handler);
    
private:
    // 路由表：key = "METHOD:PATH", value = handler
    std::map<std::string, RouteHandler> m_routes;
    RouteHandler m_notFoundHandler;
    
    // 生成路由键
    std::string makeRouteKey(const std::string& method, const std::string& path) const;
    
    // 默认 404 处理函数
    static HttpResponse defaultNotFoundHandler(const HttpRequest& request);
};

#endif // ROUTER_H
