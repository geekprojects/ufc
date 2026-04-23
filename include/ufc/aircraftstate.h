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
};

class AircraftValue
{
    DataRefType m_type = DataRefType::UNKNOWN;
    std::variant<std::monostate, int, float, std::string> m_value;

 public:
    AircraftValue()
    {
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

    bool hasValue() const
    {
        return m_type != DataRefType::UNKNOWN;
    }
};

class AircraftState : public Logger
{
    std::mutex m_mutex;
    std::map<std::string, std::shared_ptr<AircraftValue>, std::less<>> m_valuesByName;

 public:
    AircraftState() : Logger("AircraftState") {}

    std::shared_ptr<AircraftValue> getOrCreateValue(const std::string &dataName);
    std::shared_ptr<AircraftValue> getValue(const std::string &dataName);

    void init();

    bool isSet(const std::string &dataName);

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

    float getFloat(const std::string& dataName);
    int getInt(const std::string& dataName);
    std::string getString(const std::string& dataName);

    void dump();
};

}

#endif //UFC_STATE_H
