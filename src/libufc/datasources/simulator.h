//
// Created by Ian Parker on 29/01/2024.
//

#ifndef XPFD_SIMULATOR_H
#define XPFD_SIMULATOR_H

#include <ufc/datasource.h>

namespace UFC
{

class SimulatorDataSource : public DataSource
{
 public:
    explicit SimulatorDataSource(FlightConnector* flightConnector);
    ~SimulatorDataSource() override = default;

    bool init() override;
    void close() override;

    bool update() override;
};

}

#endif //XPFD_SIMULATOR_H
