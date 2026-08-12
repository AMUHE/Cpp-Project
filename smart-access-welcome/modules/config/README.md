# Config 模块

公开接口：`include/saw/config/app_config.h`。

负责读取 UTF-8 JSON 配置、应用安全默认值并校验数值范围。当前 `schemaVersion` 为 `1`；缺少配置文件时程序仍可使用默认值启动。

默认摄像头模式为 `single_device`，索引为 `0`，分辨率为 `1280 × 720`。完整字段参考 `docs/configuration.md`。
