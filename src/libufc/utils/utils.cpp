//
// Created by Ian Parker on 31/07/2024.
//

#include <ufc/utils.h>

#include "utf8.h"

#include <cstring>

using namespace std;
using namespace UFC;

vector<wstring> UFC::splitString(wstring line, wchar_t splitChar)
{
    vector<wstring> parts;

    while (!line.empty())
    {
        size_t pos = line.find(splitChar);
        if (pos == wstring::npos)
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

wstring UFC::utf82wstring(const char* start)
{
    if (start == NULL)
    {
        return L"";
    }

    int length = strlen(start);
    if (length <= 0)
    {
        return L"";
    }

    return utf82wstring(start, length);
}

wstring UFC::utf82wstring(const char* start, int length)
{
    if (start == NULL)
    {
        return L"";
    }

    const char* pos = start;
    const char* end = pos + length;
    wstring result = L"";

    while (pos < end)
    {
        wchar_t cur;
        try
        {
            cur = utf8::next(pos, end);
        }
        catch (...)
        {
            printf("UFC::utf82wstring: Invalid UTF-8 character\n");
            cur = '?';
            pos++;
        }
        if (cur == 0)
        {
            break;
        }
        result += cur;
    }
    return result;
}

string UFC::wstring2utf8(wstring str)
{
    string result = "";

    unsigned int pos;
    for (pos = 0; pos < str.length(); pos++)
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

