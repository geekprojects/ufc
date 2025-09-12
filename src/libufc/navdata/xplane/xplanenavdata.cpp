//
// Created by Ian Parker on 19/08/2025.
//

#include "xplanenavdata.h"
#include "navdata.h"

#include <ufc/utils.h>

#include <regex>
#include <cstring>

using namespace std;
using namespace UFC;

XPlaneNavDataSource::XPlaneNavDataSource(FlightConnector* flightConnector) :
    NavDataSource(flightConnector, "XPlaneNavDataSource")
{
}

std::shared_ptr<Airports> XPlaneNavDataSource::getAirports()
{
    if (m_airports == nullptr)
    {
        auto airports = std::make_shared<XPlaneAirports>(this);
        if (!airports->init())
        {
            return nullptr;
        }
        m_airports = airports;
    }
    return m_airports;
}

std::shared_ptr<NavAids> XPlaneNavDataSource::getNavAids()
{
    if (m_navAids == nullptr)
    {
        auto navAids = std::make_shared<XPlaneNavAids>(this);
        if (!navAids->init())
        {
            return nullptr;
        }
        m_navAids = navAids;
    }
    return m_navAids;
}

NavDataHeader NavDataUtils::parseHeader(std::string headerStr)
{
    NavDataHeader header;
    regex headerRegex(
        "([0-9]+) Version - data cycle ([0-9]+), build ([0-9]+), metadata ([A-Za-z0-9]+). (.*)",
        regex_constants::ECMAScript | std::regex_constants::icase);

    auto results = sregex_iterator(headerStr.begin(), headerStr.end(), headerRegex);
    auto results_end = std::sregex_iterator();
    if (!results->empty())
    {
        const std::smatch& match = *results;
        header.version = atoi(match[1].str().c_str());
        header.cycle = atoi(match[2].str().c_str());
        header.build = atoi(match[3].str().c_str());
        header.type = match[4];
        header.copyright = match[5];
    }
    else
    {
        fprintf(stderr, "Header does not match expected pattern: %s", headerStr.c_str());
    }
    return header;
}

vector<wstring> NavDataUtils::splitLine(wstring line)
{
    vector<wstring> parts;

    while (!line.empty())
    {
        auto pos = line.find(' ');
        if (pos == string::npos)
        {
            pos = line.find('\t');
        }
        if (pos == string::npos)
        {
            pos = line.length();
            if (pos == 0)
            {
                break;
            }
        }
        if (pos >= 1)
        {
            wstring part = line.substr(0, pos);
            parts.push_back(part);
        }
        if (pos == line.length())
        {
            break;
        }
        line = line.substr(pos + 1);
    }

    return parts;
}

vector<string> NavDataUtils::splitLine(string line)
{
    vector<string> parts;

    while (!line.empty())
    {
        auto pos = line.find(' ');
        if (pos == string::npos)
        {
            pos = line.find('\t');
        }
        if (pos == string::npos)
        {
            pos = line.length();
            if (pos == 0)
            {
                break;
            }
        }
        if (pos >= 1)
        {
            string part = line.substr(0, pos);
            parts.push_back(part);
        }
        if (pos == line.length())
        {
            break;
        }
        line = line.substr(pos + 1);
    }

    return parts;
}

std::wstring NavDataUtils::joinToEnd(vector<wstring> parts, int startPos)
{
    std::wstring result;
    for (unsigned int i = startPos; i < parts.size(); i++)
    {
        if (!result.empty())
        {
            result += L" ";
        }
        result += parts.at(i);
    }
    return result;
}

std::string NavDataUtils::joinToEnd(vector<string> parts, int startPos)
{
    std::string result;
    for (unsigned int i = startPos; i < parts.size(); i++)
    {
        if (!result.empty())
        {
            result += " ";
        }
        result += parts.at(i);
    }
    return result;
}

wstring NavDataUtils::readLine(FILE* fd)
{
    char lineBuffer[2048];
    char const* res = fgets(lineBuffer, 2048, fd);
    if (res == nullptr)
    {
        return L"";
    }

    auto len = (int)strnlen(lineBuffer, 2048);
    if (len == 0)
    {
        return L"";
    }
    while (len > 0 && (lineBuffer[len-1] == '\r' || lineBuffer[len-1] == '\n'))
    {
        lineBuffer[len - 1] = 0;
        len--;
    }

    if (len <= 0)
    {
        return L"";
    }

    return utf82wstring(lineBuffer);
}
