//
// Created by Ian Parker on 14/04/2024.
//

#include "navdata.h"
#include "ufc/navdata.h"
#include "../xplane.h"

#include <ufc/utils.h>

#include <cstring>

#include "ufc/flightconnector.h"

using namespace std;
using namespace UFC;


shared_ptr<NavData> XPlaneDataSource::loadNavData()
{
    auto navData = make_shared<NavData>();

    loadFixes(navData);
    loadNavAids(navData);

    return navData;
}

void XPlaneDataSource::loadFixes(const shared_ptr<NavData>& navData) const
{
    string fixPath = m_flightConnector->getConfig().xplanePath + "/Custom Data/earth_fix.dat";

    auto fixData = make_shared<Data>();
    fixData->load(fixPath);
    fixData->readLine();
    fixData->readLine(); // TODO: Extract AIRAC cycle
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
        navData->addNavAid(navAid);
    }
}

void XPlaneDataSource::loadNavAids(const std::shared_ptr<NavData>& navData) const
{
    string fixPath = m_flightConnector->getConfig().xplanePath + "/Custom Data/earth_nav.dat";

    auto fixData = make_shared<Data>();
    fixData->load(fixPath);
    fixData->readLine();
    fixData->readLine(); // TODO: Extract AIRAC cycle
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
        //double elevation = atof(line[3].c_str());
        // printf("loadNavAids: type=%d -> %d, lat=%0.2f, lng=%0.2f, id=%s\n", type, (int)navAidType, lat, lng, id.c_str());
        auto navAid = make_shared<NavAid>();
        navAid->setLocation(Coordinate(lat, lng));
        navAid->setId(id);
        navAid->setType(navAidType);
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
