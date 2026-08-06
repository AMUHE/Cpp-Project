# C/C++ 学习与项目实践

欢迎来到我的 C/C++ 学习与项目实践仓库。这里以独立子项目的方式维护游戏、Windows 桌面应用和计算机视觉项目。

> 各项目依赖和构建方式不同，请进入对应目录阅读 README，不建议在仓库根目录混合编译。

## 项目目录

### 1. [五子棋游戏（Gomoku）](gomoku/)

基于 EasyX 图形库开发的经典 15×15 五子棋游戏。

- 核心技术：二维数组判胜、EasyX 绘图、控制台坐标输入。
- 状态：🟢 已完成。

### 2. [学生管理系统（Student System）](student-system/)

适配 Visual Studio 2010 的 Windows 桌面学生管理系统。

- 核心技术：Windows 消息循环、fstream 二进制持久化、STL Vector。
- 特点：淡紫色自定义 UI、多字段排序切换。
- 状态：🟢 已完成（优化版）。

### 3. [智能门禁与网页欢迎系统（Smart Access Welcome）](smart-access-welcome/)

基于 Qt、OpenCV、SQLite 和本地 Web 服务的智能门禁项目，目标流程为摄像头采集、人脸识别、门禁决策、网页实时欢迎和审计记录。

- 核心技术：C++17、CMake、Qt、OpenCV、HTTP/WebSocket、SQLite。
- 安全设计：拒绝优先、事件冷却、数据最小化、生物特征数据不进入 Git。
- 当前进度：🟡 工程、架构、API、安全和 Windows 部署基线完成，正在迁移视觉核心。
- 项目文档：[README](smart-access-welcome/README.md) · [架构](smart-access-welcome/docs/architecture.md) · [路线图](smart-access-welcome/docs/roadmap.md)

## 仓库约定

- 每个项目使用独立子目录维护源码和文档。
- 构建产物、本地配置、凭证、数据库和个人数据不得提交。
- 新功能通过分支和 Pull Request 合并到 `main`。
- 根目录规范见 [仓库维护指南](docs/repository-layout.md)。

保持好奇，持续编码。
