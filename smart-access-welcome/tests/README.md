# 自动化测试

测试覆盖：

- `access_policy_test.cpp`：连续匹配、拒绝与冷却策略；
- `access_server_test.cpp`：HTTP、欢迎页和 WebSocket 初始快照；
- `infrastructure_test.cpp`：配置默认值、SQLite 往返和模拟门锁复位；
- `qmake/`：Qt 5.14.2 迁移环境的独立测试工程。

CMake 使用 `ctest --preset windows-debug`。旧 MinGW 环境使用：

```powershell
.\scripts\run-tests.ps1 -QtRoot <Qt目录> -MingwRoot <MinGW目录>
```

测试不得依赖真实摄像头、人脸样本、门锁或外部网络。硬件行为应通过模拟器或受控验收流程验证。
