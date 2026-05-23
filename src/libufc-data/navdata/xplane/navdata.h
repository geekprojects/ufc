//
// Created by Ian Parker on 14/04/2024.
//

#ifndef NAVDATA_H
#define NAVDATA_H

#include <string>
#include <vector>

#include "ufc/data/navdata.h"

namespace UFC
{

struct NavFile
{
    int fd = -1;
    bool eof = false;
    char* mmapData;
    char* ptr;
    char* end;
    int64_t size;

    static std::shared_ptr<NavFile> open(const std::string &fileName);

    void close();

    std::wstring readLine();
};

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
