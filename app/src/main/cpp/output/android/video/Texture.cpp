//
// Created by 祖国瑞 on 2020-02-14.
//

#include "Texture.h"
#include <android/log.h>

#define MODULE_NAME "Texture"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

bool checkGlError(const char* op) {
    GLint error;
    for (error = glGetError(); error; error = glGetError()) {
        LOGE("error::after %s() glError (0x%x)\n", op, error);
        return true;
    }
    return false;
}

Texture::Texture() {

}

Texture::~Texture() {

}

bool Texture::createTexture() {

    int ret = initTexture();
    if(ret < 0)
    {
        LOGE("init texture failed");
        dealloc();
        return false;
    }
    return true;
}

int Texture::initTexture() {
    texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return 1;
}

void Texture::updateDataToTexture(uint8_t *pixels, int width, int height) {
    if(pixels)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        if(checkGlError("glBindTexture"))
        {
            LOGE("update pic glBindTexture error");
            return;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    }
}

bool Texture::bindTexture(GLint uniformSampler) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    if(checkGlError("glBindTexture"))
    {
        LOGE("bind texture glBindTexture error");
        return false;
    }

    glUniform1i(uniformSampler, 0);
    return true;
}

void Texture::dealloc() {
    if(texture)
    {
        glDeleteTextures(1, &texture);
    }
}
