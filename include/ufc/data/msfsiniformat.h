//
// Created by Ian Parker on 23/10/2024.
//

#ifndef MSFSINIFORMAT_H
#define MSFSINIFORMAT_H

#include "fpformat.h"

namespace UFC
{
class MSFSIniFormat : public UFC::FlightPlanFormat
{
public:
    explicit MSFSIniFormat(NavDataSource* navDataSource) : FlightPlanFormat(navDataSource, "MSFSIniFormat")
    {
    }

    //FormatType getType() override { return FormatType::MSFS_INI; }
    //std::string getName() override { return "msfs-ini"; }
    //std::string getDescription() override { return "MSFS INI"; }
    //bool check(std::string filename, const std::vector<std::vector<std::string>>& file) override;
    std::shared_ptr<FlightPlan> loadFile(std::string filename) override;
    bool saveFile(std::shared_ptr<FlightPlan> flightPlan, std::string filename) override {return false; };
};
}

#endif //MSFSFORMAT_H
