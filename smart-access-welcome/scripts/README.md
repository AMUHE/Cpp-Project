# Windows 脚本

- `check-env.ps1`：检查 CMake、Qt 和 OpenCV 环境；
- `build-windows-qmake.ps1`：使用 Qt 5.14.2/MinGW 旧工具链构建，可选部署主程序；
- `run-tests.ps1`：构建并运行 qmake 测试工程。

通过参数传入 Qt、MinGW 和 OpenCV 的安装目录，不要把本机绝对路径写入仓库。生产构建应使用受支持的 64 位 Qt/OpenCV 工具链。
