# 🔨 Makefile 使用指南

## ✅ 已验证可用

Makefile 已经全面升级，支持 Milestone 3 所有功能！

---

## 🚀 快速开始（3 步）

### 1️⃣ 安装依赖（首次运行）
```bash
make install-deps
```
或手动安装：
```bash
sudo apt-get update
sudo apt-get install -y libsqlite3-dev
```

### 2️⃣ 编译并运行
```bash
make run
```

### 3️⃣ 访问聊天界面
浏览器打开：**`http://localhost:8080`**

---

## 📋 常用命令

### 编译
```bash
make              # 仅编译
make all          # 同上（明确写法）
```

### 运行
```bash
make run          # 编译并运行（8080端口）
```
**重要**：现在从项目根目录运行，能正确找到 `static/index.html`！

### 清理
```bash
make clean        # 清理编译产物
make clean-all    # 清理编译产物 + 日志 + 数据库
```

### 测试
```bash
make test         # 测试 health + metrics
make test-chat    # 测试聊天接口
```

### 帮助
```bash
make help         # 显示所有命令
make info         # 显示项目配置信息
```

---

## 🆚 Makefile vs CMake

| 操作 | Makefile | CMake |
|------|----------|-------|
| 编译 | `make` | `./scripts/build.sh` |
| 运行 | `make run` | `./scripts/run.sh` |
| 清理 | `make clean` | `rm -rf build` |
| 测试 | `make test` | `./scripts/test_milestone3.sh` |

**推荐**：日常开发用 `make`（快），正式测试用 CMake（可靠）。

---

## 🎯 工作流示例

### 开发流程
```bash
# 1. 修改代码
vim src/main.cpp

# 2. 编译并运行
make run

# 3. 浏览器测试
# http://localhost:8080

# 4. 停止服务器
# Ctrl+C
```

### 测试流程
```bash
# 终端 1：启动服务器
make run

# 终端 2：运行测试
make test
make test-chat
```

### 清理重编译
```bash
make clean all run
```

---

## 🔍 验证自动识别的源文件

```bash
make info
```

应该看到：
```
Source files:
  - src/logger/logger.cpp
  - src/utils/thread_pool.cpp
  - src/utils/json_helper.cpp          ← Milestone 3 新增
  - src/utils/signal_handler.cpp
  - src/main.cpp
  - src/database/database.cpp          ← Milestone 3 新增
  - src/http_parser.cpp
  - src/server/epoll_reactor.cpp
  - src/server/server.cpp
  - src/server/connection_manager.cpp
  - src/server/router.cpp

Linker flags:    -pthread -lsqlite3 -lstdc++fs
                                   ^^^^^^^^^^ Milestone 3 新增
```

---

## 🐛 常见问题

### Q1: `sqlite3.h: No such file or directory`
**解决**：
```bash
make install-deps
```

### Q2: UI 不显示（返回纯文本）
**原因**：旧版本 Makefile 工作目录错误

**解决**：更新后的 Makefile 已修复！重新运行：
```bash
make clean all run
```

### Q3: `stdc++fs` 链接错误
**解决**：确保使用 GCC 8+ 或 Clang 9+
```bash
g++ --version
```

---

## 📊 性能对比

### 编译速度
```bash
# 首次编译
make clean all
# 约 5-10 秒

# 增量编译（修改单个文件）
make
# 约 1-2 秒
```

### 对比 CMake
- **Makefile**：增量编译更快
- **CMake**：依赖检测更准确

---

## ✅ 验证清单

运行这些命令确认一切正常：

```bash
# 1. 查看帮助
make help          ✅

# 2. 查看项目信息
make info          ✅

# 3. 编译
make               ✅

# 4. 运行
make run           ✅

# 5. 测试（新终端）
make test          ✅
make test-chat     ✅
```

---

## 🎉 总结

**Makefile 已全面升级！** 现在支持：

- ✅ 自动查找所有源文件（包括 database、json_helper）
- ✅ 链接 sqlite3 和 stdc++fs
- ✅ 从项目根目录运行（能找到 static/）
- ✅ 丰富的命令（test、info、help 等）
- ✅ 清晰的输出和错误提示

**现在可以直接使用 `make run` 启动服务器了！** 🚀
