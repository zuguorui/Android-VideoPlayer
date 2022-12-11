//
// Created by 祖国瑞 on 2022/12/11.
//

#include "pixel_loader.h"

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
        default:
            return PIXEL_LAYOUT_UNKNOWN;
    }
}

bool compute_yuv_count_ratio(AVPixelFormat format, int *y2u, int *y2v) {
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

    if (!compute_yuv_count_ratio(format, &y2u, &y2u)) {
        return false;
    }

    int64_t pixelCount = width * height;

    int64_t yCount = pixelCount;
    int64_t uCount = yCount / y2u;
    int64_t vCount = yCount  / y2v;

    *ySize = yCount * compUseBytes;
    *uSize = uCount * compUseBytes;
    *vSize = vCount * compUseBytes;
}

bool read_yuv_pixel(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                           void *yBuffer, int *yWidth, int *yHeight,
                           void *uBuffer, int *uWidth, int *uHeight,
                           void *vBuffer, int *vWidth, int *vHeight) {

    int layout = get_pixel_layout(format);
    if (layout == PIXEL_LAYOUT_PLANNER) {
        return read_yuv_planner(frame, format, width, height, yBuffer, yWidth, yHeight, uBuffer, uWidth, uHeight, vBuffer, vWidth, vHeight);
    } else if (layout == PIXEL_LAYOUT_PACKET) {
        return read_yuv_packet(frame, format, width, height, yBuffer, yWidth, yHeight, uBuffer, uWidth, uHeight, vBuffer, vWidth, vHeight);
    } else {
        return false;
    }
}

bool read_yuv_planner(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                      void *yBuffer, int *yWidth, int *yHeight,
                      void *uBuffer, int *uWidth, int *uHeight,
                      void *vBuffer, int *vWidth, int *vHeight) {

    int compValidBits = get_yuv_comp_depth(format);
    if (compValidBits <= 0) {
        return false;
    }
    int compUseBits = 8;
    if (compValidBits > 8) {
        compUseBits = 16;
    }

    int compUseBytes = compUseBits / 8;


}




