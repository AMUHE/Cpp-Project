# Smart Access Welcome

Smart Access Welcome 是一个基于 C++17、Qt Widgets 和 OpenCV 的本地智能门禁原型。主程序支持人脸录入与识别、账号密码开门、模拟门锁、SQLite 审计、中文语音播报，以及自动打开的浏览器欢迎页。仓库还包含一个无需人脸模型的摄像头与门禁仿真程序。

> 本项目用于研发、教学和流程验证，当前没有活体检测，也没有接入真实门锁。不要未经安全评估直接用于生产门禁。

## 当前功能

- 单摄像头、左右拼接双目画面和双设备采集；
- Haar Cascade 人脸检测和 LBPH 本地识别；
- 录入过程显示 `0/20` 采集进度；
- 人脸样本、模型和 UTF-8 姓名标签持久保存，重启后自动加载；
- 模型缺失时从已有样本自动重建，无需重复录入；
- 模型与标签通过 SHA-256 清单绑定，不一致时拒绝混用；
- 画面显示归一化识别准确度，默认最低值为 20%；
- 同一身份连续匹配 3 次后才允许开门；
- 多张人脸同时出现时拒绝放行；
- 账号密码登录开门为常态入口，与人脸识别并列使用；
- 开门前必须先写入审计记录，数据库异常时保持关门；
- 模拟门锁按配置时间自动复位；
- HTTP/WebSocket 欢迎页随主程序启动，并自动在浏览器打开；
- 中文语音异步播报；
- 审计记录按保留期限自动清理，默认保留 90 天；
- 支持 CMake、qmake、Debug/Release 和自动化测试。

## 安全行为

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
回写结果并推送欢迎页
```

人脸放行需要同时满足：

1. 检测画面中只有一张人脸；
2. LBPH 距离不超过 `confidenceThreshold`；
3. 归一化准确度不低于 `minimumAccuracy`；
4. 模型标签存在且模型清单校验通过；
5. 相同身份连续匹配达到 `requiredConsecutiveMatches`；
6. 审计数据库写入成功。

这里显示的“准确度”是由 LBPH 距离归一化得到的工程评分，不是统计学概率。

## 数据隔离

运行数据不写入源码仓库或构建目录。默认使用 Qt 的 `AppLocalDataLocation`；Windows 通常为：

```text
C:\Users\<用户名>\AppData\Local\SmartAccessWelcome\SmartAccessWelcome\
├─ database\
│  ├─ access.db              # SQLite 审计数据库
│  ├─ access.db-wal          # SQLite WAL 文件，运行时可能存在
│  └─ access.db-shm
├─ biometrics\
│  ├─ faces\<姓名>\*.jpg    # 录入的人脸样本
│  ├─ model.yml              # LBPH 模型
│  ├─ labels.csv             # UTF-8 人员标签
│  └─ model.yml.manifest.json# 模型/标签 SHA-256 清单
└─ logs\
   └─ application.jsonl      # 结构化日志
```

隔离原则：

- `database/` 只保存门禁审计数据；
- `biometrics/` 只保存生物特征样本和识别模型；
- `logs/` 单独保存应用日志；
- `.gitignore` 排除数据库、WAL、模型、标签、人脸样本和本地配置；
- 从旧版本升级时，会从 `faces/` 或 `database/enrollment/` 非破坏性复制到 `biometrics/`，旧文件不会自动删除；
- 人脸样本和模型不会按时间自动删除；只有 `access_events` 中超过 `storage.retentionDays` 的审计记录会清理。

可通过 `SAW_DATA_DIR` 指定另一处独立数据根目录：

```powershell
$env:SAW_DATA_DIR = 'D:\SmartAccessData'
```

不要把真实人脸样本、数据库、日志或账号密码提交到 Git。

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
build/                       本地构建产物，不提交
```

主 qmake 工程为 `smart-access-welcome.pro`，仿真工程为 `camera-simulation-demo.pro`。

## 依赖

- C++17；
- CMake 3.21+；
- Qt 5.14.2+ 或 Qt 6；
- Qt Core、Widgets、Network、WebSockets、Sql、TextToSpeech；
- OpenCV，必须包含 `core`、`imgproc`、`imgcodecs`、`videoio`、`objdetect` 和 contrib `face`；
- Windows 使用 Qt 5.14.2 MinGW 32 位时，OpenCV 架构必须一致。

生产环境推荐 64 位 Qt/OpenCV 4.x。仓库中的 Qt 5.14.2、MinGW 32 位和 OpenCV 3.4.5 配置属于兼容环境。

## 配置

复制示例配置：

```powershell
Copy-Item config/config.example.json config/config.json
```

关键字段：

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

默认账号密码仅用于本地原型演示。实际部署必须修改密码，并应改为加盐密码哈希或外部身份系统，不能长期保存明文凭据。

摄像头模式：

| 模式 | 说明 |
|---|---|
| `single_device` | 内置或 USB 单摄像头，完整画面用于预览和识别 |
| `single_stereo_frame` | 单设备输出左右拼接画面，左半画面用于识别 |
| `separate_devices` | 两个独立摄像头，主设备用于识别 |

环境变量：

| 变量 | 用途 |
|---|---|
| `SAW_DATA_DIR` | 指定隔离的运行数据根目录 |
| `SAW_CONFIG_PATH` | 指定运行配置文件 |
| `SAW_CASCADE_PATH` | 指定 Haar 人脸检测器 XML |
| `OPENCV_ROOT` | qmake 构建时指定 OpenCV 安装目录 |

## 构建主程序

### qmake（当前 Windows 兼容环境）

```powershell
$env:OPENCV_ROOT = 'E:/path/to/opencv/install'
New-Item -ItemType Directory -Force build/terminal-release
Set-Location build/terminal-release
qmake ../../smart-access-welcome.pro "CONFIG+=release"
mingw32-make -j4
```

输出：

```text
build/terminal-release/bin/SmartAccessWelcome.exe
```

构建流程会把 OpenCV DLL 和 `haarcascade_frontalface_default.xml` 部署到程序目录。

### CMake

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug --output-on-failure
```

## 使用流程

1. 启动 `SmartAccessWelcome.exe`；
2. 本地服务成功后，默认浏览器自动打开欢迎页；
3. 选择摄像头模式和索引，点击“启动摄像头”；
4. 首次使用点击“录入新人员”，输入姓名并完成 20 张样本采集；
5. 点击“开始身份识别”，观察人脸框、姓名和准确度；
6. 也可以随时点击“账号密码登录开门”；
7. 放行、拒绝和门锁状态同步到浏览器欢迎页并写入审计数据库。

重新启动后，程序直接加载已保存的模型。若模型缺失但样本仍在，会自动重建模型。

## 摄像头性能

预览画面持续刷新，人脸识别默认每 3 帧计算一次，录入检测每 2 帧计算一次；中间帧复用最近的检测框。检测图像最大宽度缩放到 640 像素，摄像头缓冲设置为 1 帧，以降低 UI 卡顿和画面积压。

## 浏览器欢迎页与 API

服务默认监听 `127.0.0.1:8080`：

| 路径 | 用途 |
|---|---|
| `/` | 浏览器欢迎页 |
| `/health/live` | 存活检查 |
| `/health/ready` | 摄像头、识别器、数据库和语音状态 |
| `/api/v1/terminal/status` | 当前终端快照 |
| `/ws` | WebSocket 实时事件 |

服务默认只面向本机。若监听局域网地址，必须另行增加 TLS、认证、访问控制和速率限制。

## 仿真程序

`CameraSimulationDemo` 不依赖人脸样本或识别模型，可使用合成画面或真实摄像头演示欢迎页和模拟门锁：

```powershell
New-Item -ItemType Directory -Force build/camera-demo
Set-Location build/camera-demo
qmake ../../camera-simulation-demo.pro "CONFIG+=release"
mingw32-make -j4
```

## 测试

现有测试覆盖：

- 连续身份匹配、拒绝和事件冷却；
- HTTP 健康检查、状态接口和 WebSocket；
- 配置边界、SQLite 持久化和模拟门锁自动复位。

```powershell
ctest --preset windows-debug --output-on-failure
```

## 当前限制

1. 没有活体检测，无法可靠防御照片、屏幕翻拍或面具攻击；
2. LBPH 适合小规模本地原型，对光照、姿态和遮挡较敏感；
3. 摄像头读取和视觉计算仍位于 UI 线程，高分辨率或低性能设备可能卡顿；
4. 账号密码当前来自本地明文配置，只适合受控演示环境；
5. 人员以显示姓名组织样本，重名、改名和停用管理仍不完善；
6. 当前只有模拟门锁，未实现真实继电器、串口或 GPIO 适配器；
7. HTTP 服务没有内置 TLS 或管理认证；
8. 识别“准确度”是工程评分，需要按现场样本校准，不是概率。

## 文档

- [系统架构](docs/architecture.md)
- [配置说明](docs/configuration.md)
- [数据库设计](docs/database.md)
- [API](docs/api.md)
- [Windows 部署](docs/deployment-windows.md)
- [运维手册](docs/operations.md)
- [安全与隐私](docs/security-and-privacy.md)
- [测试策略](docs/testing.md)
- [仿真程序说明](docs/camera-simulation-demo.md)

## 许可证

项目代码使用 [MIT License](LICENSE)。Qt、OpenCV、语音引擎、检测模型和设备驱动分别适用其自身许可证。
