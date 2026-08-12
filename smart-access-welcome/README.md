# Smart Access Welcome

Smart Access Welcome 是一个本地优先的智能门禁终端参考实现。它将通用摄像头采集、OpenCV 人脸检测与 LBPH 识别、连续匹配门禁策略、模拟门锁、SQLite 审计、中文语音播报以及 HTTP/WebSocket 欢迎屏整合为一个可测试的 Qt 应用。

> 安全边界：当前版本是企业工程基线和演示终端，不是经过安防认证的真实门禁产品。部署到真实门锁前必须完成活体检测、管理员认证、硬件失效安全评估、TLS、密钥管理和隐私合规验收。

## 已实现能力

- 默认支持电脑内置摄像头和 USB 等外接单摄像头，同时兼容单设备左右拼接双目画面与两个独立摄像头。
- Haar 级联人脸检测、样本录入、LBPH 训练和本地识别。
- 连续匹配确认、未知人员拒绝和按身份冷却去重。
- 模拟门锁自动复位，硬件适配器接口边界清晰。
- SQLite WAL 审计事件库和按保留期清理。
- Qt TextToSpeech 中文播报；引擎运行在工作线程，故障时安全降级。
- 同端口 HTTP 健康检查、终端状态接口和 WebSocket 实时事件。
- 内置响应式欢迎页：访问 `http://127.0.0.1:8080/`。
- 配置校验、请求大小限制、连接上限和基础安全响应头。
- CMake、qmake、Qt Test、GitHub Actions 和 Windows 部署流程。

## 技术基线

- C++17、CMake 3.21+
- Qt 5.14.2+ 或 Qt 6：Core、Widgets、Network、Sql、Test、WebSockets、TextToSpeech
- OpenCV 4.x + contrib/face；旧有 3.4.5 只作为迁移兼容工具链
- Windows 10/11 x64 为目标部署平台；CI 在 Ubuntu 上验证可移植构建

## 快速开始

复制配置并根据设备调整：

```powershell
Copy-Item config/config.example.json config/config.json
```

默认配置使用索引 `0` 的单摄像头，通常对应电脑内置摄像头。连接 USB 摄像头后，如果画面仍来自内置设备，请在终端页面依次尝试索引 `1`、`2`；只有双目设备才需要切换接入方式。

如果只想快速体验页面和门禁流程，可运行独立的 `CameraSimulationDemo`。它默认使用合成画面，并提供模拟识别通过、拒绝、开锁倒计时、自动复位以及实时浏览器欢迎页，完整步骤见[摄像头与门禁仿真 Demo](docs/camera-simulation-demo.md)。

CMake 构建：

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

当前 Qt 5.14.2/MinGW 迁移环境也可以使用 qmake：

```powershell
$env:OPENCV_ROOT = 'E:/path/to/opencv/install'
qmake SmartAccessWelcome.pro
mingw32-make -j4
```

运行前设置检测模型，或将模型放到可执行文件同级/`cascades` 子目录：

```powershell
$env:SAW_CASCADE_PATH = 'C:/models/haarcascade_frontalface_default.xml'
.\SmartAccessWelcome.exe
```

可选环境变量：

| 变量 | 用途 |
|---|---|
| `SAW_CONFIG_PATH` | 指定 JSON 配置文件 |
| `SAW_DATA_DIR` | 覆盖本地数据目录，便于服务账户和测试隔离 |
| `SAW_CASCADE_PATH` | 指定人脸检测级联模型 |
| `OPENCV_ROOT` | qmake 构建时指定 OpenCV 安装根目录 |

## 运行数据

默认写入 Qt `AppLocalDataLocation`，包括：

- `faces/<姓名>/`：录入样本；
- `model.yml`、`labels.csv`：LBPH 模型与标签；
- `access.db`：门禁审计事件。

这些内容均被 Git 忽略。禁止提交真实人脸、数据库、凭据或门禁日志。

## 文档导航

- [系统架构](docs/architecture.md)
- [Qt UI 设计与 Designer 交付](docs/ui-design.md)
- [摄像头与门禁仿真 Demo](docs/camera-simulation-demo.md)
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

项目代码使用 [MIT License](LICENSE)。Qt、OpenCV、语音引擎、模型文件及摄像头驱动分别遵循其自身许可证。
