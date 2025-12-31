# 快速启动指南

## Milestone 0 完成 ✅

项目基础框架已搭建完成，可以立即运行测试。

## 快速测试

### 1. 编译项目

```bash
make
```

### 2. 启动服务器

```bash
# 方式一：使用脚本
./scripts/run.sh 8080

# 方式二：直接运行
./build/bin/ChatGPTServer 8080
```

### 3. 测试服务器

在另一个终端运行：

```bash
curl http://localhost:8080/
```

应该返回：
```
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 13

Hello, World!
```

### 4. 停止服务器

按 `Ctrl+C` 优雅退出，服务器会输出：
```
[时间] [INFO ] Received SIGINT, shutting down gracefully...
[时间] [INFO ] Server stopping...
[时间] [INFO ] Server stopped
```

## 项目结构说明

```
cppServer/
├── include/          # 头文件
│   ├── logger/      # 日志系统
│   ├── server/      # 服务器核心
│   └── utils/       # 工具类
├── src/             # 源文件（对应 include 结构）
├── build/           # 编译输出（自动生成）
├── logs/            # 日志文件（自动生成）
├── scripts/         # 脚本文件
└── tests/           # 测试文件
```

## 下一步：Milestone 1

准备实现：
- HTTP 请求解析器
- HTTP 响应构建器  
- 路由系统
- 单元测试

详见 `MILESTONES.md`

