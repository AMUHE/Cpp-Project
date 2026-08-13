# 配置参考

配置文件为 UTF-8 JSON，当前 `schemaVersion` 为 1。查找顺序：`SAW_CONFIG_PATH`、可执行文件同级 `config.json`、当前目录 `config/config.json`。缺少配置文件时使用安全默认值。

| 路径 | 默认值 | 约束/说明 |
|---|---:|---|
| `device.id` | `terminal-demo-01` | 事件中的稳定设备标识 |
| `device.displayName` | 智能门禁终端 | 运维显示名称 |
| `camera.mode` | `single_device` | `single_device`、`single_stereo_frame` 或 `separate_devices` |
| `camera.primaryIndex` | 0 | 0–32；单摄像头时为内置或外接设备索引 |
| `camera.secondaryIndex` | 1 | 0–32；仅 `separate_devices` 使用，且不得与主索引相同 |
| `camera.frameWidth` | 1280 | 320–7680 |
| `camera.frameHeight` | 720 | 240–4320 |
| `camera.captureIntervalMs` | 30 | 10–1000 ms |
| `recognition.confidenceThreshold` | 80 | LBPH 距离上限，越小越严格 |
| `recognition.minimumAccuracy` | 20 | 归一化匹配准确度下限；低于该值直接拒绝 |
| `recognition.requiredConsecutiveMatches` | 3 | 1–100 |
| `recognition.eventCooldownSeconds` | 10 | 0–3600 秒 |
| `server.bindAddress` | `127.0.0.1` | 默认只允许本机访问 |
| `server.httpPort` | 8080 | 1–65535 |
| `welcome.displayDurationSeconds` | 5 | 1–300 秒 |
| `welcome.grantedTemplate` | `欢迎，{displayName}` | 支持姓名占位符 |
| `storage.databaseFile` | `access.db` | 相对路径基于数据目录 |
| `storage.retentionDays` | 90 | 1–3650 天 |
| `speech.enabled` | true | 无引擎时自动降级 |
| `speech.locale` | `zh_CN` | 优先精确区域，其次同语言 |
| `speech.rate` | 0 | -1 到 1 |
| `speech.volume` | 1 | 0 到 1 |

超出数值范围的值回退到安全默认值；无效 JSON、未知 schema 或非法监听地址会拒绝加载。未知字段被忽略，以支持同一 schema 的向前兼容。

摄像头模式说明：

- `single_device`：普通单摄像头，适用于笔记本内置摄像头、USB 摄像头和采集卡；整帧用于预览与识别。
- `single_stereo_frame`：一个设备输出左右拼接的双目画面；完整画面用于预览，左半帧用于识别。
- `separate_devices`：同时打开两个独立设备；拼接预览，主设备画面用于识别。

Windows 通常将内置摄像头分配为索引 `0`，外接设备可能是 `1` 或更高。索引由系统和连接顺序决定，摄像头被会议软件等程序占用时可能无法打开。

生产环境不应监听 `0.0.0.0`。确需远程访问时，应由受管反向代理提供 TLS、身份认证、来源控制、限流和安全日志，并将终端网络置于隔离 VLAN。
