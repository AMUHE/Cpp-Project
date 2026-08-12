# 应用程序

本目录包含两个可独立运行的 Qt Widgets 应用：

| 目录 | 构建目标 | 用途 |
|---|---|---|
| `terminal/` | `saw_terminal` / `SmartAccessWelcome.exe` | 正式门禁终端，包含摄像头、人脸录入与识别、策略、审计、语音、模拟门锁及欢迎页服务 |
| `camera-demo/` | `saw_camera_demo` / `CameraSimulationDemo.exe` | 无人脸模型也能运行的仿真程序，支持合成画面或真实单摄像头，并同步驱动浏览器欢迎页 |

正式终端页面遵循最小文案原则，只显示运行所需的状态、设备字段和操作按钮。操作教程、摄像头索引排查和安全边界放在 `docs/` 中。仿真程序面向开发演示，可以显示仿真标识、事件记录和欢迎页入口。

两个应用共用 `modules/` 中的能力，不应复制摄像头、门锁或 HTTP/WebSocket 实现。
