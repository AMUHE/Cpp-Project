# 贡献指南

## 开发流程

1. 从最新 `main` 创建带类型前缀的分支。
2. 只修改目标项目和必要的仓库级配置，避免混合无关改动。
3. 按项目 README 完成本地构建或验证。
4. 检查未提交构建产物、运行数据、凭证或个人信息。
5. 推送分支并通过 Pull Request 合并。

## 提交规范

使用 Conventional Commits：

```text
feat: add a user-facing capability
fix: correct a defect
docs: update documentation
test: add or update tests
build: change build configuration
chore: maintain repository files
```

涉及 `smart-access-welcome` 的修改还应遵循其[项目贡献指南](smart-access-welcome/CONTRIBUTING.md)和[安全规范](smart-access-welcome/SECURITY.md)。

## Pull Request

- 说明变更目标、影响范围和验证结果。
- UI 变化附不含个人或敏感数据的截图。
- 新依赖说明版本、用途及许可证。
- 不在同一 Pull Request 中进行无关的批量格式化。

