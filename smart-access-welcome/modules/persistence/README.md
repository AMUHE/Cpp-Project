# Persistence 模块

公开接口：`include/saw/persistence/access_event_store.h`

模块使用 SQLite WAL 保存门禁事件，支持新增事件、回写门锁动作、查询近期记录和按保留期清理。数据库路径来自配置，默认位于 Qt 应用数据目录。

数据库、WAL 文件和真实门禁记录都是运行数据，已被 Git 忽略，也不应作为测试夹具提交。
