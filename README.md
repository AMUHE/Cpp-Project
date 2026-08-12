# C/C++ 学习与项目实践

这个仓库集中维护三个彼此独立的 C/C++ 项目，覆盖 EasyX 游戏、Win32 桌面应用，以及基于 Qt/OpenCV 的智能门禁终端。各项目的依赖、工具链和运行数据相互隔离，请进入对应目录构建，不要在仓库根目录混合编译。

## 项目一览

| 项目 | 技术栈 | 当前状态 | 入口 |
|---|---|---|---|
| 五子棋（Gomoku） | C/C++、EasyX | 已完成；支持 15×15 双人对弈与四方向判胜 | [项目说明](gomoku/README.md) |
| 学生管理系统（Student System） | C++、Win32 API、STL、fstream | 已完成；保留 Visual Studio 2010 兼容性 | [项目说明](student-system/README.md) |
| 智能门禁与网页欢迎系统（Smart Access Welcome） | C++17、Qt、OpenCV、SQLite、HTTP/WebSocket | 可运行的参考实现；终端、仿真 Demo 和自动化测试已落地 | [项目说明](smart-access-welcome/README.md) |

## Smart Access Welcome

当前仓库的主要项目位于 [`smart-access-welcome/`](smart-access-welcome/)。它已经实现从摄像头采集到欢迎页推送的本地闭环：

- 单摄像头、左右拼接双目画面和双设备采集；
- Haar 人脸检测、样本录入、LBPH 训练与本地识别；
- 连续匹配、未知人员拒绝、身份冷却和模拟门锁复位；
- SQLite WAL 审计、中文语音播报；
- HTTP 健康检查、终端状态接口、WebSocket 实时事件和响应式欢迎页；
- 独立 `CameraSimulationDemo`，无需真实摄像头或人脸数据即可演示门禁流程；
- CMake 与 qmake 构建，以及门禁策略、服务端和基础设施测试。

快速体验仿真 Demo 或构建完整终端前，请先阅读 [Smart Access Welcome 快速开始](smart-access-welcome/README.md#快速开始)。详细设计见[系统架构](smart-access-welcome/docs/architecture.md)，接口见 [API 协议](smart-access-welcome/docs/api.md)，当前计划见[开发路线图](smart-access-welcome/docs/roadmap.md)。

> 该项目是本地参考实现和演示终端，不是经过安防认证的门禁产品。连接真实门锁前仍需补充活体检测、管理员认证、TLS、密钥管理、硬件失效安全与隐私合规验收。

## 开发环境概览

- Gomoku：Windows，以及受 EasyX 支持的 Visual Studio 环境。
- Student System：Windows、Visual Studio 2010 或更高版本，使用多字节字符集。
- Smart Access Welcome：C++17、CMake 3.21+、Qt 5.14.2+ 或 Qt 6、带 `face` 模块的 OpenCV。

具体版本、构建命令和设备配置以各子项目 README 为准。Smart Access Welcome 的现代构建目标为 Windows 10/11 x64；仓库也保留 Qt 5.14.2、MinGW 32 位和 OpenCV 3.4.5 的迁移构建入口。

## 仓库约定

- 每个项目在独立子目录中维护源码、文档和构建说明。
- 构建产物、本地配置、凭据、数据库、日志、模型和个人数据不得提交。
- Smart Access Welcome 的人脸样本及运行数据只保存在本机，并已纳入忽略规则。
- 新功能通过分支和 Pull Request 合并到 `main`；目录规范见[仓库维护指南](docs/repository-layout.md)。

## 当前注意事项

- Smart Access Welcome 的工作流定义目前保存在子项目目录中，尚未接入仓库根目录的 GitHub Actions 自动发现路径；提交前请在本地执行对应测试。
- Student System 仍有部分历史字符串编码问题，且二进制数据文件不保证跨编译器或跨架构兼容。
- Gomoku 依赖 Windows/EasyX，目前不支持 Linux 或 macOS。

保持好奇，持续编码。
