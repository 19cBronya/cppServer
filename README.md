# ChatGPT 高性能 C++ 服务器

基于 epoll + 线程池实现的高并发推理网关，支持 REST API、多轮会话、过载保护。

## 项目结构

```
cppServer/
├── CMakeLists.txt          # CMake 构建配置
├── README.md               # 项目说明文档
├── include/                # 头文件目录
│   ├── server/            # 服务器核心
│   ├── network/           # 网络层
│   ├── utils/             # 工具类
│   └── logger/            # 日志系统
├── src/                   # 源文件目录
│   ├── server/            # 服务器实现
│   ├── network/           # 网络层实现
│   ├── utils/             # 工具类实现
│   └── logger/            # 日志系统实现
├── tests/                 # 测试文件
├── scripts/               # 脚本文件
│   ├── build.sh           # 构建脚本
│   ├── run.sh             # 运行脚本
│   └── benchmark.sh       # 压测脚本
└── docker/                # Docker 相关
    ├── Dockerfile
    └── docker-compose.yml
```

## 快速开始

### 前置要求

- C++17 编译器 (g++ 7+ 或 clang++)
- Make 或 CMake (3.15+)
- Linux 系统（支持 epoll）
- SQLite3 开发库（Milestone 3+）
  ```bash
  sudo apt-get install -y libsqlite3-dev
  ```

### 编译方式一：使用 Makefile（推荐，无需 CMake）

```bash
make
# 或
make -j$(nproc)
```

### 编译方式二：使用 CMake

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 运行

```bash
# 使用 Makefile
make run

# 或直接运行
./build/bin/ChatGPTServer [port]
# 默认端口 8080
```

### 测试

```bash
# 测试 API 端点（Milestone 1-2）
curl http://localhost:8080/              # 聊天 UI 或欢迎页面
curl http://localhost:8080/health        # 健康检查（JSON）
curl -X POST http://localhost:8080/echo \
  -H "Content-Type: application/json" \
  -d '{"test":"data"}'                   # Echo 回显

# 测试聊天 API（Milestone 3）
curl -X POST http://localhost:8080/chat \
  -H "Content-Type: application/json" \
  -d '{"message":"Hello!","session_id":""}' # 发送消息

curl http://localhost:8080/metrics       # 系统指标

# 运行单元测试
./build/bin/test_http_parser             # HTTP 解析器测试
./build/bin/test_router                  # 路由系统测试

# 运行集成测试
./scripts/quick_test.sh                  # Milestone 1-2 测试
./scripts/test_milestone3.sh             # Milestone 3 完整测试
```

## 里程碑进度

- [x] Milestone 0: 工程骨架 & 最小可跑
- [x] Milestone 1: 最小服务器 Demo (HTTP 解析 + 路由 + 测试)
- [x] Milestone 2: 高并发网络与多线程 (epoll + 线程池 + 过载保护)
- [x] Milestone 3: 工程化组件 (Logger 滚动 + Database + Router + UI)
- [ ] Milestone 4: LLM Serving
- [ ] Milestone 5: 性能工程
- [ ] Milestone 6: Docker 化部署

## 性能指标

### Milestone 2: 高并发
- **并发**: 100+ 并发连接
- **吞吐**: 1785 QPS
- **延迟**: P99 < 85ms
- **成功率**: 99.9%

### Milestone 3: 工程化
- **日志**: 分级 + 滚动（10MB/文件，保留 5 个）
- **数据库**: SQLite3（会话 + 消息持久化）
- **路由**: 6 个端点（/, /health, /chat, /history, /metrics, /echo）
- **UI**: 现代化聊天界面（响应式设计）

## 项目文档

### 核心文档
- [README.md](README.md) - 项目说明（本文档）
- [MILESTONES.md](notes/01_项目概览/里程碑计划.md) - 里程碑跟踪
- [PROJECT_OVERVIEW.md](notes/01_项目概览/项目概览.md) - 项目总览
- [PERFORMANCE.md](notes/06_性能测试/性能报告.md) - 性能测试报告

### 里程碑完成报告
- [MILESTONE1_SUMMARY.md](notes/02_阶段一_最小服务器/阶段一总结.md) - M1 完成总结
- [MILESTONE2_SUMMARY.md](notes/03_阶段二_高并发/阶段二总结.md) - M2 完成总结
- [MILESTONE3_SUMMARY.md](notes/04_阶段三_工程化/阶段三总结.md) - M3 完成总结（详细技术文档）
- [MILESTONE3_完成报告.md](notes/04_阶段三_工程化/阶段三完成报告.md) - M3 中文报告

### 快速开始
- [QUICKSTART.md](notes/01_项目概览/快速入门.md) - 快速入门
- [QUICKSTART_MILESTONE3.md](notes/04_阶段三_工程化/阶段三快速开始.md) - M3 快速开始

### 安装和构建
- [MILESTONE3_INSTALL.md](notes/04_阶段三_工程化/阶段三安装指南.md) - M3 安装说明
- [BUILD_SYSTEM_UPGRADE.md](notes/05_构建与运维/构建系统升级.md) - 构建系统升级
- [MAKEFILE_USAGE.md](notes/05_构建与运维/Makefile使用指南.md) - Makefile 使用指南

## 下一步

✅ **Milestone 3 已完成** - 工程化组件（Logger 滚动 + Database + UI）

**接下来**：
- Milestone 4: LLM Serving（推理 API + 上下文管理）
- Milestone 5: 性能工程（KV-cache + batching + 优化报告）
- Milestone 6: Docker 化部署 & 上线复现
