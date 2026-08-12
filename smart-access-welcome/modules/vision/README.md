# Vision 模块

公开接口：`include/saw/vision/face_recognizer.h`

该模块负责 Haar 人脸检测、样本归一化、LBPH 模型训练、标签保存、模型加载和阈值判断。模型与标签写入 Qt 应用数据目录，不写入源码或可执行文件目录。

模块只输出逐帧识别结果。连续匹配和开门决策由 `access-control` 与应用层处理。
