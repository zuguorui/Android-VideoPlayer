//
// Created by zu on 2024/8/6.
//

#include "NV12_RenderLogic.h"
#include "Log.h"

#define TAG "NV12_RenderLogic"

NV12_RenderLogic::NV12_RenderLogic() {
    LOGD(TAG, "NV12_RenderLogic");
    if (!renderProgram.compile(texture_vert_code, render_nv12_frag_code)) {
        LOGE(TAG, "compile program failed");
    }
    prepareObjects(frameWidth, frameHeight);
}

NV12_RenderLogic::~NV12_RenderLogic() noexcept {
    LOGD(TAG, "~NV12_RenderLogic");
    deleteObjects();
    renderProgram.release();
}

const char *NV12_RenderLogic::name() {
    return "NV12_RenderLogic";
}

void NV12_RenderLogic::render(VideoFrame *frame) {
    if (!renderProgram.isReady()) {
        return;
    }

    if (!yBuffer || !uvBuffer || !yTex || !uvTex) {
        LOGE(TAG, "objects not ready");
        return;
    }

    int pixelCount = frameWidth * frameHeight;

    int64_t yCount = pixelCount;
    int64_t uvCount = pixelCount / 2;

    // 将数据拷贝到PBO里
    // y
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    void *ref = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, pixelCount, GL_MAP_WRITE_BIT);
    memcpy(ref, frame->avFrame->data[0], pixelCount);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // uv
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uvBuffer);
    ref = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, pixelCount / 2, GL_MAP_WRITE_BIT);
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


void NV12_RenderLogic::deleteObjects() {
    LOGD(TAG, "deleteObjects");
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

void NV12_RenderLogic::prepareObjects(int width, int height) {
    LOGD(TAG, "prepareObjects, width = %d, height = %d", width, height);
    deleteObjects();

    int64_t pixelCount = width * height;
    int64_t yCount = pixelCount;
    int64_t uvCount = pixelCount / 2;

    // 创建pbo
    glGenBuffers(1, &yBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, pixelCount, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uvBuffer);
    // 注意这里的尺寸，uv总数量是像素量的一半
    glBufferData(GL_PIXEL_UNPACK_BUFFER, pixelCount / 2, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


    // 创建纹理
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, yBuffer);
    glGenTextures(1, &yTex);
    glBindTexture(GL_TEXTURE_2D, yTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE,
                 nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uvBuffer);
    glGenTextures(1, &uvTex);
    glBindTexture(GL_TEXTURE_2D, uvTex);
    // 注意这里uv平面的尺寸是图像宽高的一半
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width / 2, height / 2, 0, GL_RG, GL_UNSIGNED_BYTE,
                 nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

bool NV12_RenderLogic::isReady() {
    return renderProgram.isReady();
}

void NV12_RenderLogic::onFrameSizeChanged(int width, int height) {
    prepareObjects(width, height);
}
