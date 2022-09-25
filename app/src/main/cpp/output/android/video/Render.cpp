//
// Created by 祖国瑞 on 2020-02-14.
//

#include "Render.h"
#include <android/log.h>
#include <iostream>

using namespace std;

#define MODULE_NAME "Render"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

enum {
    ATTRIBUTE_VERTEX, ATTRIBUTE_TEXCOORD
};



Render::Render() {

}

Render::~Render() {

}

bool Render::init(int width, int height, Texture *texture) {
    backingLeft = 0;
    backingTop = 0;
    backingWidth = width;
    backingHeight = height;

    this->texture = texture;

    vertexShader = 0;
    fragmentShader = 0;
    program = 0;

    int ret = initShaders();
    if(ret < 0)
    {
        LOGE("init shader failed");
        dealloc();
        return false;
    }

    ret = useProgram();
    if(ret < 0)
    {
        LOGE("use program failed");
        dealloc();
        return false;
    }
    return true;


}


void Render::resetRenderSize(int left, int top, int width, int height) {
    backingLeft = left;
    backingTop = top;
    backingWidth = width;
    backingHeight = height;
}

void Render::render() {
    glViewport(backingLeft, backingTop, backingWidth, backingHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);

//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(program);

    static const GLfloat _vertices[] = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f};

    glVertexAttribPointer(ATTRIBUTE_VERTEX, 2, GL_FLOAT, 0, 0, _vertices);
    glEnableVertexAttribArray(ATTRIBUTE_VERTEX);

    static const GLfloat _texcoords[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    glVertexAttribPointer(ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, 0, 0, _texcoords);
    glEnableVertexAttribArray(ATTRIBUTE_TEXCOORD);

    texture->bindTexture(uniformSampler);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::dealloc() {
    if(vertexShader)
    {
        glDeleteShader(vertexShader);
    }
    if(fragmentShader)
    {
        glDeleteShader(fragmentShader);
    }

    if(program)
    {
        glDeleteProgram(program);
    }
}

int Render::initShaders() {
    vertexShader = compileShader(GL_VERTEX_SHADER, PROGRAM_VERTEX_SHADER);
    if(!vertexShader)
    {
        return -1;
    }

    fragmentShader = compileShader(GL_FRAGMENT_SHADER, PROGRAM_FRAMENT_SHADER);
    if(!fragmentShader)
    {
        return -1;
    }
    return 0;
}

GLuint Render::compileShader(GLenum type, const char *source) {
    GLint status;
    GLuint shader = glCreateShader(type);
    if(shader == 0 || shader == GL_INVALID_ENUM)
    {
        LOGE("failed to create shader %d", type);
        return 0;
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
    {
        LOGE("compile shader failed: ");
        glDeleteShader(shader);
        int logLen = 0;
        int maxLogLen = 200;
        char log[200];
        glGetShaderInfoLog(shader, maxLogLen, &logLen, log);
        LOGE("compile shader log: %s", log);
        return 0;
    }
    return shader;
}

int Render::useProgram() {
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glBindAttribLocation(program, ATTRIBUTE_VERTEX, "position");
    glBindAttribLocation(program, ATTRIBUTE_TEXCOORD, "texcoord");
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        LOGE("link program failed: ");
        glDeleteProgram(program);
        int logLen = 0;
        int maxLogLen = 200;
        char log[200];
        glGetProgramInfoLog(program, maxLogLen, &logLen, log);
        LOGE("link program log: %s", log);
        return -1;
    }
    glUseProgram(program);
    uniformSampler = glGetUniformLocation(program, "yuvTexSampler");
    return 0;
}
