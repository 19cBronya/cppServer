#pragma once

#include <string>
#include <map>

// HTTP 请求结构
struct HttpRequest {
    std::string method;      // GET, POST, etc.
    std::string path;        // /chat, /health, etc.
    std::string version;     // HTTP/1.1
    std::map<std::string, std::string> headers;
    std::string body;
    
    bool isValid() const { return !method.empty() && !path.empty(); }
};

// HTTP 响应结构
struct HttpResponse {
    int statusCode;          // 200, 404, etc.
    std::string statusText;  // OK, Not Found, etc.
    std::map<std::string, std::string> headers;
    std::string body;
    
    HttpResponse() : statusCode(200), statusText("OK") {}
    
    // 转换为 HTTP 响应字符串
    std::string toString() const;
};

// HTTP 解析器
class HttpParser {
public:
    // 解析 HTTP 请求
    static HttpRequest parseRequest(const std::string& rawRequest);
    
    // 创建 HTTP 响应
    static HttpResponse createResponse(int statusCode, const std::string& body);
    
private:
    static std::string trim(const std::string& str);
    static std::string getStatusText(int statusCode);
};
