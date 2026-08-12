# Persistence 模块

公开接口：`include/saw/persistence/access_event_store.h`。

使用 SQLite WAL 保存门禁审计事件，支持追加事件、回写门锁动作、查询近期事件和按保留期清理。数据库路径来自配置，默认位于 Qt 应用数据目录。

数据库、WAL 文件和真实门禁记录均属于运行数据，已被 Git 忽略，不得作为测试夹具提交。
