# 配置文件

| 文件 | 说明 |
|---|---|
| `config.example.json` | 可提交的示例配置，默认使用索引 `0` 的单摄像头 |
| `config.json` | 本机运行配置，已被 Git 忽略 |
| `opencv.local.pri.example` | qmake 的 OpenCV 路径模板 |
| `opencv.local.pri` | 本机 OpenCV 路径，已被 Git 忽略 |

程序依次查找 `SAW_CONFIG_PATH`、可执行文件同级的 `config.json` 和当前目录下的 `config/config.json`。字段说明见 [`docs/configuration.md`](../docs/configuration.md)。

不要在可提交的配置中写入凭据、真实人员信息或开发机的绝对路径。
