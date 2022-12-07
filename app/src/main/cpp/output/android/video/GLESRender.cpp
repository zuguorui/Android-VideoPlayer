//
// Created by 祖国瑞 on 2022/12/6.
//

#include "GLESRender.h"
#include "Log.h"

#define TAG "GLESRender"

using namespace std;

const static AVPixelFormat SUPPORTED_RGB_FORMAT[] = {
        AVPixelFormat::AV_PIX_FMT_RGB24
};


const static AVPixelFormat SUPPORTED_YUV_FORMAT[] =  {
        AVPixelFormat::AV_PIX_FMT_YUV420P,
        AVPixelFormat::AV_PIX_FMT_YUV422P,
        AVPixelFormat::AV_PIX_FMT_YUV444P,
        AVPixelFormat::AV_PIX_FMT_NV12,
        AVPixelFormat::AV_PIX_FMT_NV21
};

const static float vertices[] = {
        // vertex pos         // tex coords
        -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, // view left-bottom to tex left-top
        1.0f, -1.0f,  0.0f,  1.0f, 1.0f, // view right-bottom to tex right-top
        1.0f,  1.0f,  0.0f,  1.0f, 0.0f, // view right-top to tex right-bottom
        -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, // view left-top to tex left-bottom
};

unsigned int indices[] = {
        0, 3, 2,
        2, 1, 0
};

GLESRender::GLESRender() {

}

GLESRender::~GLESRender() {
    release();
}

bool GLESRender::create(AVPixelFormat format, AVColorSpace colorSpace, bool isHDR) {

    if (rgbSupport(format)) {
        if (!shader.compileShader(vertexShaderCode, rgbShaderCode)) {
            LOGE(TAG, "format = RGB24, compile shader failed");
            return false;
        }
        shader.use();
        shader.setInt("tex_rgb", 0);
        formatType = FormatType::NONE;
    } else if (yuvSupport(format)) {
        if (!shader.compileShader(vertexShaderCode, yuv2rgbShaderCode)) {
            LOGE(TAG, "format = %d, compile shader failed", format);
            return false;
        }
        shader.use();
        shader.setInt("tex_y", 0);
        shader.setInt("tex_u", 1);
        shader.setInt("tex_v", 2);
        formatType = FormatType::NONE;
    } else {
        LOGE(TAG, "unsupported pixel format: %d", format);
        return false;
    }

    this->format = format;
    this->colorSpace = colorSpace;
    this->isHDR = isHDR;


    if (!shader.isReady()) {
        LOGE(TAG, "shader is not ready");
        return false;
    }
    return true;
}

bool GLESRender::setWindow(ANativeWindow *window) {
    if (!eglWindow.create(window)) {
        LOGE(TAG, "create eglWindow failed");
        return false;
    }
    if (!eglWindow.isReady()) {
        LOGE(TAG, "eglWindow not ready");
        return false;
    }
    prepareVertices();
}

void GLESRender::setSize(int width, int height) {
    glViewport(0, 0, width, height);
}

void GLESRender::release() {
    eglWindow.release();
    shader.release();

    if (pix_rgb) {
        free(pix_rgb);
        pix_rgb = nullptr;
        pix_rgb_count = 0;
    }

    if (pix_y) {
        free(pix_y);
        pix_y = nullptr;
        pix_y_count = 0;
    }

    if (pix_u) {
        free(pix_u);
        pix_u = nullptr;
        pix_u_count = 0;
    }

    if (pix_v) {
        free(pix_v);
        pix_v = nullptr;
        pix_v_count = 0;
    }

    if (VAO > 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO > 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO > 0) {
        glDeleteBuffers(1, &EBO);
    }

}

void GLESRender::refresh(VideoFrame *videoFrame) {
    if (videoFrame->pixelFormat != format) {
        LOGE(TAG, "frame.format != format");
        return;
    }

    if (formatType == FormatType::RGB) {
        createRGBTex(videoFrame);
    } else if (formatType == FormatType::YUV) {
        createYUVTex(videoFrame);
    } else {
        LOGE(TAG, "unsupported format %d", format);
        return;
    }


    shader.use();

    if (formatType == FormatType::RGB) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_rgb);
    } else if (formatType == FormatType::YUV) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_y);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_u);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tex_v);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    eglWindow.swapBuffer();

}

void GLESRender::createRGBTex(VideoFrame *frame) {
    deleteRGBTex();

}

