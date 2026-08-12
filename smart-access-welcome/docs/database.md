# 数据库设计

## 存储位置与模式

SQLite 数据库默认位于应用本地数据目录的 `access.db`。启动时启用外键、WAL、`synchronous=NORMAL` 和 5 秒 busy timeout。每个存储实例使用独立 Qt SQL 连接名，避免默认连接冲突。

## `access_events`

| 字段 | 类型 | 说明 |
|---|---|---|
| `id` | TEXT PK | 事件 UUID |
| `device_id` | TEXT NOT NULL | 终端标识 |
| `person_id` | TEXT NULL | 放行人员稳定 UUID；拒绝事件为空 |
| `display_name` | TEXT NULL | 当时的显示姓名 |
| `decision` | TEXT | `granted` 或 `denied` |
| `reason` | TEXT | `recognized`、`unknown_person` 等机器原因 |
| `confidence` | REAL | 当前 LBPH 距离；不是概率 |
| `door_action` | TEXT | 模拟解锁、失败或未请求 |
| `occurred_at` | TEXT | 带时区 ISO 8601 |

`occurred_at` 建有降序索引。启动时按 `storage.retentionDays` 清理过期记录。当前版本不存截图，避免扩大生物特征暴露面。

## 迁移策略

当前迁移为幂等建表。新增字段应优先使用可空字段或带默认值字段；破坏性变更必须增加 `schema_migrations` 版本表、备份验证和回滚脚本。生产升级前应复制数据库及 WAL/SHM 文件，停机后做一致性校验。

## 备份和恢复

- 在终端停止或使用 SQLite Online Backup API 时备份。
- 备份属于敏感审计数据，必须加密、限制访问并执行保留期。
- 恢复后运行 `PRAGMA integrity_check`，确认设备 ID 和时区设置，再开放门禁流程。
