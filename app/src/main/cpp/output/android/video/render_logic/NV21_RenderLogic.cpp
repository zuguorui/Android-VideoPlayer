//
// Created by zu on 2024/8/6.
//

#include "NV21_RenderLogic.h"
#include "Log.h"

#define TAG "NV21_RenderLogic"

NV21_RenderLogic::NV21_RenderLogic() {
    if (!renderProgram.compile(texture_vert_code, render_nv21_frag_code)) {
        LOGE(TAG, "compile program failed");
    }
    prepareObjects(frameWidth, frameHeight);
}

NV21_RenderLogic::~NV21_RenderLogic() noexcept {
    deleteObjects();
    renderProgram.release();
}

const char *NV21_RenderLogic::name() {
    return "NV21_RenderLogic";
}

void NV21_RenderLogic::render(VideoFrame *frame) {
    if (!renderProgram.isReady()) {
        return;
    }

    if (!yBuffer || !uvBuffer || !yTex || !uvTex) {
        LOGE(TAG, "objects not ready");
        return;
    }

    int64_t pixelCount = frameWidth * frameHeight;

    int64_t yCount = pixelCount;
    int64_t uvCount = pixelCount / 2;

    // 将数据拷贝到PBO里
    // y
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    void *ref = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, yCount, GL_MAP_WRITE_BIT);
    memcpy(ref, frame->avFrame->data[0], pixelCount);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // uv
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uvBuffer);
    ref = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, uvCount, GL_MAP_WRITE_BIT);
    memcpy(ref, frame->avFrame->data[1], pixelCount / 2);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // 更新纹理
    // y
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    glBindTexture(GL_TEXTURE_2D, yTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight, GL_RED, GL_UNSIGNED_BYTE,
                    nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // u
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uvBuffer);
    glBindTexture(GL_TEXTURE_2D, uvTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth / 2, frameHeight / 2, GL_RG, GL_UNSIGNED_BYTE,
                    nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    renderProgram.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, yTex);
    renderProgram.setInt("y_tex", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, uvTex);
    renderProgram.setInt("uv_tex", 1);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void NV21_RenderLogic::deleteObjects() {
    if (yBuffer) {
        glDeleteBuffers(1, &yBuffer);
        yBuffer = 0;
    }
    if (uvBuffer) {
        glDeleteBuffers(1, &uvBuffer);
        uvBuffer = 0;
    }


    if (yTex) {
        glDeleteTextures(1, &yTex);
        yTex = 0;
    }
    if (uvTex) {
        glDeleteTextures(1, &uvTex);
        uvTex = 0;
    }
}

void NV21_RenderLogic::prepareObjects(int width, int height) {
    deleteObjects();

    int64_t pixelCount = width * height;
    int64_t yCount = pixelCount;
    int64_t uvCount = pixelCount / 2;

    // 创建buffer
    glGenBuffers(1, &yBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, yCount, nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uvBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, uvCount, nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


    // 创建texture并绑定到pbo
    glBindTexture(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    glGenTextures(1, &yTex);
    glBindTexture(GL_TEXTURE_2D, yTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE,
                 nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


    glBindTexture(GL_PIXEL_UNPACK_BUFFER, uvBuffer);
    glGenTextures(1, &uvTex);
    glBindTexture(GL_TEXTURE_2D, uvTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width / 2, height / 2, 0, GL_RG, GL_UNSIGNED_BYTE,
                 nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

bool NV21_RenderLogic::isReady() {
    return renderProgram.isReady();
}

void NV21_RenderLogic::onFrameSizeChanged(int width, int height) {
    prepareObjects(width, height);
}
