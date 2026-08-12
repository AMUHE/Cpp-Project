# 语音播报设计

## 行为

- 放行：按 `speech.grantedTemplate` 播报，例如“欢迎，张三”。
- 拒绝：按 `speech.deniedText` 播报，不透露相似身份或识别距离。
- 新的门禁决策会中断旧播报，避免迟到语音与屏幕状态不一致。
- 队列有固定上限，异常事件风暴不会无限占用内存。

## 线程与降级

Qt TextToSpeech 引擎在专用工作线程初始化和运行。Windows SAPI、声卡驱动或服务响应缓慢时，UI、识别、HTTP 和门锁策略仍可继续。健康接口中的 `speech` 状态为：

- `ready`：引擎可用；
- `unavailable`：已启用但初始化失败或后端错误；
- `disabled`：配置明确关闭。

语音是辅助输出，不参与授权决策。语音失败不得改变放行/拒绝结果，也不得阻止审计写入。

## Windows 部署

使用 `windeployqt` 部署 `Qt5TextToSpeech.dll` 和 `texttospeech/qtexttospeech_sapi.dll`。目标系统需启用 Windows Speech API，并安装匹配 `speech.locale` 的语音包。没有中文语音时会尝试同语言区域；仍不可用则降级。

## 隐私与可访问性

公共区域播报姓名可能造成隐私泄漏。生产部署应经隐私评估，可将模板改为“验证通过，请通行”而不读姓名。音量和播报时段应符合现场规范，并同时保留视觉反馈，不能把语音作为唯一状态通道。
