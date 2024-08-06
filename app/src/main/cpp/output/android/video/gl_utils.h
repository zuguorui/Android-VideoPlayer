//
// Created by zu on 2024/8/6.
//

#pragma once

#include <iostream>
#include <stdlib.h>
#include <GLES3/gl31.h>
#include <GLES3/gl3platform.h>
#include "SizeMode.h"

void create_vertex_objects(GLuint &VAO, GLuint &VBO, GLuint &EBO, float *vertexData, unsigned int *indices);

bool compute_vertex(int screenWidth, int screenHeight, int imageWidth, int imageHeight, int orientation, SizeMode sizeMode, float *result);
