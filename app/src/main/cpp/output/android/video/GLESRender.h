//
// Created by 祖国瑞 on 2022/12/6.
//

#ifndef ANDROID_VIDEOPLAYER_GLESRENDER_H
#define ANDROID_VIDEOPLAYER_GLESRENDER_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "android/native_window.h"
#include "GLES3/gl3.h"
#include "GLES3/gl3platform.h"
#include "Shader.h"
#include "EGLWindow.h"
#include "ShaderScripts.h"

#include "VideoFrame.h"
extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavutil/avutil.h"
}

class GLESRender {
public:
    GLESRender();
    ~GLESRender();

    bool create(AVPixelFormat format, AVColorSpace colorSpace, bool isHDR);

    bool setWindow(ANativeWindow *window);

    void setSize(int width, int height);

    void refresh(VideoFrame *videoFrame);

    void release();

    enum FormatType {
        NONE, RGB, YUV
    };

private:
    AVPixelFormat format = AVPixelFormat::AV_PIX_FMT_NONE;
    FormatType formatType = FormatType::NONE;
    AVColorSpace colorSpace = AVColorSpace::AVCOL_SPC_NB;
    bool isHDR = false;

    EGLWindow eglWindow;
    Shader shader;

    uint8_t *pix_y = nullptr;
    int64_t pix_y_count = 0;
    uint8_t *pix_u = nullptr;
    int64_t pix_u_count = 0;
    uint8_t *pix_v = nullptr;
    int64_t pix_v_count = 0;

    uint8_t *pix_rgb = nullptr;
    int64_t pix_rgb_count = 0;

    GLuint tex_y = 0;
    GLuint tex_u = 0;
    GLuint tex_v = 0;

    GLuint tex_rgb = 0;

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    void createRGBPixelBuffer(int64_t pixelCount);

    void deleteRGBPixelBuffer();

    void createYUVPixelBuffer(int64_t pixelCount);

    void deleteYUVPixelBuffer();


    void createYUVTex(VideoFrame *frame);

    void deleteYUVTex();

    void createRGBTex(VideoFrame *frame);

    void deleteRGBTex();

    bool yuvSupport(AVPixelFormat format);

    bool rgbSupport(AVPixelFormat format);

    void prepareVertices();

    void deleteVertices();


};


#endif //ANDROID_VIDEOPLAYER_GLESRENDER_H
