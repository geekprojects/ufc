//
// Created by Ian Parker on 31/07/2024.
//

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace UFC
{
std::vector<std::wstring> splitString(std::wstring line, wchar_t splitChar);
std::wstring utf82wstring(const char* str);
std::wstring utf82wstring(const char* str, int length);
std::string wstring2utf8(std::wstring string);
};

#endif //UTILS_H
