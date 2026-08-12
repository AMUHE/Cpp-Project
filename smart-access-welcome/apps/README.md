# 应用程序

这里包含两个可以独立运行的 Qt Widgets 应用。

| 目录 | 构建目标 | 用途 |
|---|---|---|
| `terminal/` | `saw_terminal` / `SmartAccessWelcome.exe` | 完整终端，包括采集、录入、识别、门禁策略、审计、语音、模拟门锁和欢迎页服务 |
| `camera-demo/` | `saw_camera_demo` / `CameraSimulationDemo.exe` | 不需要人脸模型的仿真程序，可使用合成画面或单摄像头 |

两个应用复用 `modules/` 中的实现。摄像头、门锁和 HTTP/WebSocket 代码应留在对应模块中，不在应用目录重复实现。

正式终端的界面只保留运行时需要的状态、设备信息和操作。使用教程、设备排查和安全说明放在 `docs/` 中；仿真程序可以显示事件记录和欢迎页地址，便于开发演示。
