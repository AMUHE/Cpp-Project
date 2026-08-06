# 仓库维护指南

## 目录结构

```text
Cpp-Project/
├─ .github/workflows/       仓库级持续集成
├─ docs/                    仓库级维护文档
├─ gomoku/                  EasyX 五子棋
├─ student-system/          Win32 学生管理系统
├─ smart-access-welcome/    Qt/OpenCV 智能门禁系统
├─ .editorconfig            编辑器基础格式
├─ .gitattributes           文本、行尾和二进制规则
├─ .gitignore               全仓库忽略规则
└─ README.md                项目索引
```

## 组织原则

1. 每个可独立运行的项目占用一个顶层目录，并维护自己的 README。
2. 根目录只放仓库级配置、索引和跨项目文档。
3. 项目构建产物应写入项目内 `build/` 或外部构建目录，不与源码混放。
4. 截图和静态资源保留在所属项目内，避免跨项目隐式依赖。
5. GitHub Actions 位于根 `.github/workflows/`，通过 `paths` 限定触发范围。

## 命名规则

- 目录和文件优先使用小写英文及连字符。
- 分支使用 `feature/`、`fix/`、`docs/` 或 `chore/` 前缀。
- 提交信息采用 Conventional Commits，例如 `docs: normalize project readmes`。

## 新增项目检查表

- [ ] 项目使用独立目录且命名明确。
- [ ] README 包含用途、依赖、构建、运行、文件结构和限制。
- [ ] 本地构建及运行数据已加入 `.gitignore`。
- [ ] 不包含凭证、个人数据、IDE 用户配置或二进制构建产物。
- [ ] 如需 CI，在根 `.github/workflows/` 添加按路径触发的工作流。

