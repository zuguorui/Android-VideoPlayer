//
// Created by WangXi on 2025/11/6.
//

#include "BufferBitReader.h"
#include "Log.h"

using namespace std;

#define TAG "BufferBitReader"

BufferBitReader::BufferBitReader(std::vector<uint8_t> &vec) {
    buffer = vec;
}

BufferBitReader::BufferBitReader(uint8_t *buffer, int size) {
    for (int i = 0; i < size; i++) {
        this->buffer.push_back(buffer[i]);
    }
}

bool BufferBitReader::byteAligned() {
    return bitPosInBuffer % 8 == 0;
}

bool BufferBitReader::moreBitInByteStream() {
    return bitPosInBuffer < buffer.size() * 8;
}

bool BufferBitReader::hasRemainBytes(int n) {
    int64_t alignedBytePos = bitPosInBuffer / 8;
    // 如果没对齐，就将字节数加1，当前所在中途字节算已读取字节
    if (bitPosInBuffer % 8 != 0) {
        alignedBytePos += 1;
    }
    return buffer.size() - alignedBytePos >= n;
}

void BufferBitReader::reset() {
    bitPosInBuffer = 0;
}

size_t BufferBitReader::getSize() {
    return buffer.size();
}

int64_t BufferBitReader::currentBitPosition() {
    return bitPosInBuffer;
}


uint32_t BufferBitReader::readBits(int bits) {
    if (bitPosInBuffer + bits > buffer.size() * 8) {
        return 0;
    }

    uint32_t ret = 0;
    for (int i = 0; i < bits; i++) {
        ret = ret << 1;
        uint32_t bit = (buffer[bitPosInBuffer / 8] >> (7 - (bitPosInBuffer % 8))) & 1;
        ret = ret | bit;
        bitPosInBuffer++;
    }
    return ret;
}

uint32_t BufferBitReader::nextBits(int bits) {
    if (bitPosInBuffer + bits > buffer.size() * 8) {
        return 0;
    }
    uint32_t ret = 0;
    for (int i = 0; i < bits; i++) {
        ret = ret << 1;
        uint32_t bit = (buffer[(bitPosInBuffer + i) / 8] >> (7 - ((bitPosInBuffer + i) % 8))) & 1;
        ret = ret | bit;
    }
    return ret;
}

uint8_t BufferBitReader::readByte() {
    if (!byteAligned()) {
        LOGE(TAG, "Byte not aligned");
        return 0;
    }
    if (bitPosInBuffer + 8 > buffer.size() * 8) {
        return 0;
    }
    uint8_t byte = buffer[bitPosInBuffer / 8];
    bitPosInBuffer += 8;
    return byte;
}

uint8_t BufferBitReader::nextByte() {
    if (!byteAligned()) {
        LOGE(TAG, "Byte not aligned");
        return 0;
    }
    if (bitPosInBuffer + 8 > buffer.size() * 8) {
        return 0;
    }
    uint8_t byte = buffer[bitPosInBuffer / 8];
    return byte;
}

uint32_t BufferBitReader::b(int bits) {
    if (bits <= 0 || bits > 32) {
        LOGE(TAG, "i(n): bits must be 1~32, current=%d", bits);
        return 0;
    }
    return readBits(bits);
}

int32_t BufferBitReader::i(int bits) {
    // 1. 校验输入合法性（n 必须是 1~32 比特，H.264 无 0 比特或超 32 比特的有符号整数）
    if (bits <= 0 || bits > 32) {
        LOGE(TAG, "i(n): bits must be 1~32, current=%d", bits);
        return 0;
    }
    // 2. 读取 n 比特原始无符号数据
    uint32_t raw = readBits(bits);
    // 3. 计算符号位掩码（第 n-1 位，1 表示负数，0 表示正数）
    uint32_t sign_mask = 1U << (bits - 1);

    // 4. 补码转换逻辑
    if ((raw & sign_mask) == 0) {
        // 正数：直接返回无符号值（强制转换为 int32_t，无溢出风险）
        return static_cast<int32_t>(raw);
    } else {
        // 负数：原始值 - 2^bits（还原二进制补码的真实值）
        // 注意：2^bits 需用 uint64_t 避免 bits=32 时溢出（1U<<32 会溢出 32 位无符号）
        uint64_t full_range = 1ULL << bits;
        return static_cast<int32_t>(raw - full_range);
    }
}

uint32_t BufferBitReader::f(int bits) {
    return b(bits);
}

uint32_t BufferBitReader::u(int bits) {
    return b(bits);
}

uint32_t BufferBitReader::ue() {
    int leadingZeroBits = 0;
    uint32_t bit = 0;
    while (moreBitInByteStream() && leadingZeroBits < 30) {
        bit = readBits(1);
        if (bit != 0) {
            break;
        }
        leadingZeroBits++;
    }

    if (!moreBitInByteStream()) {
        LOGE(TAG, "没有更多数据");
        return 0;
    }
    if (leadingZeroBits >= 30) {
        LOGE(TAG, "前导0个数过多");
        return 0;
    }

    uint32_t a = readBits(leadingZeroBits);

    uint32_t codeNum = (1U << leadingZeroBits) - 1 + a;

    return codeNum;
}


int32_t BufferBitReader::se() {
    // 步骤1：先调用 ue() 解析得到 codeNum
    uint32_t codeNum = ue();

    // 步骤2：按标准规则转换为有符号整数
    LOGD("BufferBitReader::se()", "codeNum=%d", codeNum);
    if (codeNum % 2 == 0) {
        // 偶数 → 负数：codeNum / 2
        return -static_cast<int32_t>(codeNum / 2);
    } else {
        // 奇数 → 正数：-(codeNum + 1) / 2
        return static_cast<int32_t>((codeNum + 1) / 2);
    }
}

void BufferBitReader::alignToNextByte() {
    if (bitPosInBuffer % 8 != 0) {
        bitPosInBuffer = (bitPosInBuffer / 8 + 1) * 8;
    }
}












