//
// Created by WangXi on 2025/11/6.
//

#ifndef ANALYZE_H264_SPS_BUFFERBITREADER_H
#define ANALYZE_H264_SPS_BUFFERBITREADER_H

#include <cstdint>
#include <stdlib.h>
#include <vector>
#include "BitReader.h"

class BufferBitReader: public BitReader {
public:
    BufferBitReader(std::vector<uint8_t> &vec);
    BufferBitReader(uint8_t *buffer, int size);

    uint32_t readBits(int bits) override;
    uint32_t nextBits(int bits) override;

    uint8_t readByte() override;
    uint8_t nextByte() override;

    bool byteAligned() override;
    bool moreBitInByteStream() override;
    bool hasRemainBytes(int n) override;

    uint32_t f(int bits) override;
    uint32_t u(int bits) override;
    uint32_t b(int bits) override;
    int32_t i(int bits) override;
    uint32_t ue() override;
    int32_t se() override;

    void alignToNextByte() override;

    void reset();

    size_t getSize();

    int64_t currentBitPosition() override;

private:
    int64_t bitPosInBuffer = 0L;
    std::vector<uint8_t> buffer;
};


#endif //ANALYZE_H264_SPS_BUFFERBITREADER_H