# Device 模块

公开接口：`include/saw/device/door_controller.h`

当前实现是 `SimulatedDoorController`。开锁后，定时器会将它恢复为上锁状态，并通过信号发送状态变化。正式终端和 `CameraSimulationDemo` 都使用这个模拟器，它不会控制真实继电器。

真实门锁适配器应提供超时、结果反馈、断线状态和默认上锁行为，并且只接受策略层给出的放行决定。
