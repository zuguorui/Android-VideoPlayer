//
// Created by zu on 2024/8/6.
//

#pragma once

#include "IRenderLogic.h"
#include "../RenderProgram.h"
#include "../ShaderScripts.h"

class NV21_RenderLogic: public IRenderLogic{
public:
    NV21_RenderLogic();
    ~NV21_RenderLogic();

    const char *name() override;

    bool isReady() override;

    void render(VideoFrame *frame) override;

    void onFrameSizeChanged(int width, int height) override;

private:
    GLuint yBuffer, uvBuffer = 0;
    GLuint yTex = 0, uvTex = 0;

    RenderProgram renderProgram;

    void prepareObjects(int width, int height);

    void deleteObjects();
};


