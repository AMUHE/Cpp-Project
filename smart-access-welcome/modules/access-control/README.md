# Access Control 模块

公开接口：`include/saw/access/access_policy.h`。

这是不依赖 Qt 的门禁策略模块，负责同一身份连续匹配、未知人员连续拒绝和事件冷却。它只产生 `Granted`、`Denied` 或继续等待的决定，不直接访问摄像头、数据库、网页或门锁。

策略参数来自配置文件，行为由 `tests/access_policy_test.cpp` 覆盖。
