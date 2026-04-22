//
// Created by Ian Parker on 20/01/2024.
//
#include <variant>

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
    std::variant<std::monostate, int, float, std::string> m_value;

 public:
    AircraftValue()
    {
    }

    explicit AircraftValue(int idx)
    {
        m_value = 0;
        m_index = idx;
    }

    [[nodiscard]] int getIndex() const
    {
        return m_index;
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

    void set(std::string const& str)
    {
        m_type = DataRefType::STRING;
        m_value = str;
    }

    void set(AircraftValue const & b)
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
                return (int)std::get<float>(m_value);
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
                return (float)std::get<int>(m_value);
            case DataRefType::FLOAT:
                return std::get<float>(m_value);
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
                return std::to_string(std::get<int>(m_value));
            case DataRefType::FLOAT:
                return std::to_string(std::get<float>(m_value));
            case DataRefType::STRING:
                return std::get<std::string>(m_value);
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
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = valuesByIndex.find(idx);
        if (it != valuesByIndex.end() && it->second)
        {
            it->second->set(b);
        }
    }

    void set(int idx, int i)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = valuesByIndex.find(idx);
        if (it != valuesByIndex.end() && it->second)
        {
            it->second->set(i);
        }
    }

    void set(int idx, float f)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = valuesByIndex.find(idx);
        if (it != valuesByIndex.end() && it->second)
        {
            it->second->set(f);
        }
    }

    void set(int idx, const std::string& str)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = valuesByIndex.find(idx);
        if (it != valuesByIndex.end() && it->second)
        {
            it->second->set(str);
        }
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

#endif //UFC_STATE_H
