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
};

class AircraftValue
{
    int m_index = -1;
    DataRefType m_type = DataRefType::UNKNOWN;
    union
    {
        int i;
        float f;
    } m_value;
    std::string m_string;

 public:
    AircraftValue()
    {
    }

    explicit AircraftValue(int idx)
    {
        m_value.i = 0;
        m_index = idx;
    }

    [[nodiscard]] int getIndex() const
    {
        return m_index;
    }

    void set(bool b)
    {
        m_type = DataRefType::BOOLEAN;
        m_value.i = b;
    }

    void set(int i)
    {
        m_type = DataRefType::INTEGER;
        m_value.i = i;
    }

    void set(float f)
    {
        m_type = DataRefType::FLOAT;
        m_value.f = f;
    }

    void set(std::string const& str)
    {
        m_type = DataRefType::STRING;
        m_string = str;
    }

    void set(AircraftValue const & b)
    {
        m_type = b.m_type;
        if (m_type == DataRefType::STRING)
        {
            m_string = b.m_string;
        }
        else
        {
            m_value = b.m_value;
        }
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
                return m_value.i;
            case DataRefType::FLOAT:
                return (int)m_value.f;
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
                return (float)m_value.i;
            case DataRefType::FLOAT:
                return m_value.f;
            default:
                return 0.0;
        }
    }

    [[nodiscard]] std::string getString() const
    {
        switch (m_type)
        {
            case DataRefType::BOOLEAN:
            case DataRefType::INTEGER:
                return std::to_string(m_value.i);
            case DataRefType::FLOAT:
                return std::to_string(m_value.f);
            case DataRefType::STRING:
                return m_string;
            default:
                return "";
        }
    }
};

class AircraftState : public Logger
{
    int m_nextIndex = 1;
    std::mutex m_mutex;

    std::map<std::string, std::shared_ptr<AircraftValue>, std::less<>> valuesByName;
    std::map<int, std::shared_ptr<AircraftValue>> valuesByIndex;

 public:
    AircraftState() : Logger("AircraftState") {}

    int getIndex(const std::string& name);
    std::shared_ptr<AircraftValue> getOrCreateValue(const std::string &dataName);
    std::shared_ptr<AircraftValue> getValue(const std::string &dataName);

    void init();

    bool isSet(const std::string &dataName)
    {
        return getValue(dataName) != nullptr;
    }

    void set(int idx, bool b)
    {
        valuesByIndex[idx]->set(b);
    }

    void set(int idx, int i)
    {
        valuesByIndex[idx]->set(i);
    }

    void set(int idx, float f)
    {
        valuesByIndex[idx]->set(f);
    }

    void set(int idx, const std::string& str)
    {
        valuesByIndex[idx]->set(str);
    }

    void set(std::string const& name, bool b)
    {
        getOrCreateValue(name)->set(b);
    }

    void set(std::string const& name, int i)
    {
        getOrCreateValue(name)->set(i);
    }

    void set(std::string const& name, float f)
    {
        getOrCreateValue(name)->set(f);
    }

    void set(std::string const& name, const std::string& str)
    {
        getOrCreateValue(name)->set(str);
    }

    void set(std::string const& name, AircraftValue const& value)
    {
        getOrCreateValue(name)->set(value);
    }

    bool hasValue(const std::string& dataName);

    float getFloat(const std::string& dataName);
    int getInt(const std::string& dataName);
    std::string getString(const std::string& dataName);

    void dump();
};

}

#endif //XPFD_STATE_H
