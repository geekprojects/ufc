//
// Created by Ian Parker on 14/04/2024.
//

#include "navdata.h"

#include <ufc/utils.h>

#include <cstring>

using namespace std;

vector<wstring> NavData::splitLine(wstring line)
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

vector<string> NavData::splitLine(string line)
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

std::wstring NavData::joinToEnd(vector<wstring> parts, int startPos)
{
    std::wstring result;
    for (int i = startPos; i < parts.size(); i++)
    {
        if (!result.empty())
        {
            result += L" ";
        }
        result += parts.at(i);
    }
    return result;
}

std::string NavData::joinToEnd(vector<string> parts, int startPos)
{
    std::string result;
    for (int i = startPos; i < parts.size(); i++)
    {
        if (!result.empty())
        {
            result += " ";
        }
        result += parts.at(i);
    }
    return result;
}

wstring NavData::readLine(FILE* fd)
{
    char lineBuffer[2048];
    fgets(lineBuffer, 2048, fd);

    int len = (int)strnlen(lineBuffer, 2048);
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

    return UFC::utf82wstring(lineBuffer);
}
