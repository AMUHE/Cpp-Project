# 正式终端

`SmartAccessWelcome` 是主程序。入口为 `main.cpp`，窗口由 `main_window.ui` 和 `main_window.cpp` 实现。

## 运行流程

1. 按配置打开内置摄像头、USB 摄像头或双目设备。
2. 录入人员并训练 LBPH 模型，或加载已有模型。
3. 将连续识别结果交给 `access-control` 判断。
4. 记录 SQLite 审计事件并调用模拟门锁。
5. 通过 HTTP/WebSocket 向浏览器发送终端状态和门禁结果。

欢迎页默认位于 `http://127.0.0.1:8080/`，监听地址和端口可在 `config.json` 中修改。

## UI 维护

界面应只包含产品名、时间、组件状态、实时画面、摄像头设置和必要的操作按钮。教程和技术说明统一维护在 `docs/`。

业务代码使用的控件名记录在 `docs/ui-design.md`。修改控件名时，需要同步更新 `main_window.cpp`。
