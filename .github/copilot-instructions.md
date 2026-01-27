<!-- copilot-instructions: concise, actionable guidance for AI coding agents -->
# 项目快速上手（给 AI agent 的关键信息）

目的：快速让 AI 代理理解工程架构、常见开发流程、代码约定与关键集成点，便于进行修复、特性实现与重构。

- **主要组件**:
  - `server`：最核心的网络入口，当前实现为同步 accept 循环，计划替换为 epoll + 线程池（查看 [src/server/server.cpp](src/server/server.cpp) 和 [include/server/server.h](include/server/server.h)）。
  - `logger`：单例日志系统，提供 `LOG_INFO/LOG_ERROR/...` 宏（查看 [include/logger/logger.h](include/logger/logger.h) 和 [src/logger/logger.cpp](src/logger/logger.cpp)）。
  - `utils`：信号处理等工具（例如 [include/utils/signal_handler.h](include/utils/signal_handler.h)）。
  - 入口：`src/main.cpp`（进程初始化、日志设置、根据第一个 arg 指定端口并启动 `Server`）。

- **代码流与设计意图**:
  - 入口流程：`main` -> 初始化 `Logger` -> 创建 `Server(port)` -> `start()` -> `run()`（当前为简单 accept + 同步响应，后续目标是 HTTP 解析、epoll + 线程池、并发限流）。
  - 日志：程序会创建 `logs/` 目录并写入 `logs/server.log`（在 `main` 中设置）。
  - 构建系统：使用 CMake（主文件 [CMakeLists.txt](CMakeLists.txt)），`include/` 已被添加为 include path，源码通过 `file(GLOB_RECURSE SOURCES "src/*.cpp")` 收集。

- **构建 / 运行 / 测试（常用命令）**:
  - 一键构建（项目根目录）：
    - `./scripts/build.sh` 或手动：
      - `mkdir build && cd build`，`cmake ..`，`make -j$(nproc)`
  - 运行：
    - `./scripts/run.sh [port]` 或 `./build/bin/ChatGPTServer [port]`（默认端口 8080）。
  - 日志文件：`logs/server.log`（注意创建 `logs/` 目录）。
  - 测试：项目包含 `tests/` 的 CMake 测试，使用 `ctest` 或 `make test`（在 build 目录）。

- **项目约定与风格（在改动代码时请遵循）**:
  - 使用 `Logger` 单例和 `LOG_*` 宏来写日志，避免 printf/直接 cout。例：`LOG_INFO("msg")`。
  - 新增头文件放到 `include/<module>/`，源文件放到 `src/<module>/`，并在 CMake 中保持目录结构一致。
  - 所有网络/IO 代码应优先考虑可优雅关闭、EAGAIN/EINTR 处理与超时策略（现有 accept 循环中已有基本处理）。
  - 编译选项在 `CMakeLists.txt` 中集中管理（使用 `CMAKE_CXX_STANDARD 17`，默认 `-O2 -g`）。

- **集成点 / 外部依赖**:
  - pthread 与动态链接库 (`${CMAKE_DL_LIBS}`) 在 CMake 中被链接。
  - Docker 支持位于 `docker/`（若存在 Dockerfile 与 docker-compose），可用于一键部署。
  - LLM 服务通常以独立进程或容器对接（该仓库目标在后续里程碑中接入本地或远端推理服务）。

- **常见修改示例（快速示范）**:
  - 增加新 HTTP 路由：修改 `Server::run()`（目前在 [src/server/server.cpp](src/server/server.cpp)）并添加路由分发模块到 `src/server/`。
  - 将 accept 循环替换为 epoll：在 `server` 模块新增 `EpollReactor` 类，保持 `Server` 的外部 API（`start() / run() / stop()`）不变以降低变更面。
  - 日志改进：调用 `Logger::getInstance().setLogFile(...)` 在 `main` 中配置日志路径，保持宏兼容。

- **定位和调试建议（来自代码现状）**:
  - 若服务器无法绑定端口：查看 `LOG_ERROR` 输出与 `bind()` 返回的 errno（[src/server/server.cpp](src/server/server.cpp)）。
  - 若连接被立即关闭或响应异常：检查 `send()` 返回值与 socket flags；短期可在 accept 后先把 clientFd 设为非阻塞再测试。
  - 想验证构建产物路径：`build/bin/ChatGPTServer`（构建后仓库会在 `build/bin` 产生可执行文件）。

- **重要文件索引（快速跳转）**:
  - [src/main.cpp](src/main.cpp)
  - [src/server/server.cpp](src/server/server.cpp)
  - [include/server/server.h](include/server/server.h)
  - [include/logger/logger.h](include/logger/logger.h)
  - [CMakeLists.txt](CMakeLists.txt)
  - [scripts/build.sh](scripts/build.sh)
  - [scripts/run.sh](scripts/run.sh)
  - [README.md](README.md)
  - [MILESTONES.md](MILESTONES.md)

如果需要我可以：
- 将 `Server` 的 accept 循环改为 epoll 的雏形并保留现有 API；或
- 添加一个最小的 HTTP 解析/路由器并为 `/health`、`/chat` 提供示例处理器。

请指出你希望我优先做哪一项或补充我遗漏的项目约定。 
