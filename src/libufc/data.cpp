#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <zlib.h>
#include <cassert>
#include "utf8.h"

#include <ufc/data.h>

using namespace std;
using namespace UFC;

#define CHUNK 16384

Data::Data()
    : Logger("Data")
{
    reset();
}

Data::Data(unsigned int length)
    : Logger("Data")
{
    m_data = new char[length];
    m_length = length;
    m_bufferSize = length;

    reset();
}

Data::Data(char* data, unsigned int length)
    : Logger("Data")
{
    m_data = data;
    m_length = length;
    m_bufferSize = length;

    reset();
}

Data::~Data()
{
    clear();
}

Data* Data::copy(char* data, unsigned int length)
{
    auto newData = new char[length];
    memcpy(newData, data, length);
    return new Data(newData, length);
}

void Data::setEndian(Endian endian)
{
    m_endian = endian;
}

bool Data::load(const string& filename)
{
    clear();
    log(DEBUG, "load: Loading: %s", filename.c_str());
    FILE* file = fopen(filename.c_str(), "r");
    if (file == nullptr)
    {
        log(ERROR, "load: Failed to load: %s", filename.c_str());
        return false;
    }

    fseek(file, 0, SEEK_END);
    m_length = ftell(file);
    m_bufferSize = m_length;
    fseek(file, 0, SEEK_SET);

    m_data = (char*) malloc(m_length);
    size_t res = fread(m_data, m_length, 1, file);
    fclose(file);

    reset();
    return (res == 1);
}

void Data::clear()
{
    if (m_data != nullptr && !m_isSub)
    {
        free(m_data);
        m_data = nullptr;
    }
    m_pos = nullptr;
    m_end = nullptr;
    m_length = 0;
    m_bufferSize = 0;
}

void Data::reset()
{
    m_pos = m_data;
    m_end = m_data + m_length;
}

bool Data::eof()
{
    return m_pos >= m_end;
}

uint32_t Data::pos()
{
    return (m_pos - m_data);
}

void Data::setPos(uint32_t pos)
{
    m_pos = m_data + pos;
}

void Data::skip(unsigned int amount)
{
    m_pos += amount;
}

uint8_t Data::read8()
{
    return *(m_pos++);
}

uint16_t Data::read16()
{
    return read16(m_endian);
}

uint16_t Data::read16(Endian endian)
{
    uint16_t res;
    if (!mustSwap(endian))
    {
        res = read8();
        res |= read8() << 8;
    }
    else
    {
        res = read8() << 8;
        res |= read8() << 0;
    }
    return res;
}

uint32_t Data::read32()
{
    return read32(m_endian);
}

uint32_t Data::read32(Endian endian)
{
    uint32_t res;
    if (!mustSwap(endian))
    {
        res = read8();
        res |= read8() << 8;
        res |= read8() << 16;
        res |= read8() << 24;
    }
    else
    {
        res = read8() << 24;
        res |= read8() << 16;
        res |= read8() << 8;
        res |= read8() << 0;
    }
    return res;
}

uint64_t Data::read64()
{
    return read64(m_endian);
}

uint64_t Data::read64(Endian endian)
{
    uint64_t res;
    if (!mustSwap(endian))
    {
        res = (uint64_t) read32(endian);
        res |= (uint64_t) read32(endian) << 32;
    }
    else
    {
        res = (uint64_t) read32(endian) << 32;
        res |= (uint64_t) read32(endian) << 0;
    }
    return res;
}

uint8_t Data::peek8()
{
    return *(m_pos);
}

uint32_t Data::peek32()
{
    return *((uint32_t*) m_pos);
}

float Data::readFloat()
{
    float res;
    res = *((float*) m_pos);
    m_pos += sizeof(float);
    return res;
}

double Data::readDouble()
{
    double res;
    res = *((double*) m_pos);
    m_pos += sizeof(double);
    return res;
}

uint64_t Data::readULEB128()
{
    uint64_t result = 0;
    int bit = 0;

    while (!eof())
    {
        uint8_t b = read8();
        result |= (((uint64_t) (b & 0x7f)) << bit);
        bit += 7;

        if (!(b & 0x80))
        {
            break;
        }
    }

    return result;
}

char* Data::readStruct(size_t len)
{
    char* pos = m_pos;
    m_pos += len;
    return pos;
}

string Data::cstr()
{
    string str;
    while (!eof())
    {
        char c = (char) read8();
        if (c == 0)
        {
            break;
        }
        str += c;
    }
    return str;
}

string Data::readString(int len)
{
    string str;
    bool eol = false;
    int i;
    for (i = 0; i < len; i++)
    {
        char c = (char) read8();
        if (!eol)
        {
            if (c == 0)
            {
                eol = true;
            }
            else
            {
                str += c;
            }
        }
    }
    return str;
}

string Data::readLine()
{
    string line;
    while (!eof())
    {
        char c = (char) read8();
        if (c == '\r')
        {
            c = (char) read8();
        }
        if (c == '\n')
        {
            break;
        }
        line += c;
    }
    return line;
}

bool Data::append8(uint8_t data)
{
    return append(&data, 1);
}

bool Data::append16(uint16_t data)
{
    if (!mustSwap(m_endian))
    {
        append8(data >> 0);
        append8(data >> 8);
    }
    else
    {
        append8(data >> 8);
        append8(data >> 0);
    }
    return true;
}

bool Data::append32(uint32_t data)
{
    if (!mustSwap(m_endian))
    {
        append8(data >> 0);
        append8(data >> 8);
        append8(data >> 16);
        append8(data >> 24);
    }
    else
    {
        append8(data >> 24);
        append8(data >> 16);
        append8(data >> 8);
        append8(data >> 0);
    }

    return true;
}

bool Data::append64(uint64_t data)
{
    if (!mustSwap(m_endian))
    {
        append32(data << 0);
        append32(data << 32);
    }
    else
    {
        append32(data << 32);
        append32(data << 0);
    }
    return true;
}

bool Data::appendFloat(float data)
{
    return append((uint8_t*) &data, sizeof(float));
}

bool Data::appendDouble(double data)
{
    return append((uint8_t*) &data, sizeof(double));
}

bool Data::append(uint8_t* data, int length)
{
    const int remaining = m_bufferSize - m_length;
    if (remaining < length)
    {
        int grow;
        if (length < 32)
        {
            grow = 64;
        }
        else if (length < 8 * 1024)
        {
            grow = length * 2;
        }
        else
        {
            grow = length;
        }

        m_bufferSize += grow;

        m_data = static_cast<char*>(realloc(m_data, m_bufferSize));

        m_pos = m_data;
        m_end = m_data + m_length;
    }

    memcpy(m_end, data, length);
    m_end += length;
    m_length += length;

    return true;
}

bool Data::appendString(string str)
{
    return append((uint8_t*) str.c_str(), str.length());
}

static string wstring2utf8(wstring str)
{
    string result;

    for (unsigned int pos = 0; pos < str.length(); pos++)
    {
        wchar_t c = str.at(pos);
        char buffer[6] = {0, 0, 0, 0, 0, 0};

        char* end;
        try
        {
            end = utf8::append(c, buffer);
        }
        catch (...)
        {
            result += '?';
            continue;
        }

        char* p;
        for (p = buffer; p < end; p++)
        {
            result += *p;
        }
    }
    return result;
}

bool Data::appendString(const wstring& str)
{
    return appendString(wstring2utf8(str));
}

bool Data::write(const std::string& file)
{
    return write(file, 0, m_length);
}

bool Data::write(const std::string& file, const uint32_t pos, const uint32_t length)
{
    FILE* fp = fopen(file.c_str(), "w");
    if (fp == nullptr)
    {
        return false;
    }

    const bool res = write(fp, pos, length);

    fclose(fp);
    return res;
}

bool Data::write(FILE* fp, uint32_t pos, uint32_t length)
{
    unsigned int res;
    res = fwrite(m_data + pos, length, 1, fp);
    return res == length;
}

Data* Data::getSubData(uint32_t pos, uint32_t length)
{
    Data* data = new Data(m_data + pos, length);
    data->m_isSub = true;
    return data;
}
