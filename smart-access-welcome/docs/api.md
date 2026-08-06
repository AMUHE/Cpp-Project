# HTTP 与 WebSocket API

本文定义 v0.1.0 的计划协议。实现前允许通过 ADR 调整，但不得无文档破坏兼容性。

## 通用约定

- API 前缀：`/api/v1`。
- 编码：UTF-8 JSON。
- 时间：带时区 ISO 8601。
- 标识符：UUID。
- 默认监听：`127.0.0.1:8080`。
- 错误包含 `code`、`message` 和 `correlationId`，不泄漏内部路径或堆栈。

## HTTP

### `GET /health/live`

只证明进程存活：

```json
{ "status": "ok" }
```

### `GET /health/ready`

```json
{
  "status": "degraded",
  "components": {
    "camera": "offline",
    "recognizer": "ready",
    "database": "ready"
  }
}
```

### `GET /api/v1/terminal/status`

```json
{
  "deviceId": "terminal-demo-01",
  "state": "idle",
  "message": "请面向摄像头",
  "updatedAt": "2026-08-06T16:30:00+08:00"
}
```

人员录入、删除、配置修改和审计查询属于受保护接口。首版实现认证前不对网络开放，也不允许欢迎页调用。

## WebSocket

- 路径：`/ws`。
- 建立连接后先发送 `terminal.snapshot`。
- 客户端采用有上限的指数退避重连，重连后重新读取状态。
- 消息包含 `schemaVersion`、`type`、`eventId` 和 `occurredAt`。

放行事件：

```json
{
  "schemaVersion": 1,
  "type": "access.granted",
  "eventId": "9dcfb064-a398-4ee0-845a-57c87bc6a103",
  "occurredAt": "2026-08-06T16:30:00+08:00",
  "payload": {
    "deviceId": "terminal-demo-01",
    "person": {
      "id": "d67d475e-d0c7-449c-a684-73cfef1728b9",
      "displayName": "张三"
    },
    "welcomeText": "欢迎，张三",
    "displayDurationSeconds": 5
  }
}
```

拒绝事件默认不包含截图或相似人员信息：

```json
{
  "schemaVersion": 1,
  "type": "access.denied",
  "eventId": "21d6db44-cae7-4efa-b35a-ad51ddc4ca78",
  "occurredAt": "2026-08-06T16:31:00+08:00",
  "payload": {
    "deviceId": "terminal-demo-01",
    "reason": "unknown_person",
    "message": "未识别，请联系管理员"
  }
}
```

客户端必须忽略未知字段。删除字段、改变含义或类型时提升 `schemaVersion`。服务端限制请求体、消息长度、连接数和发送频率；远程开放时必须增加认证、来源校验、TLS 和限流。

