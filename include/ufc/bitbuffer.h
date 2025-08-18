//
// Created by Ian Parker on 21/07/2025.
//

#ifndef BITBUFFER_H
#define BITBUFFER_H

#include <vector>
#include <cstdint>

class BitBuffer
{
    int m_nextBit = 0;
    uint8_t m_currentByte = 0;
    std::vector<uint8_t> m_buffer;

    int m_readPos = 0;

 public:
    BitBuffer() = default;
    BitBuffer(uint8_t* buffer, int length);

    void appendBit(bool bit);
    void appendByte(uint8_t bit);
    [[nodiscard]] const uint8_t* data() const { return m_buffer.data(); }
    [[nodiscard]] size_t size() const { return m_buffer.size(); }

    void flushBits();

    bool readBit();

    uint8_t readByte();
};

#endif //BITBUFFER_H
