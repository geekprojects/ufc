//
// Created by Ian Parker on 23/10/2024.
//

#ifndef XPLANEFMSFORMAT_H
#define XPLANEFMSFORMAT_H
#include "fpformat.h"

namespace UFC
{
class XPlaneFormat : public FlightPlanFormat
{
public:
    explicit XPlaneFormat(NavDataSource* navDataSource)
        : FlightPlanFormat(navDataSource, "XPlaneFormat")
    {
    }

    //FormatType getType() override { return FormatType::XPLANE; }
    //std::string getName() override { return "xplane-fms"; }
    //std::string getDescription() override { return "X-Plane FMS"; }

    //bool check(std::string filename, const std::vector<std::vector<std::string>>& file) override;
    std::shared_ptr<FlightPlan> loadFile(std::string filename) override;
    bool saveFile(std::shared_ptr<FlightPlan> flightPlan, std::string filename) override;
};
}

#endif //XPLANEFMSFORMAT_H
