#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

PORT=${1:-8080}

echo "Starting ChatGPT Server on port $PORT..."

# 创建日志目录
mkdir -p "$PROJECT_ROOT/logs"

# 运行服务器（从项目根目录，这样可以找到 static/ 目录）
cd "$PROJECT_ROOT"
./build/bin/ChatGPTServer "$PORT"

