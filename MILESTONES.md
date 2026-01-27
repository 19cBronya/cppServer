# 项目里程碑文档

## Milestone 0: 工程骨架 & 最小可跑 ✅

**目标**: 把仓库跑起来，形成工程习惯

### 交付物/验收

- [x] CMake 工程、目录结构（src/include/tests/scripts）
- [x] 能启动一个进程，输出日志，优雅退出（SIGINT）
- [x] 最小 hello world 服务可本地访问（curl 能通）

### 完成情况

- ✅ 创建了完整的项目目录结构
- ✅ 实现了日志系统（Logger）
- ✅ 实现了信号处理器（SignalHandler）
- ✅ 实现了基础服务器框架（Server）
- ✅ 支持优雅退出（Ctrl+C）
- ✅ 可以响应 HTTP 请求并返回 "Hello, World!"
- ✅ 提供了 Makefile 和 CMakeLists.txt 两种构建方式

### 测试方法

```bash
# 编译
make

# 运行服务器
./build/bin/ChatGPTServer 8080

# 在另一个终端测试
curl http://localhost:8080/
```

---

## Milestone 1: 最小服务器 Demo ✅ (2026-01-28)

**目标**: 先把"能收请求→能回响应"打通

### 交付物/验收

- [x] 支持 HTTP 基本解析（方法、路径、头部、Body）
- [x] 单连接/少量连接下稳定工作
- [x] 单元测试/简单集成测试（至少有一套可复现用例）

### 完成情况

- ✅ HTTP 解析器（parseRequest, createResponse, toString）
- ✅ 路由系统（路由注册、匹配、404/400/500 处理）
- ✅ Server 集成（HTTP 解析 + 路由分发）
- ✅ 示例路由（/, /health, /echo）
- ✅ 单元测试（34 个测试，全部通过）
  - test_http_parser: 25/25 ✅
  - test_router: 9/9 ✅
- ✅ 集成测试（4 个端点测试，全部通过）

### 测试方法

```bash
# 编译
./scripts/build.sh

# 运行服务器
./scripts/run.sh

# 运行单元测试
./build/bin/test_http_parser
./build/bin/test_router

# 运行集成测试
./scripts/quick_test.sh

# 手动测试 API
curl http://localhost:8080/health
curl http://localhost:8080/
curl -X POST http://localhost:8080/echo -d '{"test":"data"}'
```

**详细报告**: 见 [MILESTONE1_SUMMARY.md](MILESTONE1_SUMMARY.md)

---

## Milestone 2: 高并发网络与多线程

**目标**: 让服务器"接得住、稳得住"

### 交付物/验收

- [ ] epoll reactor 或类似事件驱动模型
- [ ] 线程池：任务队列 + worker + 优雅关闭
- [ ] 过载保护：超时/限流/拒绝策略（防雪崩）
- [ ] 压测：本地压到一定并发，P99 不崩、错误率可控

---

## Milestone 3: 工程化组件拼齐

**目标**: 从"能跑"进化到"像企业项目"

### 交付物/验收

- [ ] Logger：分级、滚动、格式统一（部分完成）
- [ ] Router：路由表（/health、/chat、/metrics）
- [ ] Database：至少存储会话/对话记录（支持查询/回放）
- [ ] UI：最简单聊天页能用（哪怕很朴素）

---

## Milestone 4: Docker 化部署 & 上线复现

**目标**: 把"可复现部署"做出来

### 交付物/验收

- [ ] Dockerfile + docker-compose（nginx + backend + db + llm 服务）
- [ ] 新机器/新环境：只按 README 操作即可跑起来
- [ ] Nginx 反向代理打通（/api 转发到 C++ 服务）

---

## Milestone 5: LLM Serving

**目标**: 让你的系统真的能"像 ChatGPT 一样回答"

### 交付物/验收

- [ ] 推理 API：完成一次完整对话链路（UI→网关→推理→返回）
- [ ] 上下文管理：多轮对话可用（至少基于历史拼 prompt）
- [ ] 指标/压测：能测到 token/s、延迟、并发下错误率

---

## Milestone 6: 性能工程

**目标**: 你课程的"核心亮点"，也是简历最值钱的部分

### 交付物/验收

- [ ] KV-cache 生命周期管理：append / truncate / 回收 + OOM 保护
- [ ] 量化曲线：
  - 上下文长度增长下：延迟曲线、显存占用曲线
  - batching：合并请求提高吞吐（并给出前后对比数据）
- [ ] 写一份《性能优化报告》：问题→定位→方案→结果→代价/边界

