# Smart Access Welcome

Smart Access Welcome 是一个使用 C++17、Qt Widgets 和 OpenCV 开发的本地智能门禁原型。主程序提供人脸录入与识别、账号密码开门、模拟门锁、SQLite 审计、中文语音播报和浏览器欢迎页；项目还包含一个不依赖人脸模型的仿真程序，可单独验证摄像头、欢迎页和门禁流程。

> 本项目面向研发、教学和流程验证，尚未实现活体检测，也未接入真实门锁。未经完整的安全评估，不应直接用于生产门禁。

## 功能特性

- 支持单摄像头、左右拼接双目画面和两个独立摄像头；
- 使用 Haar Cascade 检测人脸，使用 LBPH 完成本地训练和识别；
- 人脸录入时显示 `0/20` 的采集进度；
- 持久保存人脸样本、模型和 UTF-8 姓名标签，重启后自动加载；
- 模型缺失但样本仍存在时自动重建，无需重新录入；
- 使用 SHA-256 清单绑定模型与标签，发现不一致时拒绝混用；
- 在画面上显示归一化识别准确度，默认最低值为 20%；
- 同一身份连续匹配 3 次后才允许开门；
- 画面中同时出现多张人脸时拒绝放行；
- 账号密码登录开门与人脸识别并列提供；
- 开门前写入审计记录，数据库异常时保持关门；
- 模拟门锁按配置时间自动复位；
- 主程序启动 HTTP/WebSocket 服务并自动打开浏览器欢迎页；
- 异步执行中文语音播报；
- 按保留期限清理审计记录，默认保留 90 天；
- 同时提供 CMake、qmake、Debug/Release 和自动化测试流程。

## 安全策略

系统采用 fail-closed（失败时保持关闭）策略：

```text
人脸识别或账号密码验证
           │
           ▼
      通行策略判断
           │
           ▼
   写入 SQLite 审计意图
        │        │
      成功       失败
        │        └── 拒绝开门
        ▼
     请求门锁开门
        │
        ▼
回写结果并推送浏览器欢迎页
```

人脸识别仅在以下条件全部满足时放行：

1. 检测画面中只有一张人脸；
2. LBPH 距离不超过 `confidenceThreshold`；
3. 归一化准确度不低于 `minimumAccuracy`；
4. 标签存在，并且模型清单校验通过；
5. 相同身份的连续匹配次数达到 `requiredConsecutiveMatches`；
6. 审计记录成功写入数据库。

界面中的“准确度”是由 LBPH 距离归一化得到的工程评分，并非统计学概率。

## 数据隔离

运行数据不会写入源码仓库或构建目录。默认数据根目录由 Qt 的 `AppLocalDataLocation` 决定；在 Windows 上通常位于：

```text
C:\Users\<用户名>\AppData\Local\SmartAccessWelcome\SmartAccessWelcome\
├─ database\
│  ├─ access.db               # SQLite 审计数据库
│  ├─ access.db-wal           # SQLite 运行时可能产生的 WAL 文件
│  └─ access.db-shm
├─ biometrics\
│  ├─ faces\<姓名>\*.jpg     # 已录入的人脸样本
│  ├─ model.yml               # LBPH 模型
│  ├─ labels.csv              # UTF-8 人员标签
│  └─ model.yml.manifest.json # 模型/标签 SHA-256 清单
└─ logs\
   └─ application.jsonl       # 结构化日志
```

数据按用途隔离：

- `database/` 仅保存门禁审计数据；
- `biometrics/` 仅保存生物特征样本和识别模型；
- `logs/` 单独保存应用日志；
- `.gitignore` 排除数据库、WAL、模型、标签、人脸样本和本地配置；
- 升级时会从旧的 `faces/` 或 `database/enrollment/` 非破坏性复制到 `biometrics/`，不会自动删除旧文件；
- 人脸样本和模型不会按时间删除，只有 `access_events` 中超过 `storage.retentionDays` 的审计记录会被清理。

可通过 `SAW_DATA_DIR` 指定其他数据根目录：

```powershell
$env:SAW_DATA_DIR = 'D:\SmartAccessData'
```

请勿将真实人脸样本、数据库、日志或实际账号密码提交到 Git。

## 项目结构

```text
apps/
├─ terminal/                 主门禁程序
└─ camera-demo/              摄像头与门禁仿真程序
modules/
├─ access-control/           连续匹配、拒绝和冷却策略
├─ access-server/            HTTP、WebSocket 与欢迎页
├─ camera/                   摄像头采集与画面适配
├─ config/                   JSON 配置加载和边界校验
├─ device/                   模拟门锁
├─ logging/                  JSON Lines 日志
├─ persistence/              SQLite 审计存储
├─ speech/                   异步语音播报
└─ vision/                   人脸检测、训练与识别
config/                      配置示例和本地 OpenCV 配置
docs/                        架构、API、部署和运维文档
scripts/                     Windows 构建与测试脚本
tests/                       自动化测试
build/                       本地生成且不提交的构建产物
```

主 qmake 工程为 `smart-access-welcome.pro`，仿真工程为 `camera-simulation-demo.pro`。

## 依赖

- C++17；
- CMake 3.21+；
- Qt 5.14.2+ 或 Qt 6；
- Qt Core、Widgets、Network、WebSockets、Sql 和 TextToSpeech；
- OpenCV `core`、`imgproc`、`imgcodecs`、`videoio`、`objdetect` 和 contrib `face`；
- Windows 下 Qt 与 OpenCV 必须使用相同的编译器和目标架构。

生产环境建议使用 64 位 Qt 和 OpenCV 4.x。Qt 5.14.2、MinGW 32 位和 OpenCV 3.4.5 仅作为兼容环境保留。

## 配置

复制示例配置：

```powershell
Copy-Item config/config.example.json config/config.json
```

主要配置字段如下：

```json
{
  "camera": {
    "mode": "single_device",
    "primaryIndex": 0,
    "frameWidth": 1280,
    "frameHeight": 720,
    "captureIntervalMs": 30
  },
  "recognition": {
    "confidenceThreshold": 80.0,
    "minimumAccuracy": 20.0,
    "requiredConsecutiveMatches": 3,
    "eventCooldownSeconds": 10
  },
  "passwordAccess": {
    "enabled": true,
    "username": "admin",
    "password": "123456"
  },
  "storage": {
    "databaseFile": "access.db",
    "retentionDays": 90
  }
}
```

默认账号密码仅用于本地原型演示。实际部署前必须修改密码，并应将明文比较替换为加盐密码哈希或外部身份系统。

### 摄像头模式

| 模式 | 处理方式 |
|---|---|
| `single_device` | 使用内置或 USB 单摄像头，以完整画面预览和识别 |
| `single_stereo_frame` | 接收单设备左右拼接画面，使用左半画面识别 |
| `separate_devices` | 同时打开两个摄像头，使用主设备识别 |

### 环境变量

| 变量 | 用途 |
|---|---|
| `SAW_DATA_DIR` | 指定隔离的运行数据根目录 |
| `SAW_CONFIG_PATH` | 指定运行配置文件 |
| `SAW_CASCADE_PATH` | 指定 Haar 人脸检测器 XML |
| `OPENCV_ROOT` | 在 qmake 构建时指定 OpenCV 安装目录 |

## 构建主程序

### qmake（Windows 兼容环境）

```powershell
$env:OPENCV_ROOT = 'E:/path/to/opencv/install'
New-Item -ItemType Directory -Force build/terminal-release
Set-Location build/terminal-release
qmake ../../smart-access-welcome.pro "CONFIG+=release"
mingw32-make -j4
```

主程序生成于：

```text
build/terminal-release/bin/SmartAccessWelcome.exe
```

构建过程会自动部署 OpenCV DLL 和 `haarcascade_frontalface_default.xml`。

### CMake

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug --output-on-failure
```

## 使用流程

1. 启动 `SmartAccessWelcome.exe`；
2. 等待本地服务启动，浏览器会自动打开欢迎页；
3. 选择摄像头模式和索引，点击“启动摄像头”；
4. 首次使用时点击“录入新人员”，输入姓名并完成 20 张样本采集；
5. 点击“开始身份识别”，查看人脸框、姓名和准确度；
6. 也可随时点击“账号密码登录开门”；
7. 在浏览器欢迎页查看放行、拒绝和门锁状态，审计记录会保存在数据库中。

程序重启后会直接加载已保存的模型。模型缺失但样本仍在时，会自动重建模型。

## 摄像头性能

预览画面会持续刷新，人脸识别每 3 帧执行一次，录入检测每 2 帧执行一次；中间帧复用最近的检测框。检测图像最大宽度缩放到 640 像素，摄像头缓冲设置为 1 帧，以降低 UI 卡顿和画面积压。

## 浏览器欢迎页与 API

服务默认监听 `127.0.0.1:8080`：

| 路径 | 功能 |
|---|---|
| `/` | 浏览器欢迎页 |
| `/health/live` | 进程存活检查 |
| `/health/ready` | 返回摄像头、识别器、数据库和语音状态 |
| `/api/v1/terminal/status` | 返回当前终端快照 |
| `/ws` | 通过 WebSocket 推送实时事件 |

服务默认仅供本机访问。监听局域网地址前，应另行增加 TLS、认证、访问控制和速率限制。

## 仿真程序

`CameraSimulationDemo` 独立于人脸样本和识别模型运行，可使用合成画面或真实摄像头演示欢迎页和模拟门锁：

```powershell
New-Item -ItemType Directory -Force build/camera-demo
Set-Location build/camera-demo
qmake ../../camera-simulation-demo.pro "CONFIG+=release"
mingw32-make -j4
```

## 测试

自动化测试覆盖：

- 连续身份匹配、拒绝和事件冷却；
- HTTP 健康检查、状态接口和 WebSocket；
- 配置边界、SQLite 持久化和模拟门锁自动复位。

```powershell
ctest --preset windows-debug --output-on-failure
```

## 已知限制

1. 尚未实现活体检测，无法可靠防御照片、屏幕翻拍或面具攻击；
2. LBPH 更适合小规模本地原型，对光照、姿态和遮挡较敏感；
3. 摄像头读取和视觉计算仍在 UI 线程执行，高分辨率或低性能设备可能卡顿；
4. 账号密码目前从本地明文配置读取，仅适合受控演示环境；
5. 人员样本按显示姓名组织，对重名、改名和停用的管理仍不完善；
6. 目前仅提供模拟门锁，尚未实现继电器、串口或 GPIO 适配器；
7. HTTP 服务尚未内置 TLS 或管理认证；
8. 识别“准确度”是需要按现场样本校准的工程评分，并非概率。

## 文档

- [系统架构](docs/architecture.md)
- [配置说明](docs/configuration.md)
- [数据库设计](docs/database.md)
- [API 文档](docs/api.md)
- [Windows 部署说明](docs/deployment-windows.md)
- [运维手册](docs/operations.md)
- [安全与隐私说明](docs/security-and-privacy.md)
- [测试策略](docs/testing.md)
- [仿真程序说明](docs/camera-simulation-demo.md)

## 许可证

项目代码采用 [MIT License](LICENSE)。Qt、OpenCV、语音引擎、检测模型和设备驱动分别遵循各自的许可证。
