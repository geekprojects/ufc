//
// Created by Ian Parker on 27/05/2024.
//

#ifndef FLIGHTCONNECTOR_H
#define FLIGHTCONNECTOR_H

#include <thread>

#include <ufc/datasource.h>

#include "logger.h"

namespace UFC
{

class DataSource;
class Device;

class FlightConnector : public Logger
{
 private:
    std::shared_ptr<DataSource> m_dataSource;
    std::vector<Device*> m_devices;

    bool m_running = false;
    std::shared_ptr<std::thread> m_updateDeviceThread;
    std::shared_ptr<std::thread> m_updateDataSourceThread;

    static void updateDeviceThread(FlightConnector* flightConnector);
    void updateDeviceMain();

    static void updateDataSourceThread(FlightConnector* flightConnector);
    void updateDataSourceMain();

 public:
    FlightConnector();
    ~FlightConnector();

    bool init();

    std::shared_ptr<DataSource> openDefaultDataSource();
    std::shared_ptr<DataSource> openDataSource(std::string name);
    std::shared_ptr<DataSource> getDataSource() { return m_dataSource; }

    void start();
    void stop();
    void wait();
};

}

#endif //FLIGHTCONNECTOR_H
