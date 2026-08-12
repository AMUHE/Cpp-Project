# Face 原型迁移说明

首批迁移来源为 `E:\Cpp\Face`。旧窗口中的职责已拆分为：

- `modules/camera`：电脑内置/外接单摄像头、单设备左右拼接双目、双设备采集和识别帧裁切。
- `modules/vision`：级联检测、LBPH 模型训练、加载和预测。
- `apps/terminal`：Qt 终端界面与帧处理编排。

终端现已保留旧原型的完整视觉闭环：打开摄像头、检测人脸、按姓名采集
20 张归一化灰度样本、重新训练 LBPH 模型并立即启用识别。重复录入同名人员时
会追加带时间戳的样本，不会覆盖已有训练数据。

运行数据不再写入程序目录，而是写入 Qt 的 `AppLocalDataLocation`。检测模型按以下顺序查找：

1. 环境变量 `SAW_CASCADE_PATH`；
2. 可执行文件同级目录；
3. 可执行文件同级的 `cascades` 目录。

旧 `03chatServer` 使用自定义 TCP 文本协议，与目标 HTTP/WebSocket API 不兼容，因此没有直接复制。后续网络模块只复用其连接上限、缓冲区上限和断线清理思路。

网络迁移现已实现 HTTP 与 WebSocket 共端口服务，支持 `/health/live`、
`/health/ready`、`/api/v1/terminal/status` 和 `/ws`。旧项目的真实回环连接
测试方式已迁移为 HTTP 响应与 WebSocket 初始快照测试。

识别结果现经过独立的 `access-control` 策略模块：同一身份连续匹配三次才放行，
未知人员连续出现三次才拒绝，并分别执行十秒冷却。只有策略决策会发布为
`access.granted` 或 `access.denied`，原始逐帧预测不会直接暴露给网页。
