//
// Created by zu on 2024/8/6.
//

#include "gl_utils.h"


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

bool compute_vertex(int screenWidth, int screenHeight, int imageWidth, int imageHeight, int orientation, SizeMode sizeMode,  float *result) {
    if (screenWidth == 0 || screenHeight == 0 || imageWidth == 0 || imageHeight == 0 || result == nullptr) {
        return false;
    }

    float screenLeft, screenRight, screenTop, screenBottom;
    float frameLeft, frameRight, frameTop, frameBottom;

    frameLeft = 0;
    frameRight = 1;
    frameTop = 1;
    frameBottom = 0;

    float frameW2H = imageWidth * 1.0f / imageHeight;
    float screenW2H = screenWidth * 1.0f / screenHeight;

    if (sizeMode == SizeMode::FULL) {
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

    float tmpVert[20] = {
            screenLeft, screenBottom, 0, frameLeft, frameTop,
            screenRight, screenBottom, 0, frameRight, frameTop,
            screenRight, screenTop, 0, frameRight, frameBottom,
            screenLeft, screenTop, 0, frameLeft, frameBottom,
    };

    memcpy(result, tmpVert, 20 * sizeof(float));

    return true;
}
