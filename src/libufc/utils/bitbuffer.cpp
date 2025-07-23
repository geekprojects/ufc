#include <ufc/bitbuffer.h>

BitBuffer::BitBuffer(uint8_t* buffer, int length)
{
    for (int i = 0; i < length; i++)
    {
        m_buffer.push_back(buffer[i]);
    }
}

void BitBuffer::flushBits()
{
    if (m_nextBit > 0)
    {
        //printf("BitBuffer::flushBits: Flushing!\n");
        m_buffer.push_back(m_currentByte);
        m_currentByte = 0;
        m_nextBit = 0;
    }
}

void BitBuffer::appendBit(bool bit)
{
    //printf("BitBuffer::appendBit: bit=%d, currentByte=%lu, nextBit=%d\n", bit, m_buffer.size() + 1, m_nextBit);
    m_currentByte |= (uint8_t)bit << (m_nextBit);
    m_nextBit++;
    if (m_nextBit >= 8)
    {
        flushBits();
    }
}

void BitBuffer::appendByte(uint8_t byte)
{
    //printf("BitBuffer::appendByte: byte=%d, currentByte=%lu\n", byte, m_buffer.size() + 1);
    if (m_nextBit > 0)
    {
        //printf("BitBuffer::appendByte:  -> Adding by bits\n");
        for (int i = 0; i < 8; i++)
        {
            appendBit((byte >> (7 - i)) & 1);
        }
    }
    else
    {
        m_buffer.push_back(byte);
    }
}

bool BitBuffer::readBit()
{
    uint8_t byte = m_buffer.at(m_readPos);
    bool result = (byte >> (m_nextBit)) & 1;
    m_nextBit++;
    if (m_nextBit >= 8)
    {
        m_nextBit = 0;
        m_readPos++;
    }
    return result;
}

uint8_t BitBuffer::readByte()
{
    uint8_t value = 0;
    for (int i = 0; i < 8; i++)
    {
        value |= readBit() << (7 - i);
    }
    return value;
}

