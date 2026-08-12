# HTTP 与 WebSocket API

当前协议版本为 v1，默认仅监听 `127.0.0.1:8080`。所有响应和消息均为 UTF-8；时间使用带时区 ISO 8601；事件标识为 UUID。欢迎页是只读客户端，不具有人员管理、配置或审计查询权限。

## HTTP

### `GET /`

返回内置响应式欢迎页。页面不依赖 CDN，通过同源 WebSocket 接收状态；断线后执行有上限的指数退避重连。

### `GET /health/live`

```json
{"status":"ok"}
```

只证明进程和网络事件循环可响应。

### `GET /health/ready`

```json
{
  "status": "degraded",
  "components": {
    "camera": "offline",
    "recognizer": "ready",
    "database": "ready",
    "speech": "ready",
    "server": "ready"
  }
}
```

组件可报告 `ready`、`offline`、`unavailable`、`model_missing` 或 `disabled`。HTTP 状态仍为 200，监控系统应读取 JSON 的 `status`；进程无法提供响应时由连接失败表达不可用。

### `GET /api/v1/terminal/status`

```json
{
  "deviceId":"terminal-demo-01",
  "deviceName":"一号门智能终端",
  "state":"detecting",
  "message":"请面向摄像头",
  "updatedAt":"2026-08-12T16:30:00+08:00"
}
```

未知路由返回 404，非 GET 方法返回 405，请求头超过 16 KiB 返回 413。连接总数上限为 32。响应包含 `nosniff`、CSP、`no-referrer` 和 `no-store` 等基础安全头。

## WebSocket `/ws`

连接成功后首先收到 `terminal.snapshot`：

```json
{
  "schemaVersion":1,
  "type":"terminal.snapshot",
  "eventId":"f2f75003-e9cd-4fd6-a9d3-f338722e26dd",
  "occurredAt":"2026-08-12T16:30:00+08:00",
  "payload":{
    "deviceId":"terminal-demo-01",
    "deviceName":"一号门智能终端",
    "state":"idle",
    "message":"请面向摄像头",
    "updatedAt":"2026-08-12T16:29:59+08:00"
  }
}
```

放行事件：

```json
{
  "schemaVersion":1,
  "type":"access.granted",
  "eventId":"9dcfb064-a398-4ee0-845a-57c87bc6a103",
  "occurredAt":"2026-08-12T16:30:00+08:00",
  "payload":{
    "deviceId":"terminal-demo-01",
    "person":{"id":"d67d475e-d0c7-449c-a684-73cfef1728b9","displayName":"张三"},
    "welcomeText":"欢迎，张三",
    "doorAction":"simulated_unlocked",
    "displayDurationSeconds":5
  }
}
```

拒绝事件：

```json
{
  "schemaVersion":1,
  "type":"access.denied",
  "eventId":"21d6db44-cae7-4efa-b35a-ad51ddc4ca78",
  "occurredAt":"2026-08-12T16:31:00+08:00",
  "payload":{"deviceId":"terminal-demo-01","reason":"unknown_person","message":"未识别，请联系管理员"}
}
```

客户端必须忽略未知字段。删除字段、改变字段类型或语义时提升 `schemaVersion`。当前协议不发送截图、候选身份、LBPH 距离或样本路径。

## 远程访问安全

本服务没有管理认证，不得直接暴露到不可信网络。远程部署必须在反向代理或网关增加 TLS、双向设备认证、来源校验、速率限制和审计；人员录入、删除、配置修改和审计查询应由独立受保护管理面实现。
