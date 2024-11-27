//
// Created by Ian Parker on 14/04/2024.
//

#ifndef NAVDATA_H
#define NAVDATA_H

#include <string>
#include <vector>

class NavDataUtils
{
 public:
    static std::vector<std::wstring> splitLine(std::wstring line);
    static std::vector<std::string> splitLine(std::string line);
    static std::wstring joinToEnd(std::vector<std::wstring> parts, int startPos);
    static std::string joinToEnd(std::vector<std::string> parts, int startPos);
    static std::wstring readLine(FILE* fd);

    static bool shouldTrim(unsigned char ch)
    {
        return std::isspace(ch) || std::iscntrl(ch);
    }

    static void ltrim(std::string &s)
    {
        s.erase(s.begin(), std::ranges::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !NavDataUtils::shouldTrim(ch);
        }));
    }

    static void rtrim(std::string &s)
    {
        s.erase(std::ranges::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !NavDataUtils::shouldTrim(ch);
        }).base(), s.end());
    }

    static void trim(std::string &s)
    {
        rtrim(s);
        ltrim(s);
    }
};

#endif //NAVDATA_H
