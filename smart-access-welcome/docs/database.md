# 数据库设计

## 隔离位置

SQLite 审计库默认位于应用本地数据根目录的 `database/access.db`，与 `biometrics/` 中的人脸样本、LBPH 模型和姓名标签物理隔离。可通过 `SAW_DATA_DIR` 改变整个运行数据根目录。

程序不会把运行数据库写入源码或构建目录。`.gitignore` 同时排除数据库、WAL/SHM、模型、标签和人脸样本。

数据库启动参数：

```sql
PRAGMA foreign_keys=ON;
PRAGMA journal_mode=WAL;
PRAGMA synchronous=NORMAL;
PRAGMA busy_timeout=5000;
```

## `access_events`

| 字段 | 类型 | 说明 |
|---|---|---|
| `id` | TEXT PK | 事件 UUID |
| `device_id` | TEXT NOT NULL | 终端标识 |
| `person_id` | TEXT NULL | 人脸身份 UUID 或账号身份标识 |
| `display_name` | TEXT NULL | 事件发生时的显示姓名或账号 |
| `decision` | TEXT | `granted` 或 `denied` |
| `reason` | TEXT | `recognized`、`unknown_person`、`password_login`、`invalid_password` 等 |
| `confidence` | REAL | 0–100 的归一化识别准确度；账号验证事件为 0 |
| `door_action` | TEXT | `requested`、`simulated_unlocked`、`failed` 或 `not_requested` |
| `occurred_at` | TEXT | 带时区的 ISO 8601 时间 |

`occurred_at` 建有降序索引。程序启动时按 `storage.retentionDays` 删除过期审计事件，默认保留 90 天。数据库文件本身、人脸样本和识别模型不会自动删除。

## 放行事务顺序

程序先以 `door_action=requested` 写入放行意图；写入成功后才请求门锁，并把结果更新为 `simulated_unlocked` 或 `failed`。数据库不可用时拒绝开门。

## 备份与恢复

- 停止终端后备份，或使用 SQLite Online Backup API；
- 使用 WAL 模式时，应一致性备份数据库及相关 WAL/SHM 状态；
- 备份属于敏感审计数据，应加密并限制访问；
- 恢复后运行 `PRAGMA integrity_check`，确认设备 ID 和时区后再开放门禁。
