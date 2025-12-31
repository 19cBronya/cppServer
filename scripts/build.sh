#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "Building ChatGPT Server..."

cd "$PROJECT_ROOT"

# 创建构建目录
mkdir -p build
cd build

# 运行 CMake
cmake ..

# 编译
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable: $PROJECT_ROOT/build/bin/ChatGPTServer"

