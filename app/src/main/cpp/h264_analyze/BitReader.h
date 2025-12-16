//
// Created by zu on 2025/12/10.
//

#ifndef LB_CAMERA_SDK_V2_BITREADER_H
#define LB_CAMERA_SDK_V2_BITREADER_H

#include <stdlib.h>

class BitReader {
public:
    virtual uint32_t readBits(int bits) = 0;
    virtual uint32_t nextBits(int bits) = 0;

    virtual uint8_t readByte() = 0;
    virtual uint8_t nextByte() = 0;

    virtual bool byteAligned() = 0;
    virtual bool moreBitInByteStream() = 0;
    virtual bool hasRemainBytes(int n) = 0;

    bool moreByteInByteStream() {
        return hasRemainBytes(1);
    }

    virtual uint32_t f(int bits) = 0;
    virtual uint32_t u(int bits) = 0;
    virtual uint32_t b(int bits) = 0;
    virtual int32_t i(int bits) = 0;
    virtual uint32_t ue() = 0;
    virtual int32_t se() = 0;
    virtual void alignToNextByte() = 0;
    virtual int64_t currentBitPosition() = 0;
};

#endif //LB_CAMERA_SDK_V2_BITREADER_H
