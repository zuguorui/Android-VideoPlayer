//
// Created by zu on 2024/8/6.
//

#include "gl_data.h"

float texture_vertices[] = {
        // vertex pos         // tex coords
        -0.5f, -0.5f,  0.0f,  0.0f, 0.0f, // left-bottom
        0.5f, -0.5f,  0.0f,  1.0f, 0.0f, // right-bottom
        0.5f,  0.5f,  0.0f,  1.0f, 1.0f, // right-top
        -0.5f,  0.5f,  0.0f,  0.0f, 1.0f, // left-top
};

unsigned int EBO_indices[] = {
        0, 3, 2,
        2, 1, 0
};