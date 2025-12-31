#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

PORT=${1:-8080}
SERVER_BIN="$PROJECT_ROOT/build/bin/ChatGPTServer"

echo "Testing ChatGPT Server on port $PORT..."

# 检查服务器是否已编译
if [ ! -f "$SERVER_BIN" ]; then
    echo "Error: Server binary not found. Please run 'make' first."
    exit 1
fi

# 创建日志目录
mkdir -p "$PROJECT_ROOT/logs"

# 启动服务器（后台运行）
echo "Starting server..."
$SERVER_BIN "$PORT" &
SERVER_PID=$!

# 等待服务器启动
sleep 2

# 检查服务器是否在运行
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "Error: Server failed to start"
    exit 1
fi

echo "Server started with PID: $SERVER_PID"

# 测试健康检查
echo ""
echo "Testing HTTP endpoint..."
RESPONSE=$(curl -s http://localhost:$PORT/ || echo "")

if [ -n "$RESPONSE" ]; then
    echo "✓ Server responded: $RESPONSE"
else
    echo "✗ Server did not respond"
    kill $SERVER_PID 2>/dev/null || true
    exit 1
fi

# 停止服务器
echo ""
echo "Stopping server..."
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo "Test completed successfully!"

