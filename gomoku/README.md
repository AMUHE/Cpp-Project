# 五子棋（Gomoku）

使用 C/C++ 与 EasyX 实现的双人五子棋程序。玩家在控制台输入坐标，棋盘和棋子在 EasyX 图形窗口中显示。

![运行效果](chess.png)

## 功能

- 15×15 标准棋盘。
- 黑白双方交替落子。
- 拦截越界坐标和重复位置。
- 判断横向、纵向及两条对角线的五连胜负。

## 环境要求

- Windows。
- Visual Studio，或其他受 EasyX 支持的编译环境。
- [EasyX](https://easyx.cn/) 图形库。

## 编译和运行

1. 在已安装 EasyX 的 Visual Studio 中创建控制台 C++ 项目。
2. 将 `chess.cpp` 加入项目并完成编译。
3. 运行程序，在控制台输入 `行 列`，行列范围均为 `0` 到 `14`。

示例：

```text
7 7
```

## 文件

```text
chess.cpp  游戏逻辑、输入处理和 EasyX 绘制
chess.png  运行效果截图
```

## 当前限制

- 仅支持本地双人对弈。
- 输入通过控制台完成，图形窗口暂不处理鼠标落子。
- 源码依赖 Windows 和 EasyX，不支持直接在 Linux/macOS 构建。

