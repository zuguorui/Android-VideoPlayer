//
// Created by 祖国瑞 on 2022/12/6.
//

#ifndef ANDROID_VIDEOPLAYER_GLESRENDER_H
#define ANDROID_VIDEOPLAYER_GLESRENDER_H

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <iostream>
#include "android/native_window.h"
#include "GLES3/gl31.h"
#include "GLES3/gl3platform.h"
#include "RenderProgram.h"
#include "EGLWindow.h"
#include "ShaderScripts.h"
#include "pixel_loader.h"
#include "SizeMode.h"
#include "render_logic/YUV420P_RenderLogic.h"
#include "render_logic/NV21_RenderLogic.h"
#include "render_logic/NV12_RenderLogic.h"


#include "VideoFrame.h"
extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavutil/avutil.h"
}

class GLESRender {
public:
    GLESRender();
    ~GLESRender();

    bool setFormat(AVPixelFormat format, AVColorSpace colorSpace, bool isHDR);

    bool setWindow(ANativeWindow *window);

    void setScreenSize(int width, int height);

    void refresh(VideoFrame *videoFrame);

    void release();

    bool isReady();

    bool isEGLReady();

    void setSizeMode(SizeMode mode);

private:
    AVPixelFormat format = AVPixelFormat::AV_PIX_FMT_NONE;
    PixelLayout pixelLayout = PixelLayout::None;
    PixelType pixelType = PixelType::None;

    int screenWidth = 0;
    int screenHeight = 0;

    SizeMode sizeMode = SizeMode::FIT;

    int frameWidth = 0;
    int frameHeight = 0;

    int orientation = 0;

    EGLWindow eglWindow;

    IRenderLogic *renderLogic = nullptr;

};




#endif //ANDROID_VIDEOPLAYER_GLESRENDER_H
