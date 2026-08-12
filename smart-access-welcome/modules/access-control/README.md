# Access Control 模块

公开接口：`include/saw/access/access_policy.h`

这是一个不依赖 Qt 的门禁策略模块，处理同一身份的连续匹配、未知人员连续拒绝和事件冷却。它只返回 `Granted`、`Denied` 或继续等待，不直接访问摄像头、数据库、网页或门锁。

策略参数来自配置文件，相关测试位于 `tests/access_policy_test.cpp`。
