# 配置目录

- `config.example.json`：可提交的安全示例配置，默认使用索引 `0` 的普通单摄像头。
- `config.json`：本地运行配置，已被 Git 忽略。
- `opencv.local.pri.example`：qmake 的 OpenCV 路径模板。
- `opencv.local.pri`：本机 OpenCV 路径，已被 Git 忽略。

启动时依次查找 `SAW_CONFIG_PATH`、可执行文件同级 `config.json` 和当前目录下的 `config/config.json`。字段和约束见 `docs/configuration.md`。

不要在可提交文件中写入凭据、真实人员数据或开发机绝对路径。
