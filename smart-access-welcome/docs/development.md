# 开发指南

## 1. 工程目标

项目采用本地优先的模块化单体架构。实时摄像头与识别路径不依赖外部网络；网络欢迎页是只读消费者。所有放行决定必须先经过 `access-control`，原始逐帧识别结果不得直接开门或发布到网页。

## 2. 目录结构

```text
apps/terminal/          Qt 桌面终端和流程编排
modules/camera/         摄像头适配
modules/vision/         检测、训练和识别
modules/access-control/ 连续匹配和冷却策略
modules/device/         门锁适配器（当前为模拟器）
modules/persistence/    SQLite 审计存储
modules/speech/         异步语音播报
modules/access-server/  HTTP、WebSocket、内置欢迎页
modules/config/         JSON 配置加载和边界校验
tests/                  单元、集成和协议测试
docs/                   架构、接口、运维和安全文档
```

依赖方向由终端应用向模块单向汇聚。领域策略不依赖 Qt，便于确定性测试；I/O 模块通过公开头文件暴露最小接口。

### 摄像头采集模式

`modules/camera` 统一输出识别帧和预览帧，终端页面默认使用 `single_device`，因此开发机可直接使用电脑内置摄像头或 USB 外接摄像头。`primaryIndex` 选择实际设备（内置设备通常为 `0`，外接设备常为 `1` 或更高，具体由系统分配），`secondaryIndex` 只在 `separate_devices` 模式启用。

兼容模式 `single_stereo_frame` 用于单设备输出的左右拼接双目画面，`separate_devices` 用于两台独立摄像头。新增采集模式时必须明确哪一帧用于识别、哪一帧用于预览，并保持单摄像头路径不依赖第二设备。

### 正式终端文案规则

`apps/terminal/main_window.ui` 是现场运行界面，不作为使用手册。页面只保留产品名、时间、服务与组件状态、实时画面、摄像头接入字段以及启动、停止、录入、识别操作。操作流程、隐私边界、部署限制和摄像头索引排查方法必须写入 `docs/`，不得以教程卡片、英文眉题或长段落重新加入正式终端。

运行时错误和状态栏消息可以保留定位问题所必需的信息，但应使用短句，不重复页面上已经可见的状态。仿真程序 `CameraSimulationDemo` 面向开发演示，可保留必要的仿真标识和事件记录，不受正式终端精简规则约束。

## 3. 开发环境

推荐 Qt 6.x、OpenCV 4.x + contrib 和 Ninja。Windows 上必须保证 Qt、编译器和 OpenCV 架构一致；32 位库不能与 64 位程序链接。

```powershell
.\scripts\check-env.ps1
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

遗留 Qt 5.14.2 MinGW 32 位环境使用 `smart-access-welcome.pro`，只用于迁移验证。新生产构建应切换到 64 位 Qt/OpenCV。

Qt Creator 直接打开 `.pro` 时，先复制本地 OpenCV 配置：

```powershell
Copy-Item config/opencv.local.pri.example config/opencv.local.pri
```

编辑 `opencv.local.pri` 中的 `OPENCV_ROOT`。该文件被 Git 忽略，qmake 会自动加载；修改后在 Qt Creator 中执行“构建 → 运行 qmake”。

## 4. 编码规则

- C++17，RAII 管理资源，不裸持有跨模块所有权。
- 头文件放在 `include/saw/<module>/`，命名空间与模块一致。
- UI 线程只编排和渲染；语音、设备和数据库不得执行无上限阻塞操作。
- 时间对外使用带时区 ISO 8601，内部冷却使用单调或显式毫秒时间。
- 文件落盘优先 `QSaveFile`；数据库使用参数绑定，禁止拼接用户输入。
- 错误消息可供操作员定位，但网络响应不得泄漏本机路径、栈或生物特征。
- 每项行为变更必须同步测试和对应文档；协议破坏性变更提升 schema 版本。

## 5. 新模块接入

1. 在 `modules/<name>` 创建公开头文件、实现和 `CMakeLists.txt`。
2. 创建 `SAW::<Name>` 别名目标，只公开必要依赖。
3. 在 `modules/CMakeLists.txt` 注册。
4. 添加无硬件单元测试；硬件行为通过模拟器注入。
5. 更新架构、安全边界和配置参考。

## 6. 门锁适配器开发要求

真实适配器必须具备超时、明确的成功/失败反馈、断线状态和默认上锁行为。禁止把识别回调直接接到 GPIO/继电器；调用只能来自策略产生的 `Granted` 决策。驱动异常必须记录审计事件，并保持或恢复安全状态。

## 7. Definition of Done

- Debug 和 Release 构建通过，无新增编译警告。
- 自动化测试通过，硬件路径有模拟测试。
- 配置具有默认值、边界校验和文档。
- API、数据库或事件变更已记录兼容策略。
- 不包含真实人脸、凭据、绝对开发机路径和生成文件。
- 完成安全、隐私、回滚和可观测性影响评审。
