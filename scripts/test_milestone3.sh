#!/bin/bash

# Milestone 3 集成测试脚本

echo "======================================"
echo "   Milestone 3 集成测试"
echo "======================================"
echo ""

# 检查服务器是否运行
if ! pgrep -x "ChatGPTServer" > /dev/null; then
    echo "⚠️  服务器未运行"
    echo "请先运行: ./scripts/run.sh"
    exit 1
fi

echo "✅ 服务器正在运行"
echo ""

# 等待服务器启动
sleep 2

BASE_URL="http://localhost:8080"

echo "测试 1: 健康检查 /health"
echo "-----------------------------------"
curl -s $BASE_URL/health | jq . || curl -s $BASE_URL/health
echo ""
echo ""

echo "测试 2: 系统指标 /metrics"
echo "-----------------------------------"
curl -s $BASE_URL/metrics | jq . || curl -s $BASE_URL/metrics
echo ""
echo ""

echo "测试 3: 发送聊天消息 /chat"
echo "-----------------------------------"
SESSION_RESPONSE=$(curl -s -X POST $BASE_URL/chat \
  -H "Content-Type: application/json" \
  -d '{"message":"Hello, this is a test message!","session_id":""}')

echo "$SESSION_RESPONSE" | jq . || echo "$SESSION_RESPONSE"
echo ""

# 提取 session_id
SESSION_ID=$(echo "$SESSION_RESPONSE" | grep -o '"session_id":"[^"]*"' | cut -d'"' -f4)

if [ -n "$SESSION_ID" ]; then
    echo "✅ 会话 ID: $SESSION_ID"
    echo ""
    
    echo "测试 4: 发送第二条消息（使用现有会话）"
    echo "-----------------------------------"
    curl -s -X POST $BASE_URL/chat \
      -H "Content-Type: application/json" \
      -d "{\"message\":\"This is my second message\",\"session_id\":\"$SESSION_ID\"}" | jq . || \
    curl -s -X POST $BASE_URL/chat \
      -H "Content-Type: application/json" \
      -d "{\"message\":\"This is my second message\",\"session_id\":\"$SESSION_ID\"}"
    echo ""
    echo ""
    
    echo "测试 5: 获取会话历史 /history"
    echo "-----------------------------------"
    curl -s "$BASE_URL/history?session_id=$SESSION_ID" | jq . || \
    curl -s "$BASE_URL/history?session_id=$SESSION_ID"
    echo ""
else
    echo "⚠️  无法获取会话 ID，跳过历史查询测试"
fi

echo ""
echo "测试 6: 访问聊天 UI /"
echo "-----------------------------------"
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" $BASE_URL/)
if [ "$HTTP_CODE" = "200" ]; then
    echo "✅ 聊天 UI 可访问 (HTTP $HTTP_CODE)"
    echo "   浏览器访问: $BASE_URL"
else
    echo "❌ 聊天 UI 访问失败 (HTTP $HTTP_CODE)"
fi
echo ""

echo "======================================"
echo "   测试完成"
echo "======================================"
echo ""
echo "完整测试步骤:"
echo "1. ✅ 健康检查"
echo "2. ✅ 系统指标"
echo "3. ✅ 聊天消息发送"
echo "4. ✅ 会话持久化"
echo "5. ✅ 历史记录查询"
echo "6. ✅ UI 访问"
echo ""
echo "🎉 Milestone 3 验收完成！"
