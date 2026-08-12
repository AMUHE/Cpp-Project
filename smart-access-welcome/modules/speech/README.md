# Speech 模块

公开接口：`include/saw/speech/speech_announcer.h`。

通过 Qt TextToSpeech 异步播报欢迎和拒绝文案，限制队列长度，并在新门禁决定到来时中断过期播报。语音不可用属于可降级故障，不应阻塞识别线程或绕过门禁策略。

语言、语速、音量和模板由配置文件控制，详细行为见 `docs/speech.md`。
