//
// Created by zu on 2024/8/6.
//

#include "gl_utils.h"
#include "../utils.h"
#include "Log.h"

#define TAG "gl_utils"

using namespace std;

void create_vertex_objects(GLuint &VAO, GLuint &VBO, GLuint &EBO, float *vertexData, unsigned int *indices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), vertexData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool compute_vertex(int screenWidth, int screenHeight, int imageWidth, int imageHeight, int rotation, SizeMode sizeMode,  float *result) {
    if (screenWidth == 0 || screenHeight == 0 || imageWidth == 0 || imageHeight == 0 || result == nullptr) {
        return false;
    }

    float screenLeft, screenRight, screenTop, screenBottom;
    float frameLeft, frameRight, frameTop, frameBottom;

    frameLeft = 0;
    frameRight = 1;
    frameTop = 1;
    frameBottom = 0;

    // 如果是旋转90或者-90，交换图像宽高
    if (rotation % 180 != 0) {
        std::swap(imageWidth, imageHeight);
    }

    float frameW2H = imageWidth * 1.0f / imageHeight;
    float screenW2H = screenWidth * 1.0f / screenHeight;

    if (sizeMode == SizeMode::SCALE_FULL) {
        screenLeft = -1;
        screenRight = 1;
        screenTop = 1;
        screenBottom = -1;
    } else {
        if (frameW2H >= screenW2H) {
            int scaledScreenHeight = (int)(screenWidth * imageHeight * 1.0f / imageWidth);
            screenLeft = -1;
            screenRight = 1;
            screenTop = scaledScreenHeight * 1.0f / screenHeight;
            screenBottom = -screenTop;
        } else {
            int scaledScreenWidth = (int)(screenHeight * imageWidth * 1.0f / imageHeight);
            screenRight = scaledScreenWidth * 1.0f / screenWidth;
            screenLeft = -screenRight;

            screenTop = 1;
            screenBottom = -1;
        }
    }

    /*
     * 将frame坐标放到数组里进行旋转，也就是移动。
     * 注意GL的视图坐标和纹理坐标是上下颠倒的。
     * 坐标循序按视图顺时针：
     * 视图：l-t, r-t, r-b, l-b
     * 纹理：l-b, r-b, r-t, l-t
     *
     * 图像未旋转，不补偿。
     * 图像旋转-90度，需要旋转90度补偿：l-t, l-b, r-b, r-t，即纹理坐标顺时针循环移动1格。
     * 图像旋转90度，需要旋转-90度补偿：r-b, r-t, l-t, l-b，纹理坐标逆时针循环移动1格。
     * 图像旋转180度，需要旋转180度补偿：r-t, l-t, l-b, r-b，纹理坐标移动2格。
     * */
    // framePos存放纹理坐标，固定按l-b, r-b, r-t, l-t的顺序
    pair<float, float> framePos[] = {
            pair(frameLeft, frameBottom),
            pair(frameRight, frameBottom),
            pair(frameRight, frameTop),
            pair(frameLeft, frameTop),
            };

    // 补偿相反的角度
    int moveStep = -rotation / 90;

    cycle_move(framePos, 4, moveStep);

    LOGD(TAG, "rotate: %d, left = %f, top = %f, right = %f, bottom = %f", rotation, frameLeft, frameTop, frameRight, frameBottom);

    float tmpVert[20] = {
            screenLeft, screenTop, 0, framePos[0].first, framePos[0].second,
            screenRight, screenTop, 0, framePos[1].first, framePos[1].second,
            screenRight, screenBottom, 0, framePos[2].first, framePos[2].second,
            screenLeft, screenBottom, 0, framePos[3].first, framePos[3].second,
    };

    memcpy(result, tmpVert, 20 * sizeof(float));

    return true;
}
