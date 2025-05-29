//
// Created by Ian Parker on 31/07/2024.
//

#ifndef UFC_UTILS_H
#define UFC_UTILS_H

#include <string>
#include <vector>
#include <algorithm>

namespace UFC
{
std::vector<std::wstring> splitString(std::wstring line, wchar_t splitChar);
std::wstring utf82wstring(const char* str);
std::wstring utf82wstring(const char* str, int length);
std::string wstring2utf8(std::wstring string);

class StringUtils
{
 public:
    static bool shouldTrim(unsigned char ch)
    {
        return std::isspace(ch) || std::iscntrl(ch);
    }

    static void ltrim(std::string &s)
    {
        s.erase(s.begin(), std::ranges::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !shouldTrim(ch);
        }));
    }

    static void rtrim(std::string &s)
    {
        s.erase(std::ranges::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !shouldTrim(ch);
        }).base(), s.end());
    }

    static std::string trim(std::string s)
    {
        rtrim(s);
        ltrim(s);
        return s;
    }
};

};

#endif //UTILS_H
