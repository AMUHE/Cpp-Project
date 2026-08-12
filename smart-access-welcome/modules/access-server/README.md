# Access Server 模块

模块在本地 TCP 端口提供以下端点：

| 路径 | 内容 |
|---|---|
| `/` | 内置的 UTF-8 欢迎页 |
| `/health/live`、`/health/ready` | 进程和组件状态 |
| `/api/v1/terminal/status` | 当前终端快照 |
| `/ws` | 状态快照以及 `access.granted`、`access.denied` 事件 |

欢迎页源码内嵌在 `access_server.cpp`，不依赖 Node.js 或外部 CDN。正式终端使用配置的端口；仿真程序从 `8080` 到 `8089` 选择第一个空闲端口。

服务默认只监听回环地址。若允许其他设备访问，应先配置 TLS、认证、来源限制和网络隔离，不要直接暴露到公网。
