# Smart Access Welcome

基于 Qt、OpenCV 和本地 Web 服务的智能门禁终端。项目目标是完成“摄像头采集 → 人脸识别 → 门禁策略 → 模拟或真实开门 → 网页实时欢迎 → 审计记录”的完整流程。

> 当前状态：仓库与工程基线已建立，业务模块将从现有 `Face` 原型分阶段迁移。当前版本不能用于高安全等级或无人值守的真实门禁环境。

## 首版范围

- 单摄、双摄和拼接式双目摄像头接入。
- 人员录入、本地模型训练和身份识别。
- 门禁允许/拒绝、重复识别冷却和模拟开门。
- SQLite 人员与门禁事件存储。
- HTTP 静态欢迎页及 WebSocket 实时事件推送。
- Windows 构建、测试与 GitHub Actions。

## 技术基线

- C++17
- CMake 3.21+
- Qt 5.14.2 或 Qt 6.x（Core、Widgets、Network、Sql、Test）
- OpenCV 4.x（迁移期兼容现有 OpenCV 3.4.5 + contrib/face）
- Windows 10/11 x64

不再使用源码中的本机绝对依赖路径。Qt 和 OpenCV 均通过 CMake 缓存变量或环境变量定位。

## 快速开始

1. 安装 CMake、Git、Qt 和带 `face` 模块的 OpenCV。
2. 复制配置：

   ```powershell
   Copy-Item config/config.example.json config/config.json
   ```

3. 检查环境：

   ```powershell
   .\scripts\check-env.ps1
   ```

4. 配置和构建：

   ```powershell
   cmake --preset windows-debug
   cmake --build --preset windows-debug
   ctest --preset windows-debug
   ```

当前仓库尚未迁入可执行程序，现阶段 CMake 只负责验证工程和依赖配置。完整说明见 [Windows 部署文档](docs/deployment-windows.md)。

## 文档

- [产品与交付范围](docs/product-scope.md)
- [系统架构](docs/architecture.md)
- [HTTP 与 WebSocket API](docs/api.md)
- [Windows 部署环境](docs/deployment-windows.md)
- [安全与隐私](docs/security-and-privacy.md)
- [开发路线图](docs/roadmap.md)
- [贡献指南](CONTRIBUTING.md)
- [安全问题报告](SECURITY.md)

## 数据安全

`data/`、`runtime/`、数据库、模型、人脸样本、日志和本地配置均被 Git 忽略。严禁向仓库、Issue 或测试夹具提交真实人员的人脸数据、凭证或门禁记录。

## 许可证

代码按 [MIT License](LICENSE) 发布。第三方依赖及模型文件分别遵循其自身许可证。
