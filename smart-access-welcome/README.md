# Smart Access Welcome

我用 C++17、Qt Widgets 和 OpenCV 开发了 Smart Access Welcome，把它做成了一个本地优先的智能门禁原型。我在主程序里实现了人脸录入与识别、账号密码开门、模拟门锁、SQLite 审计、中文语音播报和浏览器欢迎页；我还保留了一个不依赖人脸模型的仿真程序，方便我单独验证摄像头、欢迎页和门禁流程。

> 我把这个项目定位为研发、教学和流程验证原型。我目前没有实现活体检测，也没有接入真实门锁，因此我不会把它未经安全评估直接用于生产门禁。

## 我实现的功能

- 我支持单摄像头、左右拼接双目画面和两个独立摄像头；
- 我使用 Haar Cascade 检测人脸，使用 LBPH 完成本地训练和识别；
- 我在录入时显示 `0/20` 的采集进度；
- 我持久保存人脸样本、模型和 UTF-8 姓名标签，并在重启后自动加载；
- 我在模型缺失但样本仍存在时自动重建模型，不要求重新录入；
- 我用 SHA-256 清单绑定模型与标签，发现不一致时拒绝混用；
- 我在画面上显示归一化识别准确度，默认最低值为 20%；
- 我要求同一身份连续匹配 3 次后才允许开门；
- 我在画面中同时出现多张人脸时拒绝放行；
- 我把账号密码登录开门作为常态入口，与人脸识别并列提供；
- 我在开门前先写入审计记录，数据库异常时保持关门；
- 我让模拟门锁按配置时间自动复位；
- 我随主程序启动 HTTP/WebSocket 服务，并自动打开浏览器欢迎页；
- 我异步执行中文语音播报；
- 我按保留期限清理审计记录，默认保留 90 天；
- 我同时维护 CMake、qmake、Debug/Release 和自动化测试流程。

## 我的安全策略

我采用 fail-closed（失败时保持关闭）策略：

```text
我执行人脸识别或账号密码验证
              │
              ▼
        我执行通行策略判断
              │
              ▼
      我写入 SQLite 审计意图
           │        │
         成功       失败
           │        └── 我拒绝开门
           ▼
       我请求门锁开门
           │
           ▼
  我回写结果并推送浏览器欢迎页
```

我只在人脸识别同时满足以下条件时放行：

1. 我在检测画面中只找到一张人脸；
2. 我得到的 LBPH 距离不超过 `confidenceThreshold`；
3. 我计算的归一化准确度不低于 `minimumAccuracy`；
4. 我能找到对应标签，并且模型清单校验通过；
5. 我连续得到相同身份的次数达到 `requiredConsecutiveMatches`；
6. 我成功写入审计数据库。

我把界面里的“准确度”定义为由 LBPH 距离归一化得到的工程评分，而不是统计学概率。

## 我如何隔离数据

我不会把运行数据写进源码仓库或构建目录。我默认使用 Qt 的 `AppLocalDataLocation`；在 Windows 上，我通常把数据放在：

```text
C:\Users\<用户名>\AppData\Local\SmartAccessWelcome\SmartAccessWelcome\
├─ database\
│  ├─ access.db               # 我保存的 SQLite 审计数据库
│  ├─ access.db-wal           # 我运行 SQLite 时可能产生的 WAL 文件
│  └─ access.db-shm
├─ biometrics\
│  ├─ faces\<姓名>\*.jpg     # 我录入的人脸样本
│  ├─ model.yml               # 我训练的 LBPH 模型
│  ├─ labels.csv              # 我保存的 UTF-8 人员标签
│  └─ model.yml.manifest.json # 我生成的模型/标签 SHA-256 清单
└─ logs\
   └─ application.jsonl       # 我输出的结构化日志
```

我按以下方式隔离这些数据：

- 我只在 `database/` 中保存门禁审计数据；
- 我只在 `biometrics/` 中保存生物特征样本和识别模型；
- 我单独使用 `logs/` 保存应用日志；
- 我用 `.gitignore` 排除数据库、WAL、模型、标签、人脸样本和本地配置；
- 我在升级时从旧的 `faces/` 或 `database/enrollment/` 非破坏性复制到 `biometrics/`，不会自动删除旧文件；
- 我不会按时间删除人脸样本和模型，只会清理 `access_events` 中超过 `storage.retentionDays` 的审计记录。

我可以通过 `SAW_DATA_DIR` 指定另一处数据根目录：

```powershell
$env:SAW_DATA_DIR = 'D:\SmartAccessData'
```

我不会把真实人脸样本、数据库、日志或实际账号密码提交到 Git。

## 我的项目结构

```text
apps/
├─ terminal/                 我维护的主门禁程序
└─ camera-demo/              我维护的摄像头与门禁仿真程序
modules/
├─ access-control/           我实现的连续匹配、拒绝和冷却策略
├─ access-server/            我实现的 HTTP、WebSocket 与欢迎页
├─ camera/                   我封装的摄像头采集与画面适配
├─ config/                   我实现的 JSON 配置加载和边界校验
├─ device/                   我实现的模拟门锁
├─ logging/                  我实现的 JSON Lines 日志
├─ persistence/              我实现的 SQLite 审计存储
├─ speech/                   我实现的异步语音播报
└─ vision/                   我实现的人脸检测、训练与识别
config/                      我提供的配置示例和本地 OpenCV 配置
docs/                        我维护的架构、API、部署和运维文档
scripts/                     我维护的 Windows 构建与测试脚本
tests/                       我编写的自动化测试
build/                       我在本地生成且不提交的构建产物
```

我把主 qmake 工程命名为 `smart-access-welcome.pro`，把仿真工程命名为 `camera-simulation-demo.pro`。

## 我使用的依赖

- 我使用 C++17；
- 我要求 CMake 3.21+；
- 我支持 Qt 5.14.2+ 或 Qt 6；
- 我使用 Qt Core、Widgets、Network、WebSockets、Sql 和 TextToSpeech；
- 我使用 OpenCV 的 `core`、`imgproc`、`imgcodecs`、`videoio`、`objdetect` 和 contrib `face`；
- 我在 Windows 的 Qt 5.14.2 MinGW 32 位环境中使用相同架构的 OpenCV。

我建议生产环境使用 64 位 Qt 和 OpenCV 4.x。我只把 Qt 5.14.2、MinGW 32 位和 OpenCV 3.4.5 作为兼容环境保留。

## 我如何配置程序

我先复制示例配置：

```powershell
Copy-Item config/config.example.json config/config.json
```

我主要调整以下字段：

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

我只把默认账号密码用于本地原型演示。我在实际部署前会修改密码，并把明文比较替换为加盐密码哈希或外部身份系统。

我支持三种摄像头模式：

| 模式 | 我的处理方式 |
|---|---|
| `single_device` | 我使用内置或 USB 单摄像头，并用完整画面预览和识别 |
| `single_stereo_frame` | 我接收单设备左右拼接画面，并用左半画面识别 |
| `separate_devices` | 我同时打开两个摄像头，并用主设备识别 |

我使用以下环境变量：

| 变量 | 我的用途 |
|---|---|
| `SAW_DATA_DIR` | 我用它指定隔离的运行数据根目录 |
| `SAW_CONFIG_PATH` | 我用它指定运行配置文件 |
| `SAW_CASCADE_PATH` | 我用它指定 Haar 人脸检测器 XML |
| `OPENCV_ROOT` | 我用它在 qmake 构建时指定 OpenCV 安装目录 |

## 我如何构建主程序

### qmake（我的 Windows 兼容环境）

```powershell
$env:OPENCV_ROOT = 'E:/path/to/opencv/install'
New-Item -ItemType Directory -Force build/terminal-release
Set-Location build/terminal-release
qmake ../../smart-access-welcome.pro "CONFIG+=release"
mingw32-make -j4
```

我在这里得到主程序：

```text
build/terminal-release/bin/SmartAccessWelcome.exe
```

我在构建过程中自动部署 OpenCV DLL 和 `haarcascade_frontalface_default.xml`。

### CMake

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug --output-on-failure
```

## 我的使用顺序

1. 我启动 `SmartAccessWelcome.exe`；
2. 我等待本地服务启动，浏览器会自动打开欢迎页；
3. 我选择摄像头模式和索引，再点击“启动摄像头”；
4. 我在首次使用时点击“录入新人员”，输入姓名并完成 20 张样本采集；
5. 我点击“开始身份识别”，查看人脸框、姓名和准确度；
6. 我也可以随时点击“账号密码登录开门”；
7. 我在浏览器欢迎页查看放行、拒绝和门锁状态，并在审计数据库中保留记录。

我重新启动程序后会直接加载已经保存的模型。模型缺失但样本仍在时，我会自动重建模型。

## 我如何降低摄像头卡顿

我持续刷新预览画面，但只每 3 帧执行一次人脸识别，每 2 帧执行一次录入检测；我在中间帧复用最近的检测框。我把检测图像最大宽度缩放到 640 像素，并把摄像头缓冲设置为 1 帧，以降低 UI 卡顿和画面积压。

## 我的浏览器欢迎页与 API

我默认监听 `127.0.0.1:8080`：

| 路径 | 我提供的能力 |
|---|---|
| `/` | 我提供浏览器欢迎页 |
| `/health/live` | 我提供进程存活检查 |
| `/health/ready` | 我返回摄像头、识别器、数据库和语音状态 |
| `/api/v1/terminal/status` | 我返回当前终端快照 |
| `/ws` | 我通过 WebSocket 推送实时事件 |

我默认只面向本机提供服务。如果我要监听局域网地址，我会另行增加 TLS、认证、访问控制和速率限制。

## 我的仿真程序

我让 `CameraSimulationDemo` 独立于人脸样本和识别模型运行，并支持用合成画面或真实摄像头演示欢迎页和模拟门锁：

```powershell
New-Item -ItemType Directory -Force build/camera-demo
Set-Location build/camera-demo
qmake ../../camera-simulation-demo.pro "CONFIG+=release"
mingw32-make -j4
```

## 我的测试

我用自动化测试覆盖：

- 我验证连续身份匹配、拒绝和事件冷却；
- 我验证 HTTP 健康检查、状态接口和 WebSocket；
- 我验证配置边界、SQLite 持久化和模拟门锁自动复位。

```powershell
ctest --preset windows-debug --output-on-failure
```

## 我目前保留的限制

1. 我还没有实现活体检测，因此无法可靠防御照片、屏幕翻拍或面具攻击；
2. 我使用的 LBPH 更适合小规模本地原型，对光照、姿态和遮挡较敏感；
3. 我仍在 UI 线程读取摄像头并执行视觉计算，高分辨率或低性能设备可能卡顿；
4. 我目前从本地明文配置读取账号密码，只适合受控演示环境；
5. 我按显示姓名组织人员样本，对重名、改名和停用的管理仍不完善；
6. 我目前只提供模拟门锁，还没有实现继电器、串口或 GPIO 适配器；
7. 我还没有为 HTTP 服务内置 TLS 或管理认证；
8. 我显示的识别“准确度”是工程评分，需要按现场样本校准，不是概率。

## 我维护的文档

- [我对系统架构的说明](docs/architecture.md)
- [我对配置项的说明](docs/configuration.md)
- [我对数据库设计的说明](docs/database.md)
- [我定义的 API](docs/api.md)
- [我的 Windows 部署说明](docs/deployment-windows.md)
- [我的运维手册](docs/operations.md)
- [我的安全与隐私说明](docs/security-and-privacy.md)
- [我的测试策略](docs/testing.md)
- [我的仿真程序说明](docs/camera-simulation-demo.md)

## 我的许可证说明

我使用 [MIT License](LICENSE) 发布项目代码。我使用的 Qt、OpenCV、语音引擎、检测模型和设备驱动分别遵循各自的许可证。
