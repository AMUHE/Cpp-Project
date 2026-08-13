# 摄像头与门禁仿真 Demo

`CameraSimulationDemo` 是一个独立的小型桌面程序，用来演示“画面采集 → 模拟识别 → 门禁反馈 → 自动复位”的完整交互，不读取人脸模型、人员数据库或真实门锁。

## 功能

- 默认生成动态人像、检测框和扫描线，无摄像头也能运行。
- 可切换到电脑内置摄像头或 USB 外接摄像头，使用设备索引选择画面来源。
- “模拟识别通过”显示欢迎信息，模拟门锁开启 5 秒后自动上锁。
- “模拟识别拒绝”显示拒绝状态，门锁始终保持关闭。
- 启动内置 HTTP/WebSocket 欢迎页，并将通过、拒绝和复位状态实时推送到浏览器。
- 页面记录画面启动、识别结果、门锁开启和自动复位事件。

该程序只仿真识别结果与门锁动作，不声称执行了真实人脸识别或活体检测。

## 运行流程

1. 启动程序，保留默认的“合成仿真画面（无需摄像头）”。
2. 点击“在浏览器中打开欢迎页”。服务优先使用 `http://127.0.0.1:8080/`，端口被占用时会自动尝试到 `8089`，页面显示的地址始终是实际端口。
3. 点击“启动画面”。
4. 修改仿真人员姓名，点击“模拟识别通过”，同时观察桌面端、浏览器欢迎页、开锁倒计时和自动上锁事件。
5. 点击“模拟识别拒绝”，观察浏览器同步显示拒绝状态，门锁保持关闭。
6. 如需测试真实画面，先停止画面，选择“电脑内置或外接摄像头”，设置索引后重新启动。

内置摄像头通常为索引 `0`。外接设备可能为 `1`、`2` 或其他索引，具体由 Windows 和连接顺序决定。如果打开失败，请关闭正在使用摄像头的会议软件或浏览器页面后重试。

## CMake 构建

Demo 会随主工程一起构建：

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug --target saw_camera_demo
```

生成的程序名为 `CameraSimulationDemo.exe`。

## Qt 5.14.2 qmake 构建

先按开发指南配置 `config/opencv.local.pri`，然后使用独立构建目录：

```powershell
New-Item -ItemType Directory -Force build/camera-demo
Set-Location build/camera-demo
qmake ../../camera-simulation-demo.pro
mingw32-make -j4
./bin/CameraSimulationDemo.exe
```

如果不从 Qt Creator 启动，需要确保 Qt、MinGW 和 OpenCV DLL 所在目录已加入当前进程的 `Path`，或使用 `windeployqt` 部署运行依赖。

## 无界面冒烟验证

CI 或开发机可用合成画面执行短时冒烟测试。该模式自动启动画面、触发一次识别通过，并在 1.5 秒后退出：

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
./bin/CameraSimulationDemo.exe --smoke-test
```

冒烟模式不会打开真实摄像头。
