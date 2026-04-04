//
// Created by Ian Parker on 30/03/2026.
//

#include "ufc/data/fpformat.h"

#include "ufc/utils/utils.h"
using namespace std;
using namespace UFC;

std::vector<std::vector<std::string>> FlightPlanFormat::readTextFile(std::string filename, bool split)
{
    vector<vector<string>> result;
    FILE* fd = fopen(filename.c_str(), "r");
    if (fd == nullptr)
    {
        printf("readTextFile: Failed to open file %s\n", filename.c_str());
        return result;
    }

    char lineBuffer[2048];
    while (fgets(lineBuffer, 2048, fd) != nullptr)
    {
        int len = strlen(lineBuffer);
        while (len > 0 && (lineBuffer[len-1] == '\r' || lineBuffer[len-1] == '\n'))
        {
            lineBuffer[len - 1] = 0;
            len--;
        }
        if (split)
        {
            result.push_back(splitString(lineBuffer, ' '));
        }
        else
        {
            result.push_back({lineBuffer});
        }
    }
    fclose(fd);
    return result;
}

shared_ptr<FlightPlan> FlightPlanFormat::loadFile(string fileName)
{
    return nullptr;
}
