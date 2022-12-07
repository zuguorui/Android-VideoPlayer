//
// Created by 祖国瑞 on 2022/12/7.
//

#ifndef ANDROID_VIDEOPLAYER_SHADERSCRIPTS_H
#define ANDROID_VIDEOPLAYER_SHADERSCRIPTS_H

#include <stdlib.h>

static const char *vertexShaderCode =
        "#version 330 core\n"
        "\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "\n"
        "out vec2 TexCoord;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos, 1.0f);\n"
        "    TexCoord = aTexCoord;\n"
        "}";

static const char *yuv2rgbShaderCode =
        "#version 330 core\n"
        "\n"
        "uniform sampler2D tex_y;\n"
        "uniform sampler2D tex_u;\n"
        "uniform sampler2D tex_v;\n"
        "\n"
        "in vec2 TexCoord;\n"
        "\n"
        "out vec4 FragColor;\n"
        "\n"
        "// see: https://zhuanlan.zhihu.com/p/436186749\n"
        "\n"
        "void main() {\n"
        "\n"
        "    // 部分色域BT709\n"
        "    float y = texture(tex_y, TexCoord).r - 0.0625f;\n"
        "    float u = texture(tex_u, TexCoord).r - 0.5f;\n"
        "    float v = texture(tex_v, TexCoord).r - 0.5f;\n"
        "\n"
        "    float r = 1.164f * y + 1.793f * v;\n"
        "    float g = 1.164f * y - 0.213f * u - 0.533f * v;\n"
        "    float b = 1.164f * y + 2.112f * u;\n"
        "    FragColor = vec4(r, g, b, 1.0f);\n"
        "}";

static const char *rgbShaderCode =
        "#version 330 core\n"
        "\n"
        "uniform sampler2D tex_rgb;\n"
        "\n"
        "in vec2 TexCoord;\n"
        "\n"
        "out vec4 FragColor;\n"
        "\n"
        "void main() {\n"
        "    FragColor = texture(tex_rgb, TexCoord);\n"
        "}";

#endif //ANDROID_VIDEOPLAYER_SHADERSCRIPTS_H
