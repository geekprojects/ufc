//
// Created by Ian Parker on 19/05/2025.
//

#include "ufc/data/simbriefjson.h"

#include <curl/curl.h>

using namespace std;
using namespace nlohmann;
using namespace UFC;

struct Payload
{
    char* response = nullptr;
    size_t size = 0;
};

static size_t curlCallback(char const* data, const size_t size, size_t nmemb, void* clientp)
{
    size_t realSize = size * nmemb;
    auto mem = (Payload*)clientp;

    auto ptr = (char*)realloc(mem->response, mem->size + realSize + 1);
    if (!ptr)
    {
        return 0;  /* out of memory */
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realSize);
    mem->size += realSize;
    mem->response[mem->size] = 0;

    return realSize;
}

SimbriefJson::SimbriefJson(NavDataSource* navDataSource)
    : FlightPlanFormat(navDataSource, "SimbriefJson")
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

/*
bool SimbriefJson::check(string filename, const std::vector<std::vector<std::string>>& file)
{
    if (file.at(0).at(0) != "{")
    {
        return false;
    }

    FILE* fd = fopen(filename.c_str(), "r");
    auto json = json::parse(fd);
    fclose(fd);

    return (json["fetch"]["status"].get<string>() == "Success");
}
*/

shared_ptr<FlightPlan> SimbriefJson::loadFile(string filename)
{
    FILE* fd = fopen(filename.c_str(), "r");
    auto json = json::parse(fd);
    fclose(fd);
    return load(json);
}

std::shared_ptr<FlightPlan> SimbriefJson::loadString(std::string str)
{
    auto json = json::parse(str);
    return load(json);
}

shared_ptr<FlightPlan> SimbriefJson::fetch(std::string username)
{
    string url = "https://www.simbrief.com/api/xml.fetcher.php?username=" + username + "&json=1";
    printf("Loading SimBrief plan from URL: %s\n", url.c_str());

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);

    /* we pass our 'chunk' struct to the callback function */
    Payload chunk;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);

    json json = json::parse(chunk.response);

    free(chunk.response);

    return load(json);
}

shared_ptr<FlightPlan> SimbriefJson::load(json json)
{
    auto flightPlan = make_shared<FlightPlan>();

    flightPlan->setCycle(atoi(json["params"]["airac"].get<string>().c_str()));
    string origin = json["origin"]["icao_code"].get<string>();
    string dest = json["destination"]["icao_code"].get<string>();
    flightPlan->setOrigin(getNavDataSource()->getAirports()->findByCode(origin));
    flightPlan->setDestination(getNavDataSource()->getAirports()->findByCode(dest));
    flightPlan->setCruisingAltitude(atoi(json["general"]["initial_altitude"].get<string>().c_str()));

    auto plan = json["navlog"]["fix"];
    vector<RoutePoint> points;
    for (auto fix : plan)
    {
        if (fix["is_sid_star"].get<string>() == "1")
        {
            // Skip SID and STAR way points
            continue;
        }

        float lat = atof(fix["pos_lat"].get<string>().c_str());
        float lon = atof(fix["pos_long"].get<string>().c_str());
        Coordinate coordinate = {lat, lon};

        RoutePoint wayPoint;
        string type = fix["type"].get<string>();
        string ident = fix["ident"].get<string>();
        if (type == "vor" || type == "wpt")
        {
            auto navAid = getNavDataSource()->getNavAids()->findById(ident, coordinate);
            if (navAid == nullptr)
            {
                printf("Fix:  -> No NavAid found\n");
                return nullptr;
            }
            //printf("Fix:  -> Found NavAid: %s\n", navAid->getId().c_str());
            //wayPoint = WayPoint(navAid);
            wayPoint.name = navAid->getName();
            wayPoint.position = navAid->getLocation();
            wayPoint.type = navAid->getType();
        }
        else if (type == "ltlg")
        {
            if (ident == "TOC" || ident == "TOD")
            {
                continue;
            }
            wayPoint.name = ident;
            wayPoint.position = coordinate;
            wayPoint.type = NavAidType::WAY_POINT;
        }
        wayPoint.altitude = atof(fix["altitude_feet"].get<string>().c_str());
        points.push_back(wayPoint);
    }
    flightPlan->setRoute(points);
    return flightPlan;
}

bool SimbriefJson::saveFile(std::shared_ptr<FlightPlan> flightPlan, std::string filename)
{
    return false;
}
