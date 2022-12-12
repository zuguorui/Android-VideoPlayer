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

    pixelType = get_pixel_type(format);
    pixelLayout = get_pixel_layout(format);

    if (pixelType == PIXEL_TYPE_UNKNOWN || pixelLayout == PIXEL_LAYOUT_UNKNOWN) {
        LOGE(TAG, "unsupported pixel format: %d", format);
        return false;
    }
    if (pixelType == PIXEL_TYPE_RGB) {
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
        shader.use();
        shader.setInt("tex_rgb", 0);
    } else if (pixelType == PIXEL_TYPE_YUV) {
        yuvCompDepth = get_yuv_comp_depth(format);
        if (yuvCompDepth < 0) {
            LOGE(TAG, "get_yuv_comp_depth failed, format = %d", format);
            return false;
        }
        glInternalFormat = GL_RED;
        glDataFormat = GL_RED;
        if (yuvCompDepth <= 8) {
            glDataType = GL_UNSIGNED_BYTE;
        } else if (yuvCompDepth <= 16) {
            glDataType = GL_UNSIGNED_SHORT;
        } else {
            LOGE(TAG, "unsupported yuvCompDepth: %d", yuvCompDepth);
            return false;
        }
        if (!shader.compileShader(vertexShaderCode, yuv2rgbShaderCode)) {
            LOGE(TAG, "format = %d, compile shader failed", format);
            return false;
        }
        shader.use();
        shader.setInt("tex_y", 0);
        shader.setInt("tex_u", 1);
        shader.setInt("tex_v", 2);
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

    deleteYUVPixelBuffer();
    deleteVertices();
    deleteRGBTex();
    deleteYUVTex();

}

void GLESRender::refresh(VideoFrame *videoFrame) {
    if (videoFrame->pixelFormat != format) {
        LOGE(TAG, "frame.format != format");
        return;
    }

    shader.use();

    if (pixelType == PIXEL_TYPE_RGB) {
        createRGBTex(videoFrame);
    } else if (pixelType == PIXEL_TYPE_YUV) {
        createYUVTex(videoFrame);
    } else {
        LOGE(TAG, "unsupported format %d", format);
        return;
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

    glGenTextures(1, &tex_rgb);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, frame->width, frame->height, 0, glDataFormat, glDataType, frame->avFrame->data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_rgb);
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

    if (pix_y_count < yBufSize) {
        if (pix_y) {
            free(pix_y);
            pix_y = nullptr;
        }
        pix_y_count = yBufSize;
        pix_y = (uint8_t *)malloc(pix_y_count);
    }

    if (pix_u_count < uBufSize) {
        if (pix_u) {
            free(pix_u);
            pix_u = nullptr;
        }
        pix_u_count = uBufSize;
        pix_u = (uint8_t *) malloc(pix_u_count);
    }

    if (pix_v_count < vBufSize) {
        if (pix_v) {
            free(pix_v);
            pix_v = nullptr;
        }
        pix_v_count = vBufSize;
        pix_v = (uint8_t *) malloc(pix_v_count);
    }

    read_yuv_pixel(frame->avFrame, format, frame->width, frame->height,
                   pix_y, &y_width, &y_height,
                   pix_u, &u_width, &u_height,
                   pix_v, &v_width, &v_height);


    glGenTextures(1, &tex_y);

    glBindTexture(GL_TEXTURE_2D, tex_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, y_width, y_height, 0, GL_RED, glDataType, pix_y);
    glGenerateMipmap(GL_TEXTURE_2D);


    glGenTextures(1, &tex_u);
    glBindTexture(GL_TEXTURE_2D, tex_u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, u_width, u_height, 0, GL_RED, glDataType, pix_u);
    glGenerateMipmap(GL_TEXTURE_2D);


    glGenTextures(1, &tex_v);
    glBindTexture(GL_TEXTURE_2D, tex_v);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, v_width, v_height, 0, GL_RED, glDataType, pix_v);
    glGenerateMipmap(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_y);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_u);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_v);
}

void GLESRender::deleteYUVTex() {
    glDeleteTextures(1, &tex_y);
    glDeleteTextures(1, &tex_u);
    glDeleteTextures(1, &tex_v);
}


void GLESRender::createYUVPixelBuffer(int64_t yBufSize, int64_t uBufSize, int64_t vBufSize) {
    if (pix_y_count < yBufSize || pix_y == nullptr) {
        if (pix_y) {
            free(pix_y);
            pix_y = nullptr;
        }
        pix_y_count = yBufSize;
        pix_y = (uint8_t *)malloc(pix_y_count);
    }

    if (pix_u_count < uBufSize || pix_u == nullptr) {
        if (pix_u) {
            free(pix_u);
            pix_u = nullptr;
        }
        pix_u_count = uBufSize;
        pix_u = (uint8_t *) malloc(pix_u_count);
    }

    if (pix_v_count < vBufSize || pix_v == nullptr) {
        if (pix_v) {
            free(pix_v);
            pix_v = nullptr;
        }
        pix_v_count = vBufSize;
        pix_v = (uint8_t *) malloc(pix_v_count);
    }
}

void GLESRender::deleteYUVPixelBuffer() {
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
}

void GLESRender::prepareVertices() {

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

