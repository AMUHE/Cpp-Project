# Smart Access Welcome

这是一个本地运行的智能门禁终端参考实现。程序使用 Qt 构建界面和网络服务，通过 OpenCV 完成人脸检测与 LBPH 识别，并把门禁事件写入 SQLite。仓库还提供不需要人脸模型或真实门锁的仿真程序。

> 本项目没有经过安防认证，不应直接用于真实门禁。接入真实门锁前，至少需要补充活体检测、管理员认证、TLS、密钥管理、硬件失效安全评估和隐私合规验收。

## 主要功能

- 支持单摄像头、单设备左右拼接画面和双摄像头采集；
- Haar 人脸检测、样本录入、LBPH 训练及本地识别；
- 连续匹配确认、未知人员拒绝和按身份冷却；
- 自动复位的模拟门锁；
- SQLite WAL 审计和按保留期清理；
- Qt TextToSpeech 中文播报；
- HTTP 健康检查、终端状态接口、WebSocket 事件和内置欢迎页；
- CMake 与 qmake 构建、Qt Test 测试和 GitHub Actions 流程。

## 依赖

- C++17、CMake 3.21+
- Qt 5.14.2+ 或 Qt 6：Core、Widgets、Network、Sql、Test、WebSockets、TextToSpeech
- OpenCV 4.x，包含 contrib/face 模块

目标部署平台为 Windows 10/11 x64，CI 也会在 Ubuntu 上检查构建。Qt 5.14.2、32 位 MinGW 和 OpenCV 3.4.5 仅用于旧环境迁移。

## 快速开始

先复制示例配置：

```powershell
Copy-Item config/config.example.json config/config.json
```

默认使用索引 `0` 的单摄像头。连接 USB 摄像头后如果仍显示内置摄像头，可在终端中尝试索引 `1`、`2`。只有设备输出左右拼接画面或使用两台摄像头时，才需要更改摄像头模式。

使用 CMake 构建和测试：

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

旧版 Qt/MinGW 环境可以使用 qmake：

```powershell
$env:OPENCV_ROOT = 'E:/path/to/opencv/install'
qmake SmartAccessWelcome.pro
mingw32-make -j4
```

运行前需指定 Haar 级联模型，也可以将模型放在可执行文件同级的 `cascades` 目录：

```powershell
$env:SAW_CASCADE_PATH = 'C:/models/haarcascade_frontalface_default.xml'
.\SmartAccessWelcome.exe
```

如果只需查看门禁流程，可运行 `CameraSimulationDemo`。它使用合成画面模拟识别通过、拒绝、开锁倒计时、自动复位和浏览器欢迎页，操作方法见[仿真程序说明](docs/camera-simulation-demo.md)。

## 环境变量

| 变量 | 用途 |
|---|---|
| `SAW_CONFIG_PATH` | 指定 JSON 配置文件 |
| `SAW_DATA_DIR` | 指定本地数据目录 |
| `SAW_CASCADE_PATH` | 指定 Haar 级联模型 |
| `OPENCV_ROOT` | 指定 qmake 使用的 OpenCV 安装目录 |

## 运行数据

默认数据目录是 Qt 的 `AppLocalDataLocation`：

- `faces/<姓名>/`：录入的人脸样本；
- `model.yml`、`labels.csv`：LBPH 模型和标签；
- `access.db`：门禁事件数据库。

这些文件均被 Git 忽略。不要提交真实人脸、数据库、凭据或门禁日志。

## 文档

- [系统架构](docs/architecture.md)
- [Qt UI 设计](docs/ui-design.md)
- [摄像头与门禁仿真](docs/camera-simulation-demo.md)
- [开发指南](docs/development.md)
- [配置参考](docs/configuration.md)
- [API 协议](docs/api.md)
- [数据库设计](docs/database.md)
- [语音播报](docs/speech.md)
- [测试策略](docs/testing.md)
- [Windows 部署](docs/deployment-windows.md)
- [运维手册](docs/operations.md)
- [安全与隐私](docs/security-and-privacy.md)
- [开发路线图](docs/roadmap.md)
- [原型迁移说明](docs/migration-face.md)

## 许可证

项目代码使用 [MIT License](LICENSE)。Qt、OpenCV、语音引擎、模型文件和摄像头驱动分别适用各自的许可证。
