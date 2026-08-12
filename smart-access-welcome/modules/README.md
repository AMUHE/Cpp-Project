# 业务模块

模块以 `SAW::<Name>` CMake 别名目标提供。应用层负责组合模块，领域策略不依赖终端 UI。

| 目录 | CMake 目标 | 职责 |
|---|---|---|
| `camera/` | `SAW::Camera` | 单摄像头、左右拼接画面和双设备采集 |
| `vision/` | `SAW::Vision` | Haar 检测、样本处理、LBPH 训练和识别 |
| `access-control/` | `SAW::AccessControl` | 连续匹配、拒绝和冷却策略 |
| `device/` | `SAW::Device` | 门锁接口及自动复位模拟器 |
| `persistence/` | `SAW::Persistence` | SQLite WAL 审计事件存储 |
| `access-server/` | `SAW::AccessServer` | HTTP 接口、内置欢迎页和 WebSocket 推送 |
| `speech/` | `SAW::Speech` | 异步中文语音播报 |
| `config/` | `SAW::Config` | JSON 配置加载、默认值和校验 |
| `logging/` | `SAW::Logging` | 本地结构化日志 |

新增模块时需要提供公开头文件、CMake 目标、必要测试和对应文档，并尽量减少依赖。摄像头、人脸、数据库和日志产生的运行数据不得提交。
