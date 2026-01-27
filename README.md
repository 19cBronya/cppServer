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
# 测试 API 端点
curl http://localhost:8080/              # 欢迎页面
curl http://localhost:8080/health        # 健康检查（JSON）
curl -X POST http://localhost:8080/echo \
  -H "Content-Type: application/json" \
  -d '{"test":"data"}'                   # Echo 回显

# 运行单元测试
./build/bin/test_http_parser             # HTTP 解析器测试
./build/bin/test_router                  # 路由系统测试

# 运行集成测试
./scripts/quick_test.sh                  # 自动化端到端测试
```

## 里程碑进度

- [x] Milestone 0: 工程骨架 & 最小可跑
- [x] Milestone 1: 最小服务器 Demo (HTTP 解析 + 路由 + 测试)
- [ ] Milestone 2: 高并发网络与多线程
- [ ] Milestone 3: 工程化组件
- [ ] Milestone 4: Docker 化部署
- [ ] Milestone 5: LLM Serving
- [ ] Milestone 6: 性能工程

## 开发计划

详见各里程碑文档。

# cppServer
