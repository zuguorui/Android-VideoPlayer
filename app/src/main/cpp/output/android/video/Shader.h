//
// Created by 祖国瑞 on 2022/12/6.
//

#ifndef ANDROID_VIDEOPLAYER_SHADER_H
#define ANDROID_VIDEOPLAYER_SHADER_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "GLES3/gl3.h"
#include "GLES3/gl3platform.h"

class Shader {
public:
    Shader();
    Shader(const char *vertexShaderCode, const char *fragmentShaderCode);
    ~Shader();

    bool compileShader(const char *vertexShaderCode, const char *fragmentShaderCode);

    bool isReady();

    void use();

    void release();

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;

    void setVec2(const std::string &name, const float *value);
    void setVec2(const std::string &name, float x, float y);

    void setVec3(const std::string &name, const float *value);
    void setVec3(const std::string &name, float x, float y, float z);

    void setVec4(const std::string &name, const float *value);
    void setVec4(const std::string &name, float x, float y, float z, float w);

    GLuint getProgramID();

private:
    GLuint ID = 0;
};


#endif //ANDROID_VIDEOPLAYER_SHADER_H
