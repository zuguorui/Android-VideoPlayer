//
// Created by zu on 2024/8/6.
//

#include "YUV420P_RenderLogic.h"
#include "Log.h"

#define TAG "YUV420P_RenderLogic"

YUV420P_RenderLogic::YUV420P_RenderLogic() {
    if (!renderProgram.compile(texture_vert_code, render_yuv420p_frag_code)) {
        LOGE(TAG, "compile program failed");
    }
    prepareObjects(frameWidth, frameHeight);
}

YUV420P_RenderLogic::~YUV420P_RenderLogic() noexcept {
    deleteObjects();
    renderProgram.release();
}

const char *YUV420P_RenderLogic::name() {
    return "YUV420P_RenderLogic";
}

void YUV420P_RenderLogic::render(VideoFrame *frame) {
    if (!renderProgram.isReady()) {
        return;
    }

    int pixelCount = frameWidth * frameHeight;

    int64_t yCount = pixelCount;
    int64_t uCount = pixelCount / 4;
    int64_t vCount = uCount;

    // 将数据拷贝到PBO里
    // y
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    uint8_t *yPtr = (uint8_t *)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, yCount, GL_MAP_WRITE_BIT);
    if (yPtr != nullptr) {
        memcpy(yPtr, frame->avFrame->data[0], yCount);
    } else {
        LOGE(TAG, "mapping yBuffer failed");
    }
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // u
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uBuffer);
    uint8_t *uPtr = (uint8_t *)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, uCount, GL_MAP_WRITE_BIT);
    if (uPtr != nullptr) {
        memcpy(uPtr, frame->avFrame->data[1], uCount);
    } else {
        LOGE(TAG, "mapping uBuffer failed");
    }
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // v
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vBuffer);
    uint8_t *vPtr = (uint8_t *)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, vCount, GL_MAP_WRITE_BIT);
    if (vPtr != nullptr) {
        memcpy(vPtr, frame->avFrame->data[2], vCount);
    } else {
        LOGE(TAG, "mapping vBuffer failed");
    }
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // 更新纹理
    // y
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    glBindTexture(GL_TEXTURE_2D, yTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_FORMAT, GL_DATA_TYPE,
                    nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // u
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uBuffer);
    glBindTexture(GL_TEXTURE_2D, uTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth / 2, frameHeight / 2, GL_FORMAT, GL_DATA_TYPE,
                    nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // v
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vBuffer);
    glBindTexture(GL_TEXTURE_2D, vTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth / 2, frameHeight / 2, GL_FORMAT, GL_DATA_TYPE,
                    nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


    renderProgram.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, yTex);
    renderProgram.setInt("y_tex", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, uTex);
    renderProgram.setInt("u_tex", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, vTex);
    renderProgram.setInt("v_tex", 2);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //LOGD(TAG, "draw");
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void YUV420P_RenderLogic::deleteObjects() {
    if (yBuffer) {
        glDeleteBuffers(1, &yBuffer);
        yBuffer = 0;
    }
    if (uBuffer) {
        glDeleteBuffers(1, &uBuffer);
        uBuffer = 0;
    }
    if (vBuffer) {
        glDeleteBuffers(1, &vBuffer);
        vBuffer = 0;
    }

    if (yTex) {
        glDeleteTextures(1, &yTex);
        yTex = 0;
    }
    if (uTex) {
        glDeleteTextures(1, &uTex);
        uTex = 0;
    }
    if (vTex) {
        glDeleteTextures(1, &vTex);
        vTex = 0;
    }
}

void YUV420P_RenderLogic::prepareObjects(int width, int height) {

    deleteObjects();

    int64_t pixelCount = width * height;
    int64_t yCount = pixelCount;
    int64_t uCount = pixelCount / 4;
    int64_t vCount = uCount;

    // 创建buffer
    glGenBuffers(1, &yBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, yCount, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glGenBuffers(1, &uBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, uCount, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glGenBuffers(1, &vBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, vCount, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // 创建texture并绑定到pbo
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    glGenTextures(1, &yTex);
    glBindTexture(GL_TEXTURE_2D, yTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTERNAL_FORMAT, width, height, 0, GL_FORMAT, GL_DATA_TYPE,
                 nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uBuffer);
    glGenTextures(1, &uTex);
    glBindTexture(GL_TEXTURE_2D, uTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTERNAL_FORMAT, width / 2, height / 2, 0, GL_FORMAT, GL_DATA_TYPE,
                 nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vBuffer);
    glGenTextures(1, &vTex);
    glBindTexture(GL_TEXTURE_2D, vTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTERNAL_FORMAT, width / 2, height / 2, 0, GL_FORMAT, GL_DATA_TYPE,
                 nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

bool YUV420P_RenderLogic::isReady() {
    return renderProgram.isReady();
}

void YUV420P_RenderLogic::onFrameSizeChanged(int width, int height) {
    prepareObjects(width, height);
}
