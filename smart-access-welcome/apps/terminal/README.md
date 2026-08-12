# 正式终端

`SmartAccessWelcome` 是现场运行主程序，入口为 `main.cpp`，窗口定义在 `main_window.ui` 和 `main_window.cpp`。

## 运行闭环

1. 根据接入方式打开电脑内置、USB 外接或兼容双目摄像头。
2. 录入人员并训练本地 LBPH 模型，或加载已有模型。
3. 连续识别结果经 `access-control` 策略确认。
4. 写入 SQLite 审计后调用模拟门锁。
5. 通过 HTTP/WebSocket 服务将欢迎或拒绝状态推送到浏览器。

欢迎页默认地址为 `http://127.0.0.1:8080/`，具体地址由 `config.json` 控制。

## UI 约束

正式运行界面只保留产品名、时间、组件状态、实时画面、摄像头字段以及启动、停止、录入、识别按钮。不要在 `.ui` 中加入工作流教程、隐私说明、技术背景或长段落；相关内容应更新到 `docs/`。

业务代码依赖的控件名称记录在 `docs/ui-design.md`，改名时必须同步修改 `main_window.cpp`。
