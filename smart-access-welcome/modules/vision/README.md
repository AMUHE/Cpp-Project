# Vision 模块

公开接口：`include/saw/vision/face_recognizer.h`。

负责 Haar 级联人脸检测、样本归一化、LBPH 模型训练、标签保存、模型加载和阈值判定。模型与标签写入 Qt 应用数据目录，不写入源码或可执行文件目录。

本模块输出逐帧识别结果，不负责连续匹配、开门或网页推送；这些决策必须交给 `access-control` 和应用编排层。
