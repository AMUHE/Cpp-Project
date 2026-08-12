# Access Server 模块

该模块在一个本地 TCP 端口提供：

- `/`：内置 UTF-8 浏览器欢迎页；
- `/health/live`、`/health/ready`：进程和组件状态；
- `/api/v1/terminal/status`：终端当前快照；
- `/ws`：状态快照及 `access.granted`、`access.denied` 事件。

欢迎页源码当前内嵌在 `access_server.cpp`，不需要 Node.js 或外部 CDN。正式终端使用配置端口，仿真程序在 `8080–8089` 中选择首个可用端口。

默认只监听回环地址。若要让其他设备访问，必须先完成 TLS、认证、来源限制和网络隔离，不应直接将该服务暴露到公网。
