//
// Created by Ian Parker on 20/01/2024.
//

#ifndef UFC_STATE_H
#define UFC_STATE_H

#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include <mutex>
#include <memory>
#include <variant>

#include "utils/logger.h"

namespace UFC
{
enum class DataRefType
{
    UNKNOWN,
    FLOAT,
    BOOLEAN,
    INTEGER,
    STRING,
    INT_ARRAY,
};

class AircraftValue
{
    DataRefType m_type = DataRefType::UNKNOWN;
    std::variant<std::monostate, int, float, std::wstring, std::vector<int> > m_value;

public:
    AircraftValue() = default;

    explicit AircraftValue(DataRefType type)
    {
        m_type = type;
        if (m_type == DataRefType::INT_ARRAY)
        {
            std::vector<int> v;
            m_value = v;
        }
    }

    AircraftValue(bool b)
    {
        set(b);
    }

    AircraftValue(int i)
    {
        set(i);
    }

    AircraftValue(float f)
    {
        set(f);
    }

    AircraftValue(std::wstring const &str)
    {
        set(str);
    }

    AircraftValue(std::vector<int> array)
    {
        set(array);
    }

    void set(bool b)
    {
        m_type = DataRefType::BOOLEAN;
        m_value = static_cast<int>(b);
    }

    void set(int i)
    {
        m_type = DataRefType::INTEGER;
        m_value = i;
    }

    void set(float f)
    {
        m_type = DataRefType::FLOAT;
        m_value = f;
    }

    void set(std::wstring const &str)
    {
        m_type = DataRefType::STRING;
        m_value = str;
    }

    void set(std::vector<int> const & array)
    {
        m_type = DataRefType::INT_ARRAY;
        m_value = array;
    }

    void set(AircraftValue const &b)
    {
        m_type = b.m_type;
        m_value = b.m_value;
    }

    [[nodiscard]] DataRefType getType() const
    {
        return m_type;
    }

    [[nodiscard]] int getInt() const
    {
        switch (m_type)
        {
            case DataRefType::BOOLEAN:
            case DataRefType::INTEGER:
                return std::get<int>(m_value);
            case DataRefType::FLOAT:
                return static_cast<int>(std::get<float>(m_value));
            default:
                return 0;
        }
    }

    [[nodiscard]] float getFloat() const
    {
        switch (m_type)
        {
            case DataRefType::BOOLEAN:
            case DataRefType::INTEGER:
                return static_cast<float>(std::get<int>(m_value));
            case DataRefType::FLOAT:
                return std::get<float>(m_value);
            default:
                return 0.0;
        }
    }

    [[nodiscard]] std::wstring getString() const
    {
        switch (m_type)
        {
            case DataRefType::BOOLEAN:
            case DataRefType::INTEGER:
                return std::to_wstring(std::get<int>(m_value));
            case DataRefType::FLOAT:
                return std::to_wstring(std::get<float>(m_value));
            case DataRefType::STRING:
                return std::get<std::wstring>(m_value);
            case DataRefType::INT_ARRAY:
                return L"Array of " + std::to_wstring(std::get<std::vector<int>>(m_value).size()) + L" elements";
            default:
                return L"";
        }
    }

    void setArrayInt(size_t index, int value)
    {
        auto& array = std::get<std::vector<int>>(m_value);
        if (array.size() <= index)
        {
            array.resize(index + 1);
        }
        array[index] = value;
        m_value = array;
        printf("setArrayInt(%lu)=%d, size=%lu\n", index, value, array.size());
    }

    size_t getArraySize()
    {
        switch (m_type)
        {
            case DataRefType::INT_ARRAY:
            {
                auto& array = std::get<std::vector<int>>(m_value);
                return array.size();
            }
            case DataRefType::STRING:
                return std::get<std::wstring>(m_value).size();
            default:
                return 1;
        }
    }

    int getArrayInt(size_t index)
    {
        auto& array = std::get<std::vector<int>>(m_value);
        if (index < 0 || index >= array.size())
        {
            return 0;
        }
        return array[index];
    }

    std::vector<int> getArray() const
    {
        return std::get<std::vector<int>>(m_value);
    }

    bool hasValue() const
    {
        return m_type != DataRefType::UNKNOWN;
    }
};

class AircraftState : public Logger
{
    std::mutex m_mutex;
    std::map<std::string, std::shared_ptr<AircraftValue>, std::less<> > m_valuesByName;

public:
    AircraftState() : Logger("AircraftState")
    {
    }

    std::shared_ptr<AircraftValue> getOrCreateValue(const std::string &dataName);

    std::shared_ptr<AircraftValue> getValue(const std::string &dataName);

    void init();

    bool isSet(const std::string &dataName);

    void set(std::string const &name, bool b)
    {
        getOrCreateValue(name)->set(b);
    }

    void set(std::string const &name, int i)
    {
        getOrCreateValue(name)->set(i);
    }

    void set(std::string const &name, float f)
    {
        getOrCreateValue(name)->set(f);
    }

    void set(std::string const &name, const std::wstring &str)
    {
        getOrCreateValue(name)->set(str);
    }

    void set(std::string const &name, const std::vector<int> &array)
    {
        getOrCreateValue(name)->set(array);
    }

    void set(std::string const &name, AircraftValue const &value)
    {
        getOrCreateValue(name)->set(value);
    }

    float getFloat(const std::string &dataName);

    int getInt(const std::string &dataName);

    std::wstring getString(const std::string &dataName);

    void fmsPrint(int row, std::wstring text, char fg = 'w', char bg = 'b');

    void dump();
};
}

#endif //UFC_STATE_H
