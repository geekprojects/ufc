#ifndef UFC_CORE_DATA_H_
#define UFC_CORE_DATA_H_

#include <string>
#include <cstdint>

#include <ufc/logger.h>

namespace UFC {

enum Endian
{
    NONE,
    BIG,
    LITTLE
};

class Data final : public Logger
{
 protected:
    char* m_data = nullptr;
    char* m_end = nullptr;
    char* m_pos = nullptr;
    unsigned int m_length = 0;
    unsigned int m_bufferSize = 0;
    bool m_isSub = false;
    Endian m_endian = NONE;

 public:
    Data();
    Data(char* data, unsigned int length);
    explicit Data(unsigned int length);

    ~Data() override;

    static Data* copy(char* data, unsigned int length);

    bool load(const std::string &filename);

    void setEndian(Endian endian);
    void setSwap(bool swap)
    {
        Endian endian = getMachineEndian();
        if (swap)
        {
            if (endian == LITTLE)
            {
                m_endian = BIG;
            }
            else
            {
                m_endian = LITTLE;
            }
        }
        else
        {
            m_endian = endian;
        }
    }
    [[nodiscard]] bool getSwap() const { return mustSwap(m_endian); }

    void clear();
    void reset();

    [[nodiscard]] char* getData() const { return m_data; }
    bool eof();
    uint32_t pos();
    void setPos(uint32_t pos);
    void skip(unsigned int amount);
    [[nodiscard]] uint32_t getLength() const { return m_length; }
    [[nodiscard]] uint32_t getRemaining() const { return m_end - m_pos; }
    [[nodiscard]] char* posPointer() const { return m_pos; }

    uint8_t peek8();
    uint32_t peek32();

    uint8_t read8();
    uint16_t read16();
    uint32_t read32();
    uint64_t read64();
    float readFloat();
    double readDouble();
    uint64_t readULEB128();

    uint16_t read16(Endian endian);
    uint32_t read32(Endian endian);
    uint64_t read64(Endian endian);

    char* readStruct(size_t len);

    std::string cstr();
    std::string readString(int max);
    std::string readLine();
    std::wstring readWLine();

    bool append8(uint8_t data);
    bool append16(uint16_t data);
    bool append32(uint32_t data);
    bool append64(uint64_t data);
    bool appendFloat(float data);
    bool appendDouble(double data);
    bool append(uint8_t* data, int length);
    bool appendString(std::string str);
    bool appendString(const std::wstring &str);

    bool write(const std::string &file);
    bool write(const std::string &file, uint32_t pos, uint32_t length);
    bool write(FILE* fp, uint32_t pos, uint32_t length);

    Data* getSubData(uint32_t pos, uint32_t length);

    static Endian getMachineEndian()
    {
        union
        {
            uint32_t i;
            char c[4];
        } b = {0x01020304};

        if (b.c[0] == 1)
        {
            return BIG;
        }
        return LITTLE;
    }

    static bool mustSwap(const Endian endian)
    {
        return endian != NONE && endian != getMachineEndian();
    }
};

}

#endif
