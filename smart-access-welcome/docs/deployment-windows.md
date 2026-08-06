# Windows 开发与部署环境

## 推荐环境

| 组件 | 推荐版本 | 说明 |
|---|---|---|
| Windows | 10/11 x64 | 开发和首版部署平台 |
| Git | 2.40+ | 版本管理 |
| CMake | 3.21+ | 统一构建入口 |
| Ninja | 1.11+ | 推荐生成器 |
| Qt | 6.5/6.8 LTS x64 | 推荐新环境；迁移期允许 Qt 5.14.2 |
| OpenCV | 4.x x64 + contrib | 必须包含 `face` 模块 |
| 编译器 | 与 Qt 套件一致 | MSVC 与 MinGW 二进制不得混用 |

当前计算机盘点只发现 Git；`cmake`、`qmake`、`ninja` 和 `mingw32-make` 未在当前 PowerShell 的 `PATH` 中。旧原型绑定 Qt 5.14.2、MinGW 32 位和 OpenCV 3.4.5，后续应迁移到统一的 x64 工具链。

## 依赖原则

- Qt、编译器、OpenCV 使用相同架构和兼容 ABI。
- 禁止把 `E:/opencv/...` 等开发机路径写入源码。
- 使用 `CMAKE_PREFIX_PATH` 定位 Qt，使用 `OpenCV_DIR` 定位 `OpenCVConfig.cmake`。
- 第三方运行库进入 GitHub Release 产物，不进入 Git 源码历史。

示例（替换为本机真实路径）：

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.8.0\msvc2022_64"
$env:OpenCV_DIR = "C:\Libraries\opencv\build\x64\vc17\lib"
.\scripts\check-env.ps1
```

## 配置与构建

仓库基线检查暂不要求 Qt/OpenCV：

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

迁入应用后，发布构建启用依赖发现：

```powershell
cmake --preset windows-release
cmake --build --preset windows-release
ctest --preset windows-release
```

旧 OpenCV 3.4 仅限迁移验证，可设置 `SAW_ENABLE_LEGACY_OPENCV=ON`，不得作为正式发布基线。

## 运行目录

复制本地配置：

```powershell
Copy-Item config/config.example.json config/config.json
```

生产环境建议：

```text
C:\Program Files\SmartAccessWelcome\       只读程序与网页资源
C:\ProgramData\SmartAccessWelcome\config  管理员可写配置
C:\ProgramData\SmartAccessWelcome\data    数据库、模型与样本
C:\ProgramData\SmartAccessWelcome\logs    滚动日志
```

服务账户只获得摄像头、监听端口和数据目录的必要权限；普通用户不得读取人脸样本和数据库。

## 发布打包

1. 使用 Release 配置构建并运行全部测试。
2. 使用 Qt 部署工具收集 Qt 插件，按许可证清单收集必要 OpenCV DLL。
3. 包含网页资源、检测器资源和 `config.example.json`。
4. 排除本地配置、数据库、模型、样本、日志和密钥。
5. 生成 ZIP/安装包和 SHA-256 校验值。
6. 在干净 Windows 虚拟机执行安装、启动、摄像头降级和网页访问测试。

## 网络部署与回滚

- 单机欢迎屏保持 `127.0.0.1`，无需开放防火墙端口。
- 局域网显示屏须先完成认证和 TLS，再设置非回环监听地址并限制来源。
- 不得直接暴露到公网。
- 程序按版本部署，数据目录独立；数据库迁移前备份并记录最低兼容版本。
- 回滚时先停用门锁控制，进入拒绝优先模式，验证数据兼容后再切换。

