//
// Created by 祖国瑞 on 2022/12/6.
//

#include "GLESRender.h"
#include "Log.h"
#include "Util.h"

#define TAG "GLESRender"

using namespace std;

void dumpData(const char *name, uint8_t *data, int64_t size) {
    char fullPath[50];
    sprintf(fullPath, "/data/data/com.zu.android_videoplayer/cache/%s", name);
    FILE *f = fopen(fullPath, "wb");
    fwrite(data, 1, size, f);
    fflush(f);
    fclose(f);
}



const static unsigned int indices[] = {
        0, 3, 2,
        2, 1, 0
};

GLESRender::GLESRender() {

}

GLESRender::~GLESRender() {
    release();
}

bool GLESRender::setFormat(AVPixelFormat format, AVColorSpace colorSpace, bool isHDR) {

    LOGD(TAG, "setFormat");

    if (!eglWindow.isReady()) {
        LOGE(TAG, "eglWindow is not ready");
        return false;
    }

    pixelType = get_pixel_type(format);
    pixelLayout = get_pixel_layout(format);

    if (pixelType == PixelType::None || pixelLayout == PixelLayout::None) {
        LOGE(TAG, "unsupported pixel format: %d", format);
        return false;
    }
    if (pixelType == PixelType::RGB) {
        switch (format) {
            case AV_PIX_FMT_RGB24:
                glDataType = GL_UNSIGNED_BYTE;
                glInternalFormat = GL_RGB;
                glDataFormat = GL_UNSIGNED_BYTE;
                glSupportFormat = true;
                break;
            case AV_PIX_FMT_RGB565LE:
                glDataType = GL_UNSIGNED_SHORT_5_6_5;
                glInternalFormat = GL_RGB;
                glDataFormat = GL_UNSIGNED_SHORT_5_6_5;
                glSupportFormat = true;
                break;
            case AV_PIX_FMT_RGB444LE:
                glDataType = GL_UNSIGNED_SHORT_4_4_4_4;
                glInternalFormat = GL_RGB;
                glDataFormat = GL_UNSIGNED_SHORT_4_4_4_4;
                glSupportFormat = true;
                break;
            default:
                LOGE(TAG, "unsupported RGB format: %d", format);
                return false;
        }
        if (!shader.compileShader(vertexShaderCode, rgbShaderCode)) {
            LOGE(TAG, "format = RGB24, compile shader failed");
            return false;
        }

    } else if (pixelType == PixelType::YUV) {
        yuvCompDepth = get_yuv_comp_depth(format);
        if (yuvCompDepth < 0) {
            LOGE(TAG, "get_yuv_comp_depth failed, format = %d", format);
            return false;
        }
        const char *fragmentCode;
        if (yuvCompDepth <= 8) {
            glDataType = GL_UNSIGNED_BYTE;
            glInternalFormat = GL_LUMINANCE;
            glDataFormat = GL_LUMINANCE;
            fragmentCode = yuv2rgbShaderCode;
        } else if (yuvCompDepth <= 16) {
            glDataType = GL_FLOAT;
            glInternalFormat = GL_R32F;
            glDataFormat = GL_RED;
            fragmentCode = yuv2rgbShaderCode;
        } else {
            LOGE(TAG, "unsupported yuvCompDepth: %d", yuvCompDepth);
            return false;
        }
        if (!shader.compileShader(vertexShaderCode, fragmentCode)) {
            LOGE(TAG, "format = %d, compile shader failed", format);
            return false;
        }
        eglWindow.makeCurrent();
        LOGD(TAG, "yuvCompDepth = %d", yuvCompDepth);
    } else {
        LOGE(TAG, "unsupported pixel format: %d", format);
        return false;
    }

    this->format = format;
    this->colorSpace = colorSpace;
    this->isHDR = isHDR;

    LOGD(TAG, "setFormat: format = %d, pixType = %d, glDataType = 0x%x", format, pixelType, glDataType);

    if (!shader.isReady()) {
        LOGE(TAG, "shader is not ready");
        return false;
    }
    return true;
}

bool GLESRender::setWindow(ANativeWindow *window) {
    LOGD(TAG, "setWindow");
    if (!eglWindow.create(window)) {
        LOGE(TAG, "setFormat eglWindow failed");
        return false;
    }
    if (!eglWindow.isReady()) {
        LOGE(TAG, "eglWindow not ready");
        return false;
    }
    prepareVertices();
    return true;
}

void GLESRender::setScreenSize(int width, int height) {
    bool needUpdateVertices = false;
    if (screenWidth != width || screenHeight != height) {
        needUpdateVertices = true;
    }
    screenWidth = width;
    screenHeight = height;
    if (eglWindow.isReady()) {
        glViewport(0, 0, width, height);
    }
    if (needUpdateVertices) {
        updateVertices();
    }

}

void GLESRender::release() {

    shader.release();

    deleteYUVPixelBuffer();
    deleteVertices();
    deleteRGBTex();
    deleteYUVTex();

    eglWindow.release();

}

void GLESRender::refresh(VideoFrame *videoFrame) {
    if (videoFrame->pixelFormat != format) {
        LOGE(TAG, "frame.format != format");
        return;
    }
    bool needUpdateVertices = false;
    if (videoFrame->width != frameWidth || videoFrame->height != frameHeight) {
        needUpdateVertices = true;
    }
    frameWidth = videoFrame->width;
    frameHeight = videoFrame->height;

    if (needUpdateVertices) {
        updateVertices();
    }

    eglWindow.makeCurrent();
    shader.use();



    if (pixelType == PixelType::RGB) {
        createRGBTex(videoFrame);
    } else if (pixelType == PixelType::YUV) {
        createYUVTex(videoFrame);
    } else {
        LOGE(TAG, "unsupported format %d", format);
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    eglWindow.swapBuffer();

}

void GLESRender::createRGBTex(VideoFrame *frame) {

    deleteRGBTex();

    glGenTextures(1, &tex_rgb);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, frame->width, frame->height, 0, glDataFormat, glDataType, frame->avFrame->data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_rgb);
    shader.setInt("tex_rgb", 0);
}

void GLESRender::deleteRGBTex() {
    glDeleteTextures(1, &tex_rgb);
}

void GLESRender::createYUVTex(VideoFrame *frame) {
    deleteYUVTex();

    int y_width, y_height;
    int u_width, u_height;
    int v_width, v_height;

    int64_t yBufSize, uBufSize, vBufSize;

    if (!compute_yuv_buffer_size(format, frame->width, frame->height, &yBufSize, &uBufSize, &vBufSize)) {
        LOGE(TAG, "can't compute yuv buffer size, format = %d", format);
        return;
    }

    createYUVPixelBuffer(yBufSize, uBufSize, vBufSize);
    //int64_t startTime = getSystemClockCurrentMilliseconds();
    bool readSuccess = read_yuv_pixel(frame->avFrame, format, frame->width, frame->height,
                   pix_y, &y_width, &y_height,
                   pix_u, &u_width, &u_height,
                   pix_v, &v_width, &v_height);

    //LOGD(TAG, "createYUVTex: read pixel cost %ld ms", getSystemClockCurrentMilliseconds() - startTime);

    if (!readSuccess) {
        LOGE(TAG, "failed to read yuv pixel");
        return;
    }

//    dumpData("y", pix_y, yBufSize);
//    dumpData("u", pix_u, uBufSize);
//    dumpData("v", pix_v, vBufSize);

    GLenum glError;

    glGenTextures(1, &tex_y);
    glBindTexture(GL_TEXTURE_2D, tex_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, y_width, y_height, 0, glDataFormat, glDataType, pix_y);
    glGenerateMipmap(GL_TEXTURE_2D);
    glError = glGetError();
    if (glError != GL_NO_ERROR) {
        LOGE(TAG, "y, glError: 0x%x", glError);
    }




    glGenTextures(1, &tex_u);
    glBindTexture(GL_TEXTURE_2D, tex_u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, u_width, u_height, 0, glDataFormat, glDataType, pix_u);
    glGenerateMipmap(GL_TEXTURE_2D);

    glError = glGetError();
    if (glError != GL_NO_ERROR) {
        LOGE(TAG, "u, glError: 0x%x", glError);
    }


    glGenTextures(1, &tex_v);
    glBindTexture(GL_TEXTURE_2D, tex_v);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, v_width, v_height, 0, glDataFormat, glDataType, pix_v);
    glGenerateMipmap(GL_TEXTURE_2D);

    glError = glGetError();
    if (glError != GL_NO_ERROR) {
        LOGE(TAG, "v, glError: 0x%x", glError);
    }

    shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_y);
    shader.setInt("tex_y", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_u);
    shader.setInt("tex_u", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_v);
    shader.setInt("tex_v", 2);
}

void GLESRender::deleteYUVTex() {
    glDeleteTextures(1, &tex_y);
    glDeleteTextures(1, &tex_u);
    glDeleteTextures(1, &tex_v);
}


void GLESRender::createYUVPixelBuffer(int64_t yBufSize, int64_t uBufSize, int64_t vBufSize) {
    if (pix_y_size < yBufSize || pix_y == nullptr) {
        if (pix_y) {
            free(pix_y);
            pix_y = nullptr;
        }
        pix_y_size = yBufSize;
        pix_y = (uint8_t *)malloc(pix_y_size);
    }

    if (pix_u_size < uBufSize || pix_u == nullptr) {
        if (pix_u) {
            free(pix_u);
            pix_u = nullptr;
        }
        pix_u_size = uBufSize;
        pix_u = (uint8_t *) malloc(pix_u_size);
    }

    if (pix_v_size < vBufSize || pix_v == nullptr) {
        if (pix_v) {
            free(pix_v);
            pix_v = nullptr;
        }
        pix_v_size = vBufSize;
        pix_v = (uint8_t *) malloc(pix_v_size);
    }
}

void GLESRender::deleteYUVPixelBuffer() {
    if (pix_y) {
        free(pix_y);
        pix_y = nullptr;
        pix_y_size = 0;
    }

    if (pix_u) {
        free(pix_u);
        pix_u = nullptr;
        pix_u_size = 0;
    }

    if (pix_v) {
        free(pix_v);
        pix_v = nullptr;
        pix_v_size = 0;
    }
}

void GLESRender::prepareVertices() {

    deleteVertices();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GLESRender::deleteVertices() {
    glDeleteBuffers(1, &EBO);
    EBO = 0;
    glDeleteBuffers(1, &VBO);
    VBO = 0;
    glDeleteVertexArrays(1, &VAO);
    VAO = 0;
}

bool GLESRender::isReady() {
    return eglWindow.isReady() && shader.isReady();
}

bool GLESRender::isEGLReady() {
    return eglWindow.isReady();
}

void GLESRender::setSizeMode(SizeMode mode) {
    bool needUpdateVertices = false;
    if (sizeMode != mode) {
        needUpdateVertices = true;
    }
    sizeMode = mode;
    if (needUpdateVertices) {
        updateVertices();
    }
}

void GLESRender::updateVertices() {
    deleteVertices();
    if (screenWidth == 0 || screenHeight == 0) {
        LOGE(TAG, "screen size not set");
        return;
    }
    if (frameWidth == 0 || frameHeight == 0) {
        LOGE(TAG, "frame size not confirmed");
        return;
    }
    LOGD(TAG, "updateVertices, screenSize = %dx%d, frameSize = %dx%d", screenWidth, screenHeight, frameWidth, frameHeight);

    float screenLeft, screenRight, screenTop, screenBottom;
    float frameLeft, frameRight, frameTop, frameBottom;

    frameLeft = 0;
    frameRight = 1;
    frameTop = 1;
    frameBottom = 0;

    if (sizeMode == SizeMode::FULL) {
        screenLeft = -1;
        screenRight = 1;
        screenTop = 1;
        screenBottom = -1;
    } else if (sizeMode == SizeMode::FIT) {
        float frameW2H = frameWidth * 1.0f / frameHeight;
        float screenW2H = screenWidth * 1.0f / screenHeight;

        if (frameW2H >= screenW2H) {
            int scaledScreenHeight = (int)(screenWidth * frameHeight * 1.0f / frameWidth);
            screenLeft = -1;
            screenRight = 1;
            screenTop = scaledScreenHeight * 1.0f / screenHeight;
            screenBottom = -screenTop;
        } else {
            int scaledScreenWidth = (int)(screenHeight * frameWidth * 1.0f / frameHeight);
            screenRight = scaledScreenWidth * 1.0f / screenWidth;
            screenLeft = -screenRight;

            screenTop = 1;
            screenBottom = -1;
        }

        float tmpVert[20] = {
            screenLeft, screenBottom, 0, frameLeft, frameTop,
            screenRight, screenBottom, 0, frameRight, frameTop,
            screenRight, screenTop, 0, frameRight, frameBottom,
            screenLeft, screenTop, 0, frameLeft, frameBottom,
        };

        memcpy(vertices, tmpVert, 20 * sizeof(float));

        if (eglWindow.isReady()) {
            prepareVertices();
        }
    }


}

