# Milestone 3 快速开始指南

## 🚀 3 步启动

### 步骤 1: 安装 SQLite3 开发库

```bash
sudo apt-get update
sudo apt-get install -y libsqlite3-dev
```

### 步骤 2: 编译项目

```bash
./scripts/build.sh
```

### 步骤 3: 启动服务器

```bash
./scripts/run.sh
```

## 🌐 访问聊天界面

浏览器打开: `http://localhost:8080`

## ✅ 运行测试

```bash
# 在另一个终端启动服务器
./scripts/run.sh

# 运行集成测试
./scripts/test_milestone3.sh
```

## 📋 可用的 API 端点

| 端点 | 方法 | 功能 |
|------|------|------|
| `/` | GET | 聊天 UI 界面 |
| `/health` | GET | 健康检查 |
| `/chat` | POST | 发送消息 |
| `/history` | GET | 查询历史 |
| `/metrics` | GET | 系统指标 |

## 💡 示例：使用 curl 测试

### 发送消息
```bash
curl -X POST http://localhost:8080/chat \
  -H "Content-Type: application/json" \
  -d '{"message":"Hello!","session_id":""}'
```

### 查询历史（替换 SESSION_ID）
```bash
curl "http://localhost:8080/history?session_id=SESSION_ID"
```

### 查看指标
```bash
curl http://localhost:8080/metrics
```

## 📁 数据文件位置

- **日志**: `logs/server.log` (自动滚动)
- **数据库**: `data/chat.db` (SQLite)

## 🔍 查看数据库内容

```bash
sqlite3 data/chat.db

# 在 SQLite shell 中:
.tables                    # 查看所有表
SELECT * FROM sessions;    # 查看所有会话
SELECT * FROM messages;    # 查看所有消息
.quit                      # 退出
```

## 🐛 常见问题

### 编译错误: sqlite3.h not found

**解决**: 安装 SQLite3 开发库
```bash
sudo apt-get install -y libsqlite3-dev
```

### 端口被占用

**解决**: 使用其他端口
```bash
./scripts/run.sh 9000
```

### 无法访问 UI

**解决**: 确认 `static/index.html` 文件存在
```bash
ls -l static/index.html
```

## 📚 更多信息

- 完整文档: [MILESTONE3_SUMMARY.md](MILESTONE3_SUMMARY.md)
- 安装说明: [MILESTONE3_INSTALL.md](MILESTONE3_INSTALL.md)
- 项目概述: [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md)
