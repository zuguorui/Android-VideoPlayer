//
// Created by 祖国瑞 on 2022/12/6.
//

#include "Shader.h"
#include "Log.h"

using namespace std;

#define TAG "Shader"

Shader::Shader() {

}

Shader::Shader(const char *vertexShaderCode, const char *fragmentShaderCode) {
    compileShader(vertexShaderCode, fragmentShaderCode);
}

Shader::~Shader() {
    release();
}

bool Shader::compileShader(const char *vertexShaderCode, const char *fragmentShaderCode) {

    release();
    const static int LOG_BUFFER_SIZE = 512;

    GLint success;
    char infoLog[LOG_BUFFER_SIZE];

    GLuint fragmentShader = 0;
    GLuint vertexShader = 0;

    bool result = true;

    try {


        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, LOG_BUFFER_SIZE, nullptr, infoLog);
            LOGE(TAG, "compile vertex shader failed, log: %s", infoLog);
            throw "compile vertex shader failed";
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, LOG_BUFFER_SIZE, nullptr, infoLog);
            LOGE(TAG, "compile fragment shader failed, log: %s", infoLog);
            throw "compile fragment shader failed";
        }

        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);

        glLinkProgram(ID);

        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(ID, LOG_BUFFER_SIZE, nullptr, infoLog);
            LOGE(TAG, "link program failed, log: %s", infoLog);
            throw "link program failed";
        }
    } catch (string msg) {
        LOGE(TAG, "%s", msg.c_str());
        release();
    }

    if (vertexShader > 0) {
        glDeleteShader(vertexShader);
    }

    if (fragmentShader > 0) {
        glDeleteShader(fragmentShader);
    }

    return result;
}

void Shader::release() {
    if (ID > 0) {
        glDeleteProgram(ID);
    }
    ID = 0;
}

void Shader::use() {
    if (ID > 0) {
        glUseProgram(ID);
    }
}

bool Shader::isReady() {
    return ID > 0;
}

GLuint Shader::getProgramID() {
    return ID;
}

void Shader::setBool(const std::string &name, bool value) const {
    if (ID > 0) {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
}

void Shader::setInt(const std::string &name, int value) const {
    if (ID > 0) {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
}

void Shader::setFloat(const std::string &name, float value) const {
    if (ID > 0) {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
}

void Shader::setVec2(const std::string &name, float x, float y) {
    if (ID > 0) {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
}

void Shader::setVec2(const std::string &name, const float *value) {
    if (ID > 0) {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, value);
    }
}

void Shader::setVec3(const std::string &name, float x, float y, float z) {
    if (ID > 0) {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
}

void Shader::setVec3(const std::string &name, const float *value) {
    if (ID > 0) {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value);
    }
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) {
    if (ID > 0) {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
}

void Shader::setVec4(const std::string &name, const float *value) {
    if (ID > 0) {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, value);
    }
}

