# 摄像头与门禁仿真程序

`CameraSimulationDemo` 用于演示画面、识别结果、模拟门锁和浏览器欢迎页，不需要人脸模型、人员数据库或真实门锁。

- 默认生成动态合成画面，不访问摄像头；
- 可切换到内置或 USB 摄像头；
- 模拟通过时发送 `access.granted`，开锁 5 秒后复位；
- 模拟拒绝时发送 `access.denied`，门锁保持关闭；
- 优先监听 `127.0.0.1:8080`，端口占用时依次尝试到 `8089`；
- `--smoke-test` 会启动合成画面并触发一次通过事件。

构建和操作步骤见 [`docs/camera-simulation-demo.md`](../../docs/camera-simulation-demo.md)。
