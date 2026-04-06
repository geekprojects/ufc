//
// Created by Ian Parker on 30/03/2026.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_FPFORMAT_H
#define UNIVERSALFLIGHTCONNECTOR_FPFORMAT_H

#include "flightplan.h"

namespace UFC
{

class FlightPlanFormat : protected Logger
{
    NavDataSource* m_navDataSource;

 protected:
    NavDataSource* getNavDataSource() const { return m_navDataSource; }

    static std::vector<std::vector<std::string>> readTextFile(std::string filename, bool split);

 public:
    FlightPlanFormat(NavDataSource* navSource, std::string name) : Logger(name), m_navDataSource(navSource) {}
    ~FlightPlanFormat() override = default;

    virtual std::shared_ptr<FlightPlan> loadString(std::string file) = 0;
    virtual std::shared_ptr<FlightPlan> loadFile(std::string fileName);

    virtual bool saveFile(std::shared_ptr<FlightPlan> flightPlan, std::string filename) = 0;
};

}

#endif //UNIVERSALFLIGHTCONNECTOR_FPFORMAT_H
