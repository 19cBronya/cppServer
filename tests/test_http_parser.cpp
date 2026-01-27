#include "../include/http_parser.h"
#include <iostream>
#include <cassert>
#include <string>

// 简单的测试框架
int passed = 0;
int failed = 0;

void test(const std::string& name, bool condition) {
    if (condition) {
        std::cout << "✓ " << name << std::endl;
        passed++;
    } else {
        std::cout << "✗ " << name << std::endl;
        failed++;
    }
}

void testHttpRequestParsing() {
    std::cout << "\n=== Testing HTTP Request Parsing ===" << std::endl;
    
    // Test 1: 简单的 GET 请求
    {
        std::string rawRequest = 
            "GET /test HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "User-Agent: Test\r\n"
            "\r\n";
        
        HttpRequest req = HttpParser::parseRequest(rawRequest);
        test("Parse GET method", req.method == "GET");
        test("Parse path", req.path == "/test");
        test("Parse version", req.version == "HTTP/1.1");
        test("Parse Host header", req.headers["Host"] == "localhost");
        test("Parse User-Agent header", req.headers["User-Agent"] == "Test");
        test("Request is valid", req.isValid());
    }
    
    // Test 2: POST 请求带 Body
    {
        std::string rawRequest = 
            "POST /echo HTTP/1.1\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 18\r\n"
            "\r\n"
            "{\"test\":\"hello\"}";
        
        HttpRequest req = HttpParser::parseRequest(rawRequest);
        test("Parse POST method", req.method == "POST");
        test("Parse path /echo", req.path == "/echo");
        test("Parse Content-Type", req.headers["Content-Type"] == "application/json");
        test("Parse body", req.body == "{\"test\":\"hello\"}");
    }
    
    // Test 3: 空请求
    {
        HttpRequest req = HttpParser::parseRequest("");
        test("Empty request is invalid", !req.isValid());
    }
}

void testHttpResponseCreation() {
    std::cout << "\n=== Testing HTTP Response Creation ===" << std::endl;
    
    // Test 1: 创建 200 响应
    {
        HttpResponse resp = HttpParser::createResponse(200, "OK");
        test("Response status code 200", resp.statusCode == 200);
        test("Response status text", resp.statusText == "OK");
        test("Response body", resp.body == "OK");
        
        std::string raw = resp.toString();
        test("Response contains HTTP/1.1", raw.find("HTTP/1.1") != std::string::npos);
        test("Response contains 200", raw.find("200") != std::string::npos);
        test("Response contains body", raw.find("OK") != std::string::npos);
    }
    
    // Test 2: 创建 404 响应
    {
        HttpResponse resp = HttpParser::createResponse(404, "Not Found");
        test("404 status code", resp.statusCode == 404);
        test("404 status text", resp.statusText == "Not Found");
    }
    
    // Test 3: 创建带 JSON 的响应
    {
        HttpResponse resp;
        resp.statusCode = 200;
        resp.statusText = "OK";
        resp.body = "{\"status\":\"ok\"}";
        resp.headers["Content-Type"] = "application/json";
        resp.headers["Content-Length"] = std::to_string(resp.body.length());
        
        std::string raw = resp.toString();
        test("JSON response contains body", raw.find("{\"status\":\"ok\"}") != std::string::npos);
        test("JSON response has Content-Type", raw.find("application/json") != std::string::npos);
    }
}

void testHttpResponseToString() {
    std::cout << "\n=== Testing HTTP Response toString ===" << std::endl;
    
    HttpResponse resp;
    resp.statusCode = 200;
    resp.statusText = "OK";
    resp.body = "Hello";
    resp.headers["Content-Type"] = "text/plain";
    resp.headers["Content-Length"] = "5";
    
    std::string raw = resp.toString();
    
    test("Contains status line", raw.find("HTTP/1.1 200 OK") != std::string::npos);
    test("Contains headers", raw.find("Content-Type: text/plain") != std::string::npos);
    test("Contains body separator", raw.find("\r\n\r\n") != std::string::npos);
    test("Contains body", raw.find("Hello") != std::string::npos);
}

int main() {
    std::cout << "Running HTTP Parser Tests..." << std::endl;
    
    testHttpRequestParsing();
    testHttpResponseCreation();
    testHttpResponseToString();
    
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;
    std::cout << "Total:  " << (passed + failed) << std::endl;
    
    return (failed == 0) ? 0 : 1;
}
