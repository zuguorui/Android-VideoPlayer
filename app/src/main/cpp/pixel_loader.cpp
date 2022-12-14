//
// Created by 祖国瑞 on 2022/12/11.
//

#include "pixel_loader.h"

#define TAG "pixel_loader"

int get_pixel_type(AVPixelFormat format) {
    switch (format) {
        case AVPixelFormat::AV_PIX_FMT_YUV444P:
        case AVPixelFormat::AV_PIX_FMT_YUV444P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P16LE:

        case AVPixelFormat::AV_PIX_FMT_YUV422P:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16LE:

        case AVPixelFormat::AV_PIX_FMT_YUV420P:
        case AVPixelFormat::AV_PIX_FMT_NV12:
        case AVPixelFormat::AV_PIX_FMT_NV21:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16LE:

        case AVPixelFormat::AV_PIX_FMT_YUYV422:
        case AVPixelFormat::AV_PIX_FMT_YVYU422:
        case AVPixelFormat::AV_PIX_FMT_UYVY422:
        case AVPixelFormat::AV_PIX_FMT_UYYVYY411:
            return PIXEL_TYPE_YUV;
        case AVPixelFormat::AV_PIX_FMT_RGB24:
        case AVPixelFormat::AV_PIX_FMT_RGB555BE:
        case AVPixelFormat::AV_PIX_FMT_RGB555LE:
        case AVPixelFormat::AV_PIX_FMT_RGB565BE:
        case AVPixelFormat::AV_PIX_FMT_RGB565LE:
        case AVPixelFormat::AV_PIX_FMT_BGR24:
        case AVPixelFormat::AV_PIX_FMT_BGR555BE:
        case AVPixelFormat::AV_PIX_FMT_BGR555LE:
        case AVPixelFormat::AV_PIX_FMT_BGR565BE:
        case AVPixelFormat::AV_PIX_FMT_BGR565LE:
            return PIXEL_TYPE_RGB;
        default:
            return PIXEL_TYPE_UNKNOWN;
    }
}

int get_pixel_layout(AVPixelFormat format) {
    switch (format) {
        case AVPixelFormat::AV_PIX_FMT_YUV444P:
        case AVPixelFormat::AV_PIX_FMT_YUV444P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P16LE:

        case AVPixelFormat::AV_PIX_FMT_YUV422P:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16LE:

        case AVPixelFormat::AV_PIX_FMT_YUV420P:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16LE:
            return PIXEL_LAYOUT_PLANNER;
        case AVPixelFormat::AV_PIX_FMT_YUYV422:
        case AVPixelFormat::AV_PIX_FMT_YVYU422:
        case AVPixelFormat::AV_PIX_FMT_UYVY422:
        case AVPixelFormat::AV_PIX_FMT_UYYVYY411:

        case AVPixelFormat::AV_PIX_FMT_RGB24:
        case AVPixelFormat::AV_PIX_FMT_RGB555BE:
        case AVPixelFormat::AV_PIX_FMT_RGB555LE:
        case AVPixelFormat::AV_PIX_FMT_RGB565BE:
        case AVPixelFormat::AV_PIX_FMT_RGB565LE:
        case AVPixelFormat::AV_PIX_FMT_BGR24:
        case AVPixelFormat::AV_PIX_FMT_BGR555BE:
        case AVPixelFormat::AV_PIX_FMT_BGR555LE:
        case AVPixelFormat::AV_PIX_FMT_BGR565BE:
        case AVPixelFormat::AV_PIX_FMT_BGR565LE:
            return PIXEL_LAYOUT_PACKET;
        case AVPixelFormat::AV_PIX_FMT_NV12:
        case AVPixelFormat::AV_PIX_FMT_NV21:
            return PIXEL_LAYOUT_SEMI_PLANNER;
        default:
            return PIXEL_LAYOUT_UNKNOWN;
    }
}

bool get_yuv_comp_count_ratio(AVPixelFormat format, int *y2u, int *y2v) {
    switch (format) {
        case AVPixelFormat::AV_PIX_FMT_YUV444P:
        case AVPixelFormat::AV_PIX_FMT_YUV444P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P16LE:

        case AVPixelFormat::AV_PIX_FMT_RGB24:
        case AVPixelFormat::AV_PIX_FMT_RGB555BE:
        case AVPixelFormat::AV_PIX_FMT_RGB555LE:
        case AVPixelFormat::AV_PIX_FMT_RGB565BE:
        case AVPixelFormat::AV_PIX_FMT_RGB565LE:
        case AVPixelFormat::AV_PIX_FMT_BGR24:
        case AVPixelFormat::AV_PIX_FMT_BGR555BE:
        case AVPixelFormat::AV_PIX_FMT_BGR555LE:
        case AVPixelFormat::AV_PIX_FMT_BGR565BE:
        case AVPixelFormat::AV_PIX_FMT_BGR565LE:
            *y2u = 1;
            *y2v = 1;
            return true;
        case AVPixelFormat::AV_PIX_FMT_YUV422P:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16LE:
            *y2u = 2;
            *y2v = 2;
            return true;
        case AVPixelFormat::AV_PIX_FMT_YUV420P:
        case AVPixelFormat::AV_PIX_FMT_NV12:
        case AVPixelFormat::AV_PIX_FMT_NV21:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16LE:
            *y2u = 4;
            *y2v = 4;
            return true;
        default:
            return false;
    }
}

int get_yuv_comp_depth(AVPixelFormat format) {
    switch (format) {
        case AVPixelFormat::AV_PIX_FMT_YUV444P:
        case AVPixelFormat::AV_PIX_FMT_YUV422P:
        case AVPixelFormat::AV_PIX_FMT_YUV420P:
        case AVPixelFormat::AV_PIX_FMT_NV12:
        case AVPixelFormat::AV_PIX_FMT_NV21:
        case AVPixelFormat::AV_PIX_FMT_YUYV422:
        case AVPixelFormat::AV_PIX_FMT_YVYU422:
        case AVPixelFormat::AV_PIX_FMT_UYVY422:
        case AVPixelFormat::AV_PIX_FMT_UYYVYY411:
            return 8;
        case AVPixelFormat::AV_PIX_FMT_YUV444P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10LE:
            return 10;
        case AVPixelFormat::AV_PIX_FMT_YUV444P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12LE:
            return 12;
        case AVPixelFormat::AV_PIX_FMT_YUV444P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14LE:
            return 14;
        case AVPixelFormat::AV_PIX_FMT_YUV444P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P16LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16LE:
            return 16;
        default:
            return -1;

    }
}

bool get_yuv_comp_size_ratio(AVPixelFormat format, int *y2u_width, int *y2u_height, int *y2v_width, int *y2v_height) {
    switch (format) {
        case AVPixelFormat::AV_PIX_FMT_YUV444P:
        case AVPixelFormat::AV_PIX_FMT_YUV444P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV444P16LE:
            *y2u_width = 1;
            *y2u_height = 1;
            *y2v_width = 1;
            *y2v_height = 1;
            return true;
        case AVPixelFormat::AV_PIX_FMT_YUV422P:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV422P16LE:
        case AVPixelFormat::AV_PIX_FMT_UYVY422:
        case AVPixelFormat::AV_PIX_FMT_YUYV422:
        case AVPixelFormat::AV_PIX_FMT_YVYU422:
            *y2u_width = 2;
            *y2u_height = 1;
            *y2v_width = 2;
            *y2v_height = 1;
            return true;
        case AVPixelFormat::AV_PIX_FMT_YUV420P:
        case AVPixelFormat::AV_PIX_FMT_NV12:
        case AVPixelFormat::AV_PIX_FMT_NV21:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P10LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P12LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P14LE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16BE:
        case AVPixelFormat::AV_PIX_FMT_YUV420P16LE:
        case AVPixelFormat::AV_PIX_FMT_UYYVYY411:
            *y2u_width = 2;
            *y2u_height = 2;
            *y2v_width = 2;
            *y2v_height = 2;
            return true;
        default:
            return false;
    }
}

bool compute_yuv_buffer_size(AVPixelFormat format, int64_t width, int64_t height, int64_t *ySize, int64_t *uSize, int64_t *vSize) {
    int compValidBits = get_yuv_comp_depth(format);
    if (compValidBits <= 0) {
        return false;
    }
    int compUseBits = 8;
    if (compValidBits > 8) {
        compUseBits = 16;
    }

    int compUseBytes = compUseBits / 8;

    int y2u = 1;
    int y2v = 1;

    if (!get_yuv_comp_count_ratio(format, &y2u, &y2v)) {
        return false;
    }

    int64_t pixelCount = width * height;

    int64_t yCount = pixelCount;
    int64_t uCount = yCount / y2u;
    int64_t vCount = yCount  / y2v;

    *ySize = yCount * compUseBytes;
    *uSize = uCount * compUseBytes;
    *vSize = vCount * compUseBytes;

    return true;
}

bool read_yuv_pixel(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                    uint8_t *yBuffer, int *yWidth, int *yHeight,
                    uint8_t *uBuffer, int *uWidth, int *uHeight,
                    uint8_t *vBuffer, int *vWidth, int *vHeight) {

    int layout = get_pixel_layout(format);
    if (layout == PIXEL_LAYOUT_PLANNER) {
        return read_yuv_planner(frame, format, width, height, yBuffer, yWidth, yHeight, uBuffer, uWidth, uHeight, vBuffer, vWidth, vHeight);
    } else if (layout == PIXEL_LAYOUT_PACKET) {
        return read_yuv_packet(frame, format, width, height, yBuffer, yWidth, yHeight, uBuffer, uWidth, uHeight, vBuffer, vWidth, vHeight);
    } else if (layout == PIXEL_LAYOUT_SEMI_PLANNER) {
        return read_yuv_semi_planner(frame, format, width, height, yBuffer, yWidth, yHeight, uBuffer, uWidth, uHeight, vBuffer, vWidth, vHeight);
    } else {
        return false;
    }
}

bool read_yuv_planner(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                      uint8_t *yBuffer, int *yWidth, int *yHeight,
                      uint8_t *uBuffer, int *uWidth, int *uHeight,
                      uint8_t *vBuffer, int *vWidth, int *vHeight) {

    int compValidBits = get_yuv_comp_depth(format);
    if (compValidBits <= 0) {
        return false;
    }
    int compUseBits = 8;
    if (compValidBits > 8) {
        compUseBits = 16;
    }

    int compUseBytes = compUseBits / 8;

    int shiftBits = compUseBits - compValidBits;

    int y2u_width, y2u_height, y2v_width, y2v_height;

    if (!get_yuv_comp_size_ratio(format, &y2u_width, &y2u_height, &y2v_width, &y2v_height)) {
        LOGE(TAG, "get_yuv_comp_size_ratio false");
        return false;
    }

    *yWidth = width;
    *yHeight = height;
    *uWidth = *yWidth / y2u_width;
    *uHeight = *yHeight / y2u_height;
    *vWidth = *yWidth / y2v_width;
    *vHeight = *yHeight / y2v_height;

    int64_t yCount = (*yWidth) * (*yHeight);
    int64_t uCount = (*uWidth) * (*uHeight);
    int64_t vCount = (*vWidth) * (*vHeight);

    memcpy(yBuffer, frame->data[0], yCount * compUseBytes);
    memcpy(uBuffer, frame->data[1], uCount * compUseBytes);
    memcpy(vBuffer, frame->data[2], vCount * compUseBytes);

    if (shiftBits != 0) {
        if (compUseBytes == 2) {
            uint16_t *ys = (uint16_t *)yBuffer;
            for (int64_t i = 0; i < yCount; i++) {
                ys[i] <<= shiftBits;
            }

            uint16_t *us = (uint16_t *)uBuffer;
            for (int64_t i = 0; i <uCount; i++) {
                us[i] <<= shiftBits;
            }

            uint16_t *vs = (uint16_t *)vBuffer;
            for (int64_t i = 0; i < vCount; i++) {
                vs[i] <<= shiftBits;
            }
        } else if (compUseBytes == 4) {
            uint32_t *ys = (uint32_t *)yBuffer;
            for (int64_t i = 0; i < yCount; i++) {
                ys[i] <<= shiftBits;
            }

            uint32_t *us = (uint32_t *)uBuffer;
            for (int64_t i = 0; i <uCount; i++) {
                us[i] <<= shiftBits;
            }

            uint32_t *vs = (uint32_t *)vBuffer;
            for (int64_t i = 0; i < vCount; i++) {
                vs[i] <<= shiftBits;
            }
        } else {
            LOGE(TAG, "shiftBits = %d, compValidBits = %d, compUsedBits = %d", shiftBits, compValidBits, compUseBits);
        }
    }

    return true;
}

const static int Y_INDEX = 0;
const static int U_INDEX = 1;
const static int V_INDEX = 2;

bool read_yuv_packet(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                     uint8_t *yBuffer, int *yWidth, int *yHeight,
                     uint8_t *uBuffer, int *uWidth, int *uHeight,
                     uint8_t *vBuffer, int *vWidth, int *vHeight) {
    // yuv packet is always 8bit depth
//    int compValidBits = get_yuv_comp_depth(format);
//    if (compValidBits <= 0) {
//        return false;
//    }
//    int compUseBits = 8;
//    if (compValidBits > 8) {
//        compUseBits = 16;
//    }
//
//    int compUseBytes = compUseBits / 8;
//
//    int shiftBits = compUseBits - compValidBits;

    int y2u_width, y2u_height, y2v_width, y2v_height;

    if (!get_yuv_comp_size_ratio(format, &y2u_width, &y2u_height, &y2v_width, &y2v_height)) {
        LOGE(TAG, "get_yuv_comp_size_ratio false");
        return false;
    }



    *yWidth = width;
    *yHeight = height;
    *uWidth = *yWidth / y2u_width;
    *uHeight = *yHeight / y2u_height;
    *vWidth = *yWidth / y2v_width;
    *vHeight = *yHeight / y2v_height;

    int64_t yCount = (*yWidth) * (*yHeight);
    int64_t uCount = (*uWidth) * (*uHeight);
    int64_t vCount = (*vWidth) * (*vHeight);

    uint8_t *dstBuffers[] = { yBuffer, uBuffer, vBuffer };

    int compIndex[8];
    int stride = 0;

    int64_t yWriteIndex = 0;
    int64_t uWriteIndex = 0;
    int64_t vWriteIndex = 0;

    int64_t *writeIndex[3] = { &yWriteIndex, &uWriteIndex, &vWriteIndex };

    if (format == AVPixelFormat::AV_PIX_FMT_YUYV422) {
        compIndex[0] = Y_INDEX;
        compIndex[1] = U_INDEX;
        compIndex[2] = Y_INDEX;
        compIndex[3] = V_INDEX;
        stride = 4;
    } else if (format == AVPixelFormat::AV_PIX_FMT_YVYU422) {
        compIndex[0] = Y_INDEX;
        compIndex[1] = V_INDEX;
        compIndex[2] = Y_INDEX;
        compIndex[3] = U_INDEX;
        stride = 4;

    } else if (format == AVPixelFormat::AV_PIX_FMT_UYVY422) {
        compIndex[0] = U_INDEX;
        compIndex[1] = Y_INDEX;
        compIndex[2] = V_INDEX;
        compIndex[3] = Y_INDEX;
        stride = 4;
    } else if (format == AVPixelFormat::AV_PIX_FMT_UYYVYY411) {
        compIndex[0] = U_INDEX;
        compIndex[1] = Y_INDEX;
        compIndex[2] = Y_INDEX;
        compIndex[3] = V_INDEX;
        compIndex[4] = Y_INDEX;
        compIndex[5] = Y_INDEX;
        stride = 6;
    } else {
        LOGE(TAG, "unsupported yuv packet format %d", format);
        return false;
    }

    int64_t loopCount = uCount;
    int cj;
    int64_t *wj;
    uint8_t *bj;
    for (int64_t i = 0; i < loopCount; i += stride) {
        for (int j = 0; j < stride; j++) {
            cj = compIndex[j];
            wj = writeIndex[cj];
            bj = dstBuffers[cj];
            bj[*wj] = frame->data[0][i + j];
            (*wj) += 1;
        }
    }

    return true;

}


bool read_yuv_semi_planner(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                           uint8_t *yBuffer, int *yWidth, int *yHeight,
                           uint8_t *uBuffer, int *uWidth, int *uHeight,
                           uint8_t *vBuffer, int *vWidth, int *vHeight) {

    int y2u_width, y2u_height, y2v_width, y2v_height;

    if (!get_yuv_comp_size_ratio(format, &y2u_width, &y2u_height, &y2v_width, &y2v_height)) {
        LOGE(TAG, "get_yuv_comp_size_ratio false");
        return false;
    }



    *yWidth = width;
    *yHeight = height;
    *uWidth = *yWidth / y2u_width;
    *uHeight = *yHeight / y2u_height;
    *vWidth = *yWidth / y2v_width;
    *vHeight = *yHeight / y2v_height;

    int64_t yCount = (*yWidth) * (*yHeight);
    int64_t uCount = (*uWidth) * (*uHeight);
    int64_t vCount = (*vWidth) * (*vHeight);

    uint8_t *uvBuffers[] = { uBuffer, vBuffer };

    int compIndex[2];

    int64_t uWriteIndex = 0;
    int64_t vWriteIndex = 0;

    int64_t *writeIndex[] = { &uWriteIndex, &vWriteIndex };

    if (format == AVPixelFormat::AV_PIX_FMT_NV12) {
        compIndex[0] = U_INDEX;
        compIndex[1] = V_INDEX;
    } else if (format == AVPixelFormat::AV_PIX_FMT_NV21) {
        compIndex[0] = V_INDEX;
        compIndex[1] = U_INDEX;
    } else {
        LOGE(TAG, "unsupported yuv semi-planned format %d", format);
        return false;
    }

    int64_t loopCount = uCount;
    int cj;
    int64_t *wj;
    uint8_t *bj;

    for (int64_t i = 0; i < loopCount; i += 2) {
        for (int j = 0; j < 2; j++) {
            cj = compIndex[j];
            wj = writeIndex[cj];
            bj = uvBuffers[cj];
            bj[*wj] = frame->data[1][i + j];
            (*wj) += 1;
        }
    }

    return true;

}




