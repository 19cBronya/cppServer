#include "../include/server/router.h"
#include "../include/http_parser.h"
#include <iostream>
#include <cassert>

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

// 测试用的处理函数
HttpResponse testHandler(const HttpRequest& req) {
    HttpResponse resp;
    resp.statusCode = 200;
    resp.statusText = "OK";
    resp.body = "Test OK";
    return resp;
}

HttpResponse echoHandler(const HttpRequest& req) {
    HttpResponse resp;
    resp.statusCode = 200;
    resp.statusText = "OK";
    resp.body = "Echo: " + req.path;
    return resp;
}

void testRouteRegistration() {
    std::cout << "\n=== Testing Route Registration ===" << std::endl;
    
    Router router;
    
    // 注册路由
    router.addRoute("GET", "/test", testHandler);
    router.addRoute("POST", "/echo", echoHandler);
    
    std::cout << "✓ Routes registered successfully" << std::endl;
    passed++;
}

void testRouteMatching() {
    std::cout << "\n=== Testing Route Matching ===" << std::endl;
    
    Router router;
    router.addRoute("GET", "/test", testHandler);
    router.addRoute("POST", "/echo", echoHandler);
    
    // Test 1: 匹配 GET /test
    {
        HttpRequest req;
        req.method = "GET";
        req.path = "/test";
        req.version = "HTTP/1.1";
        
        HttpResponse resp = router.handleRequest(req);
        test("GET /test returns 200", resp.statusCode == 200);
        test("GET /test returns correct body", resp.body == "Test OK");
    }
    
    // Test 2: 匹配 POST /echo
    {
        HttpRequest req;
        req.method = "POST";
        req.path = "/echo";
        req.version = "HTTP/1.1";
        
        HttpResponse resp = router.handleRequest(req);
        test("POST /echo returns 200", resp.statusCode == 200);
        test("POST /echo returns correct body", resp.body.find("Echo:") != std::string::npos);
    }
    
    // Test 3: 不存在的路由返回 404
    {
        HttpRequest req;
        req.method = "GET";
        req.path = "/notfound";
        req.version = "HTTP/1.1";
        
        HttpResponse resp = router.handleRequest(req);
        test("Non-existent route returns 404", resp.statusCode == 404);
    }
    
    // Test 4: 方法不匹配返回 404
    {
        HttpRequest req;
        req.method = "DELETE";  // 没有注册 DELETE 方法
        req.path = "/test";
        req.version = "HTTP/1.1";
        
        HttpResponse resp = router.handleRequest(req);
        test("Wrong method returns 404", resp.statusCode == 404);
    }
}

void testInvalidRequest() {
    std::cout << "\n=== Testing Invalid Request Handling ===" << std::endl;
    
    Router router;
    router.addRoute("GET", "/test", testHandler);
    
    // 无效的请求（缺少 method）
    HttpRequest req;
    req.method = "";
    req.path = "";
    
    HttpResponse resp = router.handleRequest(req);
    test("Invalid request returns 400", resp.statusCode == 400);
}

void testCustomNotFoundHandler() {
    std::cout << "\n=== Testing Custom 404 Handler ===" << std::endl;
    
    Router router;
    
    // 自定义 404 处理函数
    router.setNotFoundHandler([](const HttpRequest& req) {
        HttpResponse resp;
        resp.statusCode = 404;
        resp.statusText = "Not Found";
        resp.body = "Custom 404: " + req.path + " not found";
        return resp;
    });
    
    HttpRequest req;
    req.method = "GET";
    req.path = "/custom404";
    req.version = "HTTP/1.1";
    
    HttpResponse resp = router.handleRequest(req);
    test("Custom 404 handler is called", resp.body.find("Custom 404") != std::string::npos);
}

int main() {
    std::cout << "Running Router Tests..." << std::endl;
    
    testRouteRegistration();
    testRouteMatching();
    testInvalidRequest();
    testCustomNotFoundHandler();
    
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;
    std::cout << "Total:  " << (passed + failed) << std::endl;
    
    return (failed == 0) ? 0 : 1;
}
