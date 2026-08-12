# Camera 模块

公开接口：`include/saw/camera/stereo_camera.h`

| `CameraMode` | 配置值 | 行为 |
|---|---|---|
| `SingleDevice` | `single_device` | 打开一个内置、USB 或采集卡设备，完整帧用于预览和识别 |
| `SideBySideDevice` | `single_stereo_frame` | 接收单设备的左右拼接画面，完整帧用于预览，左半帧用于识别 |
| `SeparateDevices` | `separate_devices` | 打开两台设备并拼接预览，主设备帧用于识别 |

Windows 上优先使用 DirectShow，失败后回退到 OpenCV 默认后端。设备索引由操作系统分配，模块不保存设备名称。

普通单摄像头模式不会打开第二台设备，也不会裁剪识别帧。目前尚未实现硬件断线后的自动恢复；读取失败由应用层提示，门锁保持关闭。
