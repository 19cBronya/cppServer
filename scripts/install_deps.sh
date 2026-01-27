#!/bin/bash

# 安装项目依赖脚本

echo "======================================"
echo "   安装 ChatGPT Server 依赖"
echo "======================================"
echo ""

echo "需要安装以下依赖:"
echo "  - libsqlite3-dev (SQLite3 开发库)"
echo ""

# 检查是否有 sudo 权限
if sudo -n true 2>/dev/null; then
    echo "检测到 sudo 权限，开始安装..."
    sudo apt-get update
    sudo apt-get install -y libsqlite3-dev
    echo ""
    echo "✅ 依赖安装完成！"
else
    echo "⚠️  需要 sudo 权限来安装依赖。"
    echo ""
    echo "请手动运行以下命令:"
    echo ""
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y libsqlite3-dev"
    echo ""
    echo "然后重新运行 ./scripts/build.sh 进行编译。"
fi
