# 🔄 重启服务器步骤

## 问题说明
之前的 `run.sh` 从 `build/bin/` 目录启动服务器，导致找不到 `static/index.html`。

现在已修复，从项目根目录启动。

## 重启步骤

### 1. 停止当前服务器
```bash
# 按 Ctrl+C 停止服务器
# 或者在新终端中运行：
pkill ChatGPTServer
```

### 2. 重新启动
```bash
cd /home/ava/cppServer
./scripts/run.sh
```

### 3. 测试 UI
浏览器访问：`http://localhost:8080`

现在应该能看到漂亮的聊天界面了！🎉

## 预期效果

访问 `http://localhost:8080/` 应该看到：
- 紫色渐变背景
- 白色聊天窗口
- "💬 ChatGPT Server" 标题
- 消息输入框和发送按钮

## 如果还是不行

手动从项目根目录启动：
```bash
cd /home/ava/cppServer
./build/bin/ChatGPTServer 8080
```

确保看到日志：
```
[INFO] Handling request: GET /
```

然后刷新浏览器。
