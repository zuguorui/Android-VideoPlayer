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
#include "pixel_loader.h"
#include "SizeMode.h"

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

    void setScreenSize(int width, int height);

    void refresh(VideoFrame *videoFrame);

    void release();

    bool isReady();

    void setSizeMode(SizeMode mode);

private:
    AVPixelFormat format = AVPixelFormat::AV_PIX_FMT_NONE;
    int pixelType = PIXEL_TYPE_UNKNOWN;
    int yuvCompDepth = 0;
    int pixelLayout = PIXEL_LAYOUT_UNKNOWN;
    bool glSupportFormat = false;
    AVColorSpace colorSpace = AVColorSpace::AVCOL_SPC_NB;
    bool isHDR = false;

    int screenWidth = 0;
    int screenHeight = 0;

    SizeMode sizeMode = SizeMode::FIT;

    int frameWidth = 0;
    int frameHeight = 0;

    float vertices[20];

    GLuint glInternalFormat = GL_RGB;
    GLuint glDataType = GL_UNSIGNED_BYTE;
    GLuint glDataFormat = GL_RGB;

    EGLWindow eglWindow;
    Shader shader;

    uint8_t *pix_y = nullptr;
    int64_t pix_y_count = -1;
    uint8_t *pix_u = nullptr;
    int64_t pix_u_count = -1;
    uint8_t *pix_v = nullptr;
    int64_t pix_v_count = -1;

    GLuint tex_y = 0;
    GLuint tex_u = 0;
    GLuint tex_v = 0;

    GLuint tex_rgb = 0;

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    void updateVertices();

    void createYUVPixelBuffer(int64_t yBufSize, int64_t uBufSize, int64_t vBufSize);

    void deleteYUVPixelBuffer();


    void createYUVTex(VideoFrame *frame);

    void deleteYUVTex();

    void createRGBTex(VideoFrame *frame);

    void deleteRGBTex();

    void prepareVertices();

    void deleteVertices();



};


#endif //ANDROID_VIDEOPLAYER_GLESRENDER_H
