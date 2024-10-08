//
// Created by 祖国瑞 on 2022/12/7.
//

#ifndef ANDROID_VIDEOPLAYER_SHADERSCRIPTS_H
#define ANDROID_VIDEOPLAYER_SHADERSCRIPTS_H

#include <stdlib.h>


static const char* texture_vert_code =
        "#version 310 es\n"
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

static const char* render_yuv420p_frag_code =
        "#version 310 es\n"
        "\n"
        "precision mediump float;\n"
        "\n"
        "uniform sampler2D y_tex;\n"
        "uniform sampler2D u_tex;\n"
        "uniform sampler2D v_tex;\n"
        "\n"
        "\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "\n"
        "void main() {\n"
        "    float y = texture(y_tex, TexCoord).r;\n"
        "    float u = texture(u_tex, TexCoord).r;\n"
        "    float v = texture(v_tex, TexCoord).r;\n"
        "\n"
        "    float y1 = y;\n"
        "    float u1 = u;\n"
        "    float v1 = v;\n"
        "\n"
        "    y -= 0.0625f;\n"
        "    u -= 0.5f;\n"
        "    v -= 0.5f;\n"
        "\n"
        "    float r = 1.164f * y + 1.793f * v;\n"
        "    float g = 1.164f * y - 0.213f * u - 0.533f * v;\n"
        "    float b = 1.164f * y + 2.112f * u;\n"
        "\n"
        "    FragColor = vec4(r, g, b, 1.0f);\n"
        "}\n"
        "\n";

static const char* render_nv21_frag_code =
        "#version 310 es\n"
        "\n"
        "precision mediump float;\n"
        "\n"
        "uniform sampler2D y_tex;\n"
        "uniform sampler2D uv_tex;\n"
        "\n"
        "\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "\n"
        "void main() {\n"
        "    float y = texture(y_tex, TexCoord).r;\n"
        "    vec2 uv = texture(uv_tex, TexCoord).rg;\n"
        "    // nv21是先v后u\n"
        "    float v = uv.r;\n"
        "    float u = uv.g;\n"
        "\n"
        "    float y1 = y;\n"
        "    float u1 = u;\n"
        "    float v1 = v;\n"
        "\n"
        "    y -= 0.0625f;\n"
        "    u -= 0.5f;\n"
        "    v -= 0.5f;\n"
        "\n"
        "    float r = 1.164f * y + 1.793f * v;\n"
        "    float g = 1.164f * y - 0.213f * u - 0.533f * v;\n"
        "    float b = 1.164f * y + 2.112f * u;\n"
        "    FragColor = vec4(r, g, b, 1.0f);\n"
        "}\n"
        "\n";

static const char* render_nv12_frag_code =
        "#version 310 es\n"
        "\n"
        "precision mediump float;\n"
        "\n"
        "uniform sampler2D y_tex;\n"
        "uniform sampler2D uv_tex;\n"
        "\n"
        "\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "\n"
        "void main() {\n"
        "    float y = texture(y_tex, TexCoord).r;\n"
        "    vec2 uv = texture(uv_tex, TexCoord).rg;\n"
        "    // nv12是先u后v\n"
        "    float v = uv.g;\n"
        "    float u = uv.r;\n"
        "\n"
        "    float y1 = y;\n"
        "    float u1 = u;\n"
        "    float v1 = v;\n"
        "\n"
        "    y -= 0.0625f;\n"
        "    u -= 0.5f;\n"
        "    v -= 0.5f;\n"
        "\n"
        "    float r = 1.164f * y + 1.793f * v;\n"
        "    float g = 1.164f * y - 0.213f * u - 0.533f * v;\n"
        "    float b = 1.164f * y + 2.112f * u;\n"
        "    FragColor = vec4(r, g, b, 1.0f);\n"
        "}\n"
        "\n";

#endif //ANDROID_VIDEOPLAYER_SHADERSCRIPTS_H
