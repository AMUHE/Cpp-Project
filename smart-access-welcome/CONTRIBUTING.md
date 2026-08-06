# 贡献指南

## 分支与提交

- `main` 必须保持可构建、可测试和可发布。
- 功能分支使用 `feature/<topic>`，缺陷分支使用 `fix/<topic>`。
- 提交信息采用 Conventional Commits，例如 `feat: add access event state machine`。
- 通过 Pull Request 合并；不得提交构建产物、个人配置或生物特征数据。

## 本地检查

提交前至少执行：

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
git diff --check
```

涉及协议、配置、数据模型或部署方式的修改，必须同步更新 `docs/` 和示例配置。

## Pull Request 要求

- 描述问题、方案、影响范围和验证结果。
- 新功能包含正常、失败和边界场景测试。
- UI 变更附截图，但截图不得包含真实人脸或个人数据。
- 新依赖说明用途、版本、许可证和维护风险。

