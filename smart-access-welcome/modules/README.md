# 业务模块

模块通过 `SAW::<Name>` CMake 别名目标提供能力，应用层只负责编排。依赖应保持单向，领域策略不得依赖终端 UI。

| 目录 | CMake 目标 | 职责 |
|---|---|---|
| `camera/` | `SAW::Camera` | 单摄像头、拼接双目和双设备采集，输出预览帧与识别帧 |
| `vision/` | `SAW::Vision` | Haar 检测、样本归一化、LBPH 训练与识别 |
| `access-control/` | `SAW::AccessControl` | 连续匹配、拒绝和冷却策略 |
| `device/` | `SAW::Device` | 门锁接口；当前实现为自动复位模拟器 |
| `persistence/` | `SAW::Persistence` | SQLite WAL 审计事件存储 |
| `access-server/` | `SAW::AccessServer` | HTTP 状态接口、内置欢迎页和 WebSocket 推送 |
| `speech/` | `SAW::Speech` | 异步中文语音播报与故障降级 |
| `config/` | `SAW::Config` | JSON 配置加载、默认值和范围校验 |
| `logging/` | `SAW::Logging` | 本地结构化日志 |

新增模块时需提供公开头文件、最小依赖、CMake 目标、无硬件测试和对应文档。摄像头、人脸、数据库和日志产生的运行数据不得提交到仓库。
