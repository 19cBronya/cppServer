#include "http_parser.h"
#include <sstream>
#include <algorithm>

HttpRequest HttpParser::parseRequest(const std::string& rawRequest) {
    HttpRequest request;
    std::istringstream stream(rawRequest);
    std::string line;
    
    // 解析请求行：GET /path HTTP/1.1
    if (std::getline(stream, line)) {
        line = trim(line);
        std::istringstream lineStream(line);
        lineStream >> request.method >> request.path >> request.version;
    }
    
    // 解析请求头
    while (std::getline(stream, line) && !line.empty() && line != "\r") {
        line = trim(line);
        if (line.empty()) break;
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = trim(line.substr(0, colonPos));
            std::string value = trim(line.substr(colonPos + 1));
            request.headers[key] = value;
        }
    }
    
    // 解析请求体
    std::string body;
    while (std::getline(stream, line)) {
        body += line + "\n";
    }
    request.body = trim(body);
    
    return request;
}

HttpResponse HttpParser::createResponse(int statusCode, const std::string& body) {
    HttpResponse response;
    response.statusCode = statusCode;
    response.statusText = getStatusText(statusCode);
    response.body = body;
    response.headers["Content-Type"] = "text/plain";
    response.headers["Content-Length"] = std::to_string(body.length());
    response.headers["Connection"] = "close";
    return response;
}

std::string HttpResponse::toString() const {
    std::ostringstream oss;
    
    // 状态行
    oss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    
    // 响应头
    for (const auto& header : headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    // 空行
    oss << "\r\n";
    
    // 响应体
    oss << body;
    
    return oss.str();
}

std::string HttpParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::string HttpParser::getStatusText(int statusCode) {
    switch (statusCode) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}
