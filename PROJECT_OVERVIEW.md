# ChatGPT Server - 项目总览

## 项目介绍

一个可部署、可压测、可优化的高性能 C++ LLM 推理服务器。

**核心特性**:
- epoll + 线程池高并发架构
- HTTP REST API
- 多轮对话支持
- 过载保护
- Docker 一键部署
- 性能数据验证

## 项目架构

```
┌─────────────────────────────────────────────────┐
│                   Client                        │
│           (Web UI / API Client)                 │
└────────────────────┬────────────────────────────┘
                     │ HTTP/REST
┌────────────────────▼────────────────────────────┐
│               Nginx (Optional)                  │
│         (Reverse Proxy + HTTPS)                 │
└────────────────────┬────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────┐
│          ChatGPT Server (C++)                   │
│  ┌──────────────────────────────────────┐      │
│  │      EpollReactor (事件循环)         │      │
│  │  ┌────────────────────────────┐      │      │
│  │  │  Listen Socket             │      │      │
│  │  │  Client Sockets (epoll)    │      │      │
│  │  └────────────────────────────┘      │      │
│  └──────────┬───────────────────────────┘      │
│             │                                    │
│  ┌──────────▼───────────────────────────┐      │
│  │       ThreadPool (并行处理)          │      │
│  │  ┌────┬────┬────┬────┐              │      │
│  │  │ W1 │ W2 │ W3 │ W4 │              │      │
│  │  └────┴────┴────┴────┘              │      │
│  └──────────┬───────────────────────────┘      │
│             │                                    │
│  ┌──────────▼───────────────────────────┐      │
│  │       HTTP Parser + Router           │      │
│  │  ┌────────────────────────────┐      │      │
│  │  │ GET  /                     │      │      │
│  │  │ GET  /health               │      │      │
│  │  │ POST /chat                 │      │      │
│  │  │ GET  /history              │      │      │
│  │  │ GET  /metrics              │      │      │
│  │  │ POST /echo                 │      │      │
│  │  └────────────────────────────┘      │      │
│  └──────────┬───────────────────────────┘      │
│             │                                    │
│  ┌──────────▼───────────────────────────┐      │
│  │   ConnectionManager (过载保护)       │      │
│  │  - 连接限制: 10000                   │      │
│  │  - 超时检测: 60s                     │      │
│  │  - 请求限制: 10000/conn              │      │
│  └──────────────────────────────────────┘      │
└─────────────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────┐
│          LLM Inference Engine                   │
│    (本地模型 / API 接入)                        │
└─────────────────────────────────────────────────┘
```

## 技术栈

### 核心技术
- **语言**: C++17
- **网络**: epoll (Linux)
- **并发**: std::thread + 线程池
- **HTTP**: 自研解析器
- **构建**: CMake + Make

### 开发工具
- **编译器**: g++ / clang++
- **调试**: gdb
- **压测**: Python + ab/wrk
- **容器**: Docker + docker-compose

## 项目结构

```
cppServer/
├── include/                    # 头文件
│   ├── server/
│   │   ├── server.h           # 服务器主类
│   │   ├── router.h           # 路由系统
│   │   ├── epoll_reactor.h    # epoll 事件循环
│   │   └── connection_manager.h # 连接管理
│   ├── database/              # 数据库（M3+）
│   │   └── database.h         # SQLite 封装
│   ├── http_parser.h          # HTTP 解析器
│   ├── logger/
│   │   └── logger.h           # 日志系统（M3 增强）
│   └── utils/
│       ├── signal_handler.h   # 信号处理
│       ├── thread_pool.h      # 线程池
│       └── json_helper.h      # JSON 工具（M3+）
│
├── src/                        # 源文件
│   ├── main.cpp               # 主程序
│   ├── server/                # 服务器实现
│   ├── http_parser.cpp        # HTTP 实现
│   ├── logger/                # 日志实现
│   └── utils/                 # 工具实现
│
├── tests/                      # 测试
│   ├── test_http_parser.cpp   # HTTP 测试
│   └── test_router.cpp        # 路由测试
│
├── scripts/                    # 脚本
│   ├── build.sh               # 编译
│   ├── run.sh                 # 运行
│   ├── quick_test.sh          # 快速测试
│   ├── demo.sh                # 演示
│   ├── benchmark.sh           # Shell 压测
│   └── simple_benchmark.py    # Python 压测
│
├── static/                    # 静态文件（M3+）
│   └── index.html             # 聊天 UI
│
├── data/                      # 数据目录（M3+，运行时生成）
│   └── chat.db                # SQLite 数据库
│
├── CMakeLists.txt             # CMake 配置
├── Makefile                   # Make 配置
├── README.md                  # 项目说明
├── MILESTONES.md              # 里程碑跟踪
├── MILESTONE1_SUMMARY.md      # M1 总结
├── MILESTONE2_SUMMARY.md      # M2 总结
├── MILESTONE3_SUMMARY.md      # M3 总结
├── PERFORMANCE.md             # 性能报告
└── PROJECT_OVERVIEW.md        # 本文档
```

## 已完成的里程碑

### Milestone 0: 工程骨架 ✅
- CMake 工程结构
- 日志系统
- 信号处理
- Hello World 服务器

### Milestone 1: 最小服务器 Demo ✅
- HTTP 解析器
- 路由系统
- 示例路由（/, /health, /echo）
- 单元测试（34 个）
- 集成测试

**关键指标**:
- 单连接稳定工作 ✅
- HTTP 请求/响应正确 ✅
- 测试通过率 100% ✅

### Milestone 2: 高并发网络与多线程 ✅
- epoll 事件驱动
- 线程池（任务队列 + worker）
- 连接管理（限流 + 超时 + 拒绝）
- 过载保护
- 压测验证

**关键指标**:
- 并发: 100+ ✅
- QPS: 1785 ✅
- P99 延迟: 83ms ✅
- 成功率: 99.9% ✅

### Milestone 3: 工程化组件 ✅
- Logger 增强（日志滚动、分级、格式统一）
- Database 集成（SQLite3 会话/对话记录存储）
- Router 增强（/chat、/history、/metrics 端点）
- Web UI（现代化聊天界面）
- 静态文件服务
- JSON 工具类

**关键指标**:
- 日志滚动: 10MB/文件，保留 5 个 ✅
- 数据库: SQLite3（2 表 + 索引）✅
- 路由: 6 个端点 ✅
- UI: 响应式聊天界面 ✅

## 待完成的里程碑

### Milestone 4: Docker 化部署
- [ ] Dockerfile
- [ ] docker-compose（nginx + backend + db）
- [ ] Nginx 反向代理配置
- [ ] 一键部署脚本

### Milestone 5: LLM Serving
- [ ] 推理 API（/chat/completions）
- [ ] 本地模型部署 / API 接入
- [ ] 上下文管理（多轮对话）
- [ ] 流式响应（SSE）
- [ ] 性能指标（token/s, 延迟）

### Milestone 6: 性能工程
- [ ] KV-cache 生命周期管理
- [ ] Batching 推理
- [ ] 显存/内存优化
- [ ] 性能曲线（延迟、吞吐、显存）
- [ ] 优化报告（问题 → 方案 → 结果）

## 快速开始

### 编译

```bash
# 方式 1: 使用脚本
./scripts/build.sh

# 方式 2: 手动 CMake
mkdir build && cd build
cmake ..
make -j$(nproc)

# 方式 3: 使用 Makefile
make
```

### 运行

```bash
# 默认配置（端口 8080，自动检测 CPU 核心数）
./build/bin/ChatGPTServer

# 指定端口
./build/bin/ChatGPTServer 9000

# 指定端口和线程数
./build/bin/ChatGPTServer 9000 8
```

### 测试

```bash
# 单元测试
./build/bin/test_http_parser
./build/bin/test_router

# 集成测试
./scripts/quick_test.sh

# 压测
python3 scripts/simple_benchmark.py

# 完整演示
./scripts/demo.sh
```

### API 测试

```bash
# 健康检查
curl http://localhost:8080/health
# => {"status":"ok","service":"ChatGPT Server"}

# 根路径（Milestone 3: 聊天 UI）
curl http://localhost:8080/
# => <html>... 聊天界面 ...</html>

# 聊天 API（Milestone 3）
curl -X POST http://localhost:8080/chat \
  -H "Content-Type: application/json" \
  -d '{"message":"Hello","session_id":""}'
# => {"session_id":"...", "message":"Hello", "reply":"..."}

# 系统指标（Milestone 3）
curl http://localhost:8080/metrics
# => {"service":"ChatGPT Server","total_sessions":5,"total_messages":20}

# Echo
curl -X POST http://localhost:8080/echo \
  -H "Content-Type: application/json" \
  -d '{"message":"Hello"}'
# => {"method":"POST","path":"/echo","body":"{\"message\":\"Hello\"}"}
```

## 性能指标

### 当前性能 (Milestone 2)

| 指标 | 值 |
|------|-----|
| 并发连接 | 100+ |
| QPS | 1785 |
| P50 延迟 | 4.68 ms |
| P99 延迟 | 83.05 ms |
| 成功率 | 99.9% |

### 配置限制

| 参数 | 默认值 |
|------|--------|
| 最大连接数 | 10000 |
| 连接超时 | 60 秒 |
| 每连接最大请求数 | 10000 |
| 线程池大小 | CPU 核心数 |

## 开发指南

### 添加新路由

```cpp
// src/main.cpp
server.getRouter().addRoute("GET", "/api/new", 
    [](const HttpRequest& req) {
        HttpResponse resp;
        resp.statusCode = 200;
        resp.body = "{\"data\":\"hello\"}";
        resp.headers["Content-Type"] = "application/json";
        return resp;
    });
```

### 修改配置

```cpp
// src/server/connection_manager.cpp
ConnectionManager(
    10000,  // maxConnections
    60,     // timeoutSeconds
    10000   // maxRequestsPerConnection
);
```

### 调试

```bash
# 编译 Debug 版本
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# 使用 gdb
gdb ./build/bin/ChatGPTServer
```

## 贡献指南

### 分支策略
- `main`: 稳定版本
- `develop`: 开发版本
- `feature/*`: 功能分支

### 提交规范
```
feat: 新功能
fix: 修复 bug
docs: 文档更新
perf: 性能优化
test: 测试相关
refactor: 重构
```

## 许可证

MIT License

## 联系方式

- **项目地址**: /home/ava/cppServer
- **文档**: README.md, MILESTONES.md
- **问题反馈**: [项目仓库 Issues]

---

**最后更新**: 2026-01-28  
**当前版本**: v0.3.0-milestone3  
**下一个里程碑**: Milestone 4 - Docker 化部署
