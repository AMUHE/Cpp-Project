# Config 模块

公开接口：`include/saw/config/app_config.h`

模块读取 UTF-8 JSON 配置，补充默认值并校验数值范围。当前 `schemaVersion` 为 `1`；找不到配置文件时，程序仍可使用默认值启动。

默认摄像头模式为 `single_device`，索引为 `0`，分辨率为 `1280 × 720`。全部字段见 [`docs/configuration.md`](../../docs/configuration.md)。
