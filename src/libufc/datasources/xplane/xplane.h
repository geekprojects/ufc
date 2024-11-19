//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_XPLANE_H
#define XPFD_XPLANE_H

#include <ufc/datasource.h>
#include "xplaneclient.h"
#include "xpmapping.h"

#include <yaml-cpp/yaml.h>

#include <map>

namespace UFC
{


class XPlaneDataSource : public DataSource
{
 private:
    std::shared_ptr<XPlaneClient> m_client;

    XPMapping m_mapping;

    int m_xPlaneVersion = 0;

    void update(const std::map<int, float>& values);

 public:
    explicit XPlaneDataSource(FlightConnector* flightConnector);
    ~XPlaneDataSource() override = default;

    std::shared_ptr<Airports> loadAirports() override;

    bool connect() override;
    void disconnect() override;

    bool update() override;

    void command(std::string command) override;

    int getXPlaneVersion() { return m_xPlaneVersion; }
};

}

#endif //XPFD_XPLANE_H
