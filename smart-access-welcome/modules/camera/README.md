# Camera 模块

公开接口：`include/saw/camera/stereo_camera.h`。

支持三种模式：

| `CameraMode` | 配置值 | 行为 |
|---|---|---|
| `SingleDevice` | `single_device` | 电脑内置、USB 外接摄像头或采集卡；完整帧用于预览和识别 |
| `SideBySideDevice` | `single_stereo_frame` | 单设备输出左右拼接画面；完整帧预览，左半帧识别 |
| `SeparateDevices` | `separate_devices` | 同时打开两个设备并拼接预览，主设备帧用于识别 |

Windows 优先使用 DirectShow，失败后回退到 OpenCV 默认后端。设备索引由操作系统分配；模块不枚举或持久化设备名称。

普通单摄像头路径不得要求第二设备，也不得裁切识别帧。硬件断连恢复尚属路线图任务，当前读取失败由应用层显示异常并保持门锁关闭。
