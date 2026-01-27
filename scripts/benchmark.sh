#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# 配置
PORT=8084
NUM_THREADS=4
SERVER_BIN="$PROJECT_ROOT/build/bin/ChatGPTServer"
SERVER_PID=""

# 颜色输出
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# 清理函数
cleanup() {
    if [ ! -z "$SERVER_PID" ]; then
        echo -e "${YELLOW}Stopping server...${NC}"
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
}

trap cleanup EXIT

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  ChatGPT Server Benchmark (Milestone 2)${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# 检查依赖
if ! command -v ab &> /dev/null; then
    echo -e "${YELLOW}Warning: 'ab' (Apache Bench) not found. Installing...${NC}"
    echo "Please install apache2-utils: sudo apt-get install apache2-utils"
    echo "Trying 'wrk' as fallback..."
fi

# 启动服务器
echo -e "${YELLOW}Starting server (port=$PORT, threads=$NUM_THREADS)...${NC}"
mkdir -p "$PROJECT_ROOT/logs"
$SERVER_BIN $PORT $NUM_THREADS > "$PROJECT_ROOT/logs/benchmark_server.log" 2>&1 &
SERVER_PID=$!

echo "Server PID: $SERVER_PID"
sleep 3

# 检查服务器是否启动
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}✗ Server failed to start${NC}"
    cat "$PROJECT_ROOT/logs/benchmark_server.log"
    exit 1
fi

echo -e "${GREEN}✓ Server started${NC}"
echo ""

# 基准测试函数
run_benchmark() {
    local NAME="$1"
    local URL="$2"
    local CONCURRENCY="$3"
    local REQUESTS="$4"
    
    echo -e "${BLUE}=== $NAME ===${NC}"
    echo "URL: $URL"
    echo "Concurrency: $CONCURRENCY"
    echo "Total Requests: $REQUESTS"
    echo ""
    
    if command -v ab &> /dev/null; then
        ab -c $CONCURRENCY -n $REQUESTS -q "$URL" 2>&1 | grep -E "(Requests per second|Time per request|Percentage of the requests|Failed requests)"
    elif command -v wrk &> /dev/null; then
        echo "Using wrk (note: request count is approximate)"
        wrk -c $CONCURRENCY -t 4 -d 10s "$URL"
    else
        echo "Neither 'ab' nor 'wrk' found. Using simple curl loop..."
        local START=$(date +%s)
        for i in $(seq 1 $REQUESTS); do
            curl -s "$URL" > /dev/null &
            if [ $((i % CONCURRENCY)) -eq 0 ]; then
                wait
            fi
        done
        wait
        local END=$(date +%s)
        local DURATION=$((END - START))
        echo "Duration: ${DURATION}s"
        echo "Requests/sec: $((REQUESTS / DURATION))"
    fi
    
    echo ""
}

# 热身
echo -e "${YELLOW}Warming up...${NC}"
for i in {1..10}; do
    curl -s "http://localhost:$PORT/health" > /dev/null
done
echo ""

# 基准测试 1: 低并发
run_benchmark "Test 1: Low Concurrency" "http://localhost:$PORT/health" 10 1000

# 基准测试 2: 中等并发
run_benchmark "Test 2: Medium Concurrency" "http://localhost:$PORT/health" 50 5000

# 基准测试 3: 高并发
run_benchmark "Test 3: High Concurrency" "http://localhost:$PORT/health" 100 10000

# 基准测试 4: POST 请求
if command -v ab &> /dev/null; then
    echo -e "${BLUE}=== Test 4: POST Requests ===${NC}"
    echo "URL: http://localhost:$PORT/echo"
    echo "Concurrency: 50"
    echo "Total Requests: 2000"
    echo ""
    
    # 创建临时 POST 数据文件
    echo '{"test":"benchmark"}' > /tmp/post_data.json
    ab -c 50 -n 2000 -p /tmp/post_data.json -T "application/json" -q "http://localhost:$PORT/echo" 2>&1 | grep -E "(Requests per second|Time per request|Percentage of the requests|Failed requests)"
    rm -f /tmp/post_data.json
    echo ""
fi

# 检查服务器日志中的统计信息
echo -e "${BLUE}=== Server Statistics ===${NC}"
sleep 2
tail -20 "$PROJECT_ROOT/logs/benchmark_server.log" | grep -E "(Connection|Thread|Rejected)" || echo "No statistics found in log"

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Benchmark Complete${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Server log: $PROJECT_ROOT/logs/benchmark_server.log"
echo ""
echo "Summary:"
echo "  - Tested low (10), medium (50), and high (100) concurrency"
echo "  - Total requests: ~16000"
echo "  - Server handled with epoll + thread pool"
echo ""
