# 基于FFmpeg的Android视频播放器

基于FFmpeg6.0。支持Android的NdkMediaCodec，不必从Native将数据拷贝到Java层。支持av1_mediacodec硬解。



软解支持aom、fdk-aac、mp3lame、x264、x265。相应第三方库的Android构建都有对应的仓库。



代码支持结构支持多平台，平台相关解码器定义在`platform.h`。音视频输入输出接口位于`Factory`中。
