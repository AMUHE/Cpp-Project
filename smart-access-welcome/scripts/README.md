# Windows 脚本

- `check-env.ps1`：检查 CMake、Qt 和 OpenCV 环境。
- `build-windows-qmake.ps1`：使用 Qt 5.14.2/MinGW 迁移工具链构建并可选部署主程序。
- `run-tests.ps1`：构建并执行 qmake 测试工程。

脚本参数应传入 Qt、MinGW 和 OpenCV 根目录，不要把本机绝对路径写回可提交文件。生产构建应采用受支持的 64 位 Qt/OpenCV 工具链。
