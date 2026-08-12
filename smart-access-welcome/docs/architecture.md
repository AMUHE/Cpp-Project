# 系统架构

## 架构原则

- 本地优先：识别和门禁决策默认在终端完成。
- 拒绝优先：依赖异常、结果不确定或策略不可用时保持门锁关闭。
- 端口适配：摄像头、识别器、数据库、网页服务和门锁通过接口隔离。
- 隐私最小化：欢迎流程只传递展示所需信息。
- 事件驱动：识别结果成为领域事件，由 UI、门锁和网页分别消费。

## 组件关系

```text
Qt Terminal UI
      │
Camera Adapter -> Face Detector -> Recognizer -> Recognition Result
                                                  │
                                           Access Policy
                                                  │
                                    ┌─────────────┴─────────────┐
                                    │                           │
                             Door Controller              Event Store
                                    │                           │
                                    └──────── Event Bus ────────┘
                                              │        │
                                      Speech Worker  HTTP/WebSocket Server
                                                       │
                                                Browser Welcome UI
```

## 模块职责

| 模块 | 职责 | 不应承担 |
|---|---|---|
| `camera` | 枚举设备、采集帧、恢复断线 | 身份判断、数据库访问 |
| `vision` | 人脸检测、归一化、标签预测 | 开门、网页推送 |
| `identity` | 人员元数据、录入、模型版本 | 操作 UI 控件 |
| `access-control` | 权限、连续匹配、冷却、状态机 | 直接控制摄像头 |
| `device` | 模拟门锁及硬件适配接口 | 决定谁有权限 |
| `persistence` | SQLite 仓储、迁移、事务 | 识别算法 |
| `access-server` | HTTP、WebSocket、静态资源 | 修改识别置信度 |
| `config` | JSON 加载、默认值和边界校验 | 动态执行业务逻辑 |
| `speech` | 有界队列、异步语音合成和降级 | 参与授权决策 |

## 门禁状态机

```text
IDLE -> DETECTING -> VERIFYING -> GRANTED -> COOLDOWN -> IDLE
                         │
                         ├────────> DENIED -> IDLE
                         └────────> ERROR  -> IDLE/DEGRADED
```

- `VERIFYING` 累计配置数量的连续一致匹配。
- `GRANTED` 先持久化 `requested` 事件，再调用门锁；门锁结果回写事件状态。审计不可用时拒绝开门。
- `COOLDOWN` 按人员与设备组合去重，不全局阻塞其他人员。
- 识别器、数据库或策略异常进入拒绝路径。

## 核心数据模型

- `Person`：不可变 UUID、展示名、状态、创建与更新时间。
- `BiometricProfile`：人员 ID、识别引擎、模型版本、撤销时间。
- `AccessEvent`：事件 ID、设备、可空人员 ID、决策、原因、置信度、门锁动作和时间。

## 并发与故障隔离

- 摄像头与 UI 编排运行在 Qt 主事件循环，单帧处理必须有界。
- 语音引擎运行在专用 `QThread`；SAPI 初始化、播报或后端错误不会阻塞 HTTP 与识别。
- SQLite 使用独立命名连接、WAL 和 busy timeout。门锁动作必须以成功写入审计意图为前置条件。
- WebSocket 只消费策略后的领域事件，不接收原始逐帧预测。
- 组件状态通过 `/health/ready` 暴露；非关键语音故障降级，关键审计故障拒绝放行。

## 迁移策略

旧 `Face/MainWindow` 的摄像头、检测和 LBPH 代码先以行为保持方式迁入，再拆分模块。硬编码 OpenCV 路径、应用目录数据存储和 UI 内训练逻辑不得保留。`03chatServer` 的 TCP 分帧和测试思路可参考，欢迎流程改用带版本的 HTTP/WebSocket 协议。
