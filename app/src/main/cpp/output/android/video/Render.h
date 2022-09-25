//
// Created by 祖国瑞 on 2020-02-14.
//

#ifndef OPENGLTEST_PICRENDER_H
#define OPENGLTEST_PICRENDER_H

#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "Texture.h"

static const char *PROGRAM_FRAMENT_SHADER =
        "varying highp vec2 v_texcoord;\n"
        "uniform sampler2D yuvTexSampler;\n"
        "void main(){\n"
        "   gl_FragColor = texture2D(yuvTexSampler, v_texcoord);\n"
        "}\n";

static const char *PROGRAM_VERTEX_SHADER =
        "attribute vec4 position;\n"
        "attribute vec2 texcoord;\n"
        "varying vec2 v_texcoord;\n"
        "void main(){\n"
        "   gl_Position = position;\n"
        "   v_texcoord = texcoord;\n"
        "}\n";

class Render {
public:
    Render();
    ~Render();

    bool init(int width, int height, Texture *texture);

    void render();

    void dealloc();

    void resetRenderSize(int left, int top, int width, int height);

private:
    GLint backingLeft;
    GLint backingTop;
    GLint backingWidth;
    GLint backingHeight;

    GLuint vertexShader;
    GLuint fragmentShader;

    Texture *texture;

    GLint uniformSampler;

    GLuint program;

    int useProgram();

    int initShaders();

    GLuint compileShader(GLenum type, const char *source);



};


#endif //OPENGLTEST_PICRENDER_H
