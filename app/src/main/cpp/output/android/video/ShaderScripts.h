//
// Created by 祖国瑞 on 2022/12/7.
//

#ifndef ANDROID_VIDEOPLAYER_SHADERSCRIPTS_H
#define ANDROID_VIDEOPLAYER_SHADERSCRIPTS_H

#include <stdlib.h>

static const char *vertexShaderCode =
        "#version 300 es\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos, 1.0f);\n"
        "    TexCoord = aTexCoord;\n"
        "}\n";


static const char *yuv2rgbShaderCode =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler2D tex_y;\n"
        "uniform sampler2D tex_u;\n"
        "uniform sampler2D tex_v;\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    float y = texture(tex_y, TexCoord).r - 0.0625f;\n"
        "    float u = texture(tex_u, TexCoord).r - 0.5f;\n"
        "    float v = texture(tex_v, TexCoord).r - 0.5f;\n"
        "    float r = 1.164f * y + 1.793f * v;\n"
        "    float g = 1.164f * y - 0.213f * u - 0.533f * v;\n"
        "    float b = 1.164f * y + 2.112f * u;\n"
        "    //float a = texture(tex_y, TexCoord).r;\n"
        "    FragColor = vec4(r, g, b, 1.0f);\n"
        "}\n";

static const char *yuv16ui2rgbShaderCode =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform usampler2D tex_y;\n"
        "uniform usampler2D tex_u;\n"
        "uniform usampler2D tex_v;\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    float y = float(texture(tex_y, TexCoord).r) - 0.0625f;\n"
        "    float u = float(texture(tex_u, TexCoord).r) - 0.5f;\n"
        "    float v = float(texture(tex_v, TexCoord).r) - 0.5f;\n"
        "    float r = 1.164f * y + 1.793f * v;\n"
        "    float g = 1.164f * y - 0.213f * u - 0.533f * v;\n"
        "    float b = 1.164f * y + 2.112f * u;\n"
        "    //float a = texture(tex_y, TexCoord).r;\n"
        "    FragColor = vec4(r, g, b, 1.0f);\n"
        "}\n";

static const char *rgbShaderCode =
        "#version 300 es\n"
        "\n"
        "uniform sampler2D tex_rgb;\n"
        "\n"
        "in vec2 TexCoord;\n"
        "\n"
        "out vec4 FragColor;\n"
        "\n"
        "void main() {\n"
        "    FragColor = texture(tex_rgb, TexCoord);\n"
        "}\n";

#endif //ANDROID_VIDEOPLAYER_SHADERSCRIPTS_H
