//
// Created by Ian Parker on 14/04/2024.
//

#ifndef NAVDATA_H
#define NAVDATA_H

#include <string>
#include <vector>

namespace UFC
{

class NavDataUtils
{
public:
    static std::vector<std::wstring> splitLine(std::wstring line);
    static std::vector<std::string> splitLine(std::string line);
    static std::wstring joinToEnd(std::vector<std::wstring> parts, int startPos);
    static std::string joinToEnd(std::vector<std::string> parts, int startPos);
    static std::wstring readLine(FILE* fd);

    static NavDataHeader parseHeader(std::string headerStr);
};
}

#endif //NAVDATA_H
