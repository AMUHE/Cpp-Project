# Device 模块

公开接口：`include/saw/device/door_controller.h`。

当前提供 `SimulatedDoorController`：开锁后使用有界定时器自动恢复上锁，并通过信号通知状态变化。它用于正式终端的软件演示和 `CameraSimulationDemo`，不控制真实继电器。

新增真实门锁适配器时必须提供超时、结果反馈、断线状态和默认上锁行为，并且只能接受策略层产生的放行决定。
