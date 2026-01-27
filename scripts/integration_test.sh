#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# 配置
PORT=8080
if [ ! -z "$1" ]; then
    PORT=$1
fi

SERVER_BIN="$PROJECT_ROOT/build/bin/ChatGPTServer"
SERVER_PID=""

# 颜色输出
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 清理函数
cleanup() {
    if [ ! -z "$SERVER_PID" ]; then
        echo -e "${YELLOW}Stopping server (PID: $SERVER_PID)...${NC}"
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
}

# 注册清理函数
trap cleanup EXIT

# 检查服务器是否存在
if [ ! -f "$SERVER_BIN" ]; then
    echo -e "${RED}✗ Server binary not found: $SERVER_BIN${NC}"
    echo "Please run: ./scripts/build.sh"
    exit 1
fi

echo "=== Integration Test ==="
echo "Port: $PORT"
echo ""

# 启动服务器
echo -e "${YELLOW}Starting server...${NC}"
mkdir -p "$PROJECT_ROOT/logs"
$SERVER_BIN $PORT > /dev/null 2>&1 &
SERVER_PID=$!

# 等待服务器启动
echo "Waiting for server to start..."
sleep 2

# 检查服务器是否在运行
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}✗ Server failed to start${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Server started (PID: $SERVER_PID)${NC}"
echo ""

# 测试计数
PASSED=0
FAILED=0

# 测试函数
test_endpoint() {
    local NAME="$1"
    local METHOD="$2"
    local PATH="$3"
    local EXPECTED_CODE="$4"
    local DATA="$5"
    
    echo -n "Testing $NAME... "
    
    if [ "$METHOD" = "POST" ]; then
        RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "http://localhost:$PORT$PATH" \
            -H "Content-Type: application/json" \
            -d "$DATA" 2>/dev/null)
    else
        RESPONSE=$(curl -s -w "\n%{http_code}" "http://localhost:$PORT$PATH" 2>/dev/null)
    fi
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n 1)
    BODY=$(echo "$RESPONSE" | head -n -1)
    
    if [ "$HTTP_CODE" = "$EXPECTED_CODE" ]; then
        echo -e "${GREEN}✓ (HTTP $HTTP_CODE)${NC}"
        PASSED=$((PASSED + 1))
        return 0
    else
        echo -e "${RED}✗ (Expected $EXPECTED_CODE, got $HTTP_CODE)${NC}"
        echo "  Response: $BODY"
        FAILED=$((FAILED + 1))
        return 1
    fi
}

# 运行测试
echo "=== Running Tests ==="

# Test 1: GET /
test_endpoint "GET /" "GET" "/" "200"

# Test 2: GET /health
test_endpoint "GET /health" "GET" "/health" "200"

# Test 3: POST /echo
test_endpoint "POST /echo" "POST" "/echo" "200" '{"message":"hello"}'

# Test 4: GET /echo
test_endpoint "GET /echo" "GET" "/echo" "200"

# Test 5: 404 Not Found
test_endpoint "GET /notfound (404)" "GET" "/notfound" "404"

# 详细测试：检查响应内容
echo ""
echo "=== Detailed Tests ==="

# 测试 /health 返回 JSON
echo -n "Testing /health returns JSON... "
HEALTH_RESPONSE=$(curl -s "http://localhost:$PORT/health")
if echo "$HEALTH_RESPONSE" | grep -q "status"; then
    echo -e "${GREEN}✓${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗${NC}"
    echo "  Response: $HEALTH_RESPONSE"
    FAILED=$((FAILED + 1))
fi

# 测试 /echo 回显
echo -n "Testing /echo returns request data... "
ECHO_RESPONSE=$(curl -s -X POST "http://localhost:$PORT/echo" \
    -H "Content-Type: application/json" \
    -d '{"test":"data"}')
if echo "$ECHO_RESPONSE" | grep -q "echo"; then
    echo -e "${GREEN}✓${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗${NC}"
    echo "  Response: $ECHO_RESPONSE"
    FAILED=$((FAILED + 1))
fi

# 测试结果
echo ""
echo "=== Test Results ==="
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo "Total:  $((PASSED + FAILED))"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
