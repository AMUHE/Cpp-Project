# C/C++ 项目合集

仓库中有三个相互独立的项目。它们使用不同的工具链，请进入各自目录构建。

| 项目 | 简介 | 环境 |
|---|---|---|
| [五子棋](gomoku/README.md) | 15×15 本地双人五子棋 | C/C++、EasyX、Windows |
| [学生管理系统](student-system/README.md) | 带文件存储的 Win32 桌面程序 | C++、Win32 API、Visual Studio |
| [Smart Access Welcome](smart-access-welcome/README.md) | 摄像头、人脸识别、门禁策略和网页欢迎屏的参考实现 | C++17、Qt、OpenCV、SQLite |

## Smart Access Welcome

这是目前内容最多的子项目，位于 [`smart-access-welcome/`](smart-access-welcome/)。它包含完整终端和一个不依赖人脸数据的仿真程序，可演示以下流程：

- 摄像头采集及 Haar 人脸检测；
- LBPH 样本训练和本地识别；
- 连续匹配、拒绝、冷却和模拟开锁；
- SQLite 审计、中文语音播报；
- HTTP 状态接口、WebSocket 事件和浏览器欢迎页。

构建方法见[项目 README](smart-access-welcome/README.md)，设计和接口分别见[系统架构](smart-access-welcome/docs/architecture.md)与 [API 协议](smart-access-welcome/docs/api.md)。

> Smart Access Welcome 用于本地开发和演示，未经过安防认证。接入真实门锁前，需要另行完成活体检测、管理员认证、TLS、密钥管理、硬件失效安全评估和隐私合规验收。

## 仓库约定

- 源码、文档和构建说明放在对应的项目目录中。
- 不提交构建产物、本地配置、凭据、数据库、日志、模型或个人数据。
- Smart Access Welcome 的人脸样本和运行数据只应保存在本机。
- 目录说明见[仓库维护指南](docs/repository-layout.md)。
