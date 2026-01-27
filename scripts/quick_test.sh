#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# 配置
PORT=8081  # 使用不同端口避免冲突
SERVER_BIN="$PROJECT_ROOT/build/bin/ChatGPTServer"
SERVER_PID=""

# 颜色输出
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 清理函数
cleanup() {
    if [ ! -z "$SERVER_PID" ]; then
        echo -e "${YELLOW}Stopping server (PID: $SERVER_PID)...${NC}"
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
}

trap cleanup EXIT

echo "=== Quick Integration Test ==="
echo ""

# 启动服务器
echo "Starting server on port $PORT..."
mkdir -p "$PROJECT_ROOT/logs"
$SERVER_BIN $PORT > /tmp/server_test.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

# 等待服务器启动
sleep 2

# 检查服务器是否在运行
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}✗ Server failed to start${NC}"
    cat /tmp/server_test.log
    exit 1
fi

echo -e "${GREEN}✓ Server started${NC}"
echo ""

# 测试函数
test_request() {
    local NAME="$1"
    local URL="$2"
    local METHOD="${3:-GET}"
    
    echo -n "Testing $NAME... "
    
    if [ "$METHOD" = "POST" ]; then
        RESPONSE=$(curl -s -w "\n%{http_code}" -X POST "$URL" \
            -H "Content-Type: application/json" \
            -d '{"test":"data"}' 2>&1)
    else
        RESPONSE=$(curl -s -w "\n%{http_code}" "$URL" 2>&1)
    fi
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n 1)
    
    if [ "$HTTP_CODE" = "200" ] || [ "$HTTP_CODE" = "404" ]; then
        echo -e "${GREEN}✓ (HTTP $HTTP_CODE)${NC}"
        return 0
    else
        echo -e "${RED}✗ (HTTP $HTTP_CODE)${NC}"
        echo "$RESPONSE"
        return 1
    fi
}

# 运行测试
PASSED=0
FAILED=0

test_request "GET /health" "http://localhost:$PORT/health" && PASSED=$((PASSED+1)) || FAILED=$((FAILED+1))
sleep 1

test_request "GET /" "http://localhost:$PORT/" && PASSED=$((PASSED+1)) || FAILED=$((FAILED+1))
sleep 1

test_request "POST /echo" "http://localhost:$PORT/echo" "POST" && PASSED=$((PASSED+1)) || FAILED=$((FAILED+1))
sleep 1

test_request "GET /notfound" "http://localhost:$PORT/notfound" && PASSED=$((PASSED+1)) || FAILED=$((FAILED+1))

echo ""
echo "=== Results ==="
echo "Passed: $PASSED"
echo "Failed: $FAILED"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
