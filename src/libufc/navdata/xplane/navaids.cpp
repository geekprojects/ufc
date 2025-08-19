//
// Created by Ian Parker on 14/04/2024.
//

#include "xplanenavdata.h"

#include <ufc/utils.h>

#include <cstring>
#include <regex>

#include "navdata.h"
#include "ufc/data.h"
#include "ufc/flightconnector.h"

using namespace std;
using namespace UFC;


shared_ptr<NavAids> XPlaneNavDataSource::loadNavAids()
{
    auto navData = make_shared<NavAids>();

    loadFixes(navData);
    loadNavAidData(navData);

    return navData;
}

NavDataHeader XPlaneNavDataSource::loadHeader(std::string headerStr)
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
        log(WARN, "Header does not match expected pattern: %s", headerStr.c_str());
    }
    return header;
}

void XPlaneNavDataSource::loadFixes(const shared_ptr<NavAids>& navData)
{
    string fixPath = getFlightConnector()->getConfig().xplanePath + "/Custom Data/earth_fix.dat";

    auto fixData = make_shared<Data>();
    fixData->load(fixPath);
    fixData->readLine();
    string headerStr = fixData->readLine();

    auto header = loadHeader(headerStr);
    if (header.version != 1200)
    {
        log(ERROR, "Unsupported version of earth_fix: version=%d", header.version);
        return;
    }
    navData->setHeader(header);

    while (!fixData->eof())
    {
        string lineStr = fixData->readLine();
        lineStr = StringUtils::trim(lineStr);
        if (lineStr.size() == 0)
        {
            continue;
        }
        auto line = NavDataUtils::splitLine(lineStr);
        if (line.size() < 6)
        {
            continue;
        }

        double lat = atof(line[0].c_str());
        double lng = atof(line[1].c_str());
        string id = line[2];
        //string region = line[3];
        //int type = atoi(line[4].c_str());

        auto navAid = make_shared<NavAid>();
        navAid->setLocation(Coordinate(lat, lng));
        navAid->setId(id);
        navAid->setType(NavAidType::FIX);
        navData->addNavAid(navAid);
    }
}

void XPlaneNavDataSource::loadNavAidData(const std::shared_ptr<NavAids>& navData)
{
    string fixPath = getFlightConnector()->getConfig().xplanePath + "/Custom Data/earth_nav.dat";

    auto fixData = make_shared<Data>();
    fixData->load(fixPath);
    fixData->readLine();
    string headerStr = fixData->readLine();

    auto header = loadHeader(headerStr);
    if (header.version != 1200)
    {
        log(ERROR, "Unsupported version of earth_nav: version=%d", header.version);
        return;
    }
    navData->setHeader(header);

    while (!fixData->eof())
    {
        string lineStr = fixData->readLine();
        StringUtils::trim(lineStr);
        if (lineStr.size() == 0)
        {
            continue;
        }
        auto line = NavDataUtils::splitLine(lineStr);
        if (line.size() < 11)
        {
            continue;
        }

        int type = atoi(line[0].c_str());
        if (!(type == 2 || type == 3 || /* type == 4 || type == 5 || */ type == 12 || type == 13))
        {
            continue;
        }

        NavAidType navAidType = NavAidType::WAY_POINT;
        switch (type)
        {
            case 2: navAidType = NavAidType::NDB; break;
            case 3: navAidType = NavAidType::VOR; break;
            //case 4:
            //case 5: navAidType = NavAidType::LOC; break;
            case 12:
            case 13: navAidType = NavAidType::DME; break;
        }

        double lat = atof(line[1].c_str());
        double lng = atof(line[2].c_str());
        string id = line[7];
        int elevation = atoi(line[3].c_str());
        int frequency = atoi(line[4].c_str());
        // printf("loadNavAids: type=%d -> %d, lat=%0.2f, lng=%0.2f, id=%s\n", type, (int)navAidType, lat, lng, id.c_str());
        auto navAid = make_shared<NavAid>();
        navAid->setLocation(Coordinate(lat, lng));
        navAid->setId(id);
        navAid->setType(navAidType);
        navAid->setElevation(elevation);
        navAid->setFrequency(frequency);
        navData->addNavAid(navAid);
    }
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

    return UFC::utf82wstring(lineBuffer);
}
