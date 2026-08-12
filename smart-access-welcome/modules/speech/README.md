# Speech 模块

公开接口：`include/saw/speech/speech_announcer.h`

模块通过 Qt TextToSpeech 异步播报欢迎和拒绝消息。队列长度有限，新门禁结果到来时会中断过期播报。语音不可用不会阻塞识别，也不会改变门禁决策。

语言、语速、音量和文本模板由配置文件控制，详见 [`docs/speech.md`](../../docs/speech.md)。
