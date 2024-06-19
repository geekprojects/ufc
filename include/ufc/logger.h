/*
 * libgeek - The GeekProjects utility suite
 * Copyright (C) 2014, 2015, 2016 GeekProjects.com
 *
 * This file is part of libgeek.
 *
 * libgeek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libgeek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libgeek.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UFC_CORE_LOGGER_H_
#define UFC_CORE_LOGGER_H_

#include <string>

namespace UFC {

enum LoggerLevel_t
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger
{
 private:
    std::string m_name;
    int m_depth;

 public:
    explicit Logger(const std::string &name);
    explicit Logger(const std::wstring &name);
    virtual ~Logger();

    void setLoggerName(const std::string &name);
    void setLoggerName(const std::wstring &name);

    void log(LoggerLevel_t level, const char* format, ...);
    void logv(LoggerLevel_t level, const char* format, va_list ap);
    void debug(const char* format, ...);
    void error(const char* format, ...);

    // Not thread safe!
    void pushDepth() { m_depth++; }
    void popDepth()  { m_depth--; }
};

};

#endif
