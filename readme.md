# 基于FFmpeg的Android视频播放器

基于FFmpeg6.0。支持Android的NdkMediaCodec，不比走jni调用。支持av1_mediacodec硬解。

软解支持aom、fdk-aac、mp3lame、x264、x265。相应第三方库的Android构建都有对应的仓库。

代码支持结构支持多平台，平台相关解码器定义在`platform.h`。音视频输入输出接口位于`Factory`中。


## 关键模块

### Player

使用FFmpeg进行解码及控制播放的逻辑。

### cpp/output

与操作系统相关的音视频输出。Android端的视频渲染使用OpenGL ES。

### FFmpegCodec

对FFmpeg的codec操作的包装。集成了操作硬件codec的能力。

### FFmpegMuxer

使用FFmpeg进行mux的基础逻辑。不包括io的部分。

### FFmpegUrlMuxer

在FFmpegMuxer的基础上，自定义了输出，可以选择将mux输出推流或者保存为文件。基于FFmpeg的AVIOContext实现。输入则依赖于Android上层的音视频编码器。

### RtmpPusher

在FFmpegMuxer的基础上，自定义AVIOContext，然后将FFmpeg输出手动通过librtmp推流。相比FFmpegUrlMuxer，这部分更多是手动处理FFmpeg与RTMP的交互。

### java/encoder

android端的MediaCodec编码。


## 更新日志

- 修复退出播放器时阻塞的问题。
- 修复了内存泄漏。
- 修复单独播放音频或视频时占用内存过大的问题。
- 修复了播放完毕后崩溃的问题。
- 优化了播放状态回调。
- 修复了解码器在第二次打开之后不正常的问题。
- 修复了seek时卡顿的问题。
- 修复了仅有视频轨时播放速度不正常的问题。
- 修复了文件播放完毕后的状态异常问题。
- 新增了HandlerThread方便运行异步任务。
- 修改GL的渲染逻辑，对于YUV420P、NV21和NV12这几种常见格式，将直接送入着色器而不需要预处理。提升性能表现。
- 升级到OpenGL ES 3.1，可以支持计算着色器以应对可能出现的更复杂的格式。
- 修复某些文件在metadata里指定旋转方向不生效的问题。
- 增加FFmpegUrlMuxer，并通过自定义AVIOContext，可选录像保存为文件或者推流到rtmp。
- 增加RTMP相关库。增加RtmpPusher，手动处理AVIOContext和RTMP交互。