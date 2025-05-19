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

#include "ufc/navdata.h"

namespace UFC
{


class XPlaneDataSource : public DataSource
{
 private:
    std::shared_ptr<XPlaneClient> m_client;

    XPMapping m_mapping;

    int m_xPlaneVersion = 0;

    void update(const std::map<int, float>& values);

    NavDataHeader loadHeader(std::string headerStr);
    void loadFixes(const std::shared_ptr<NavAids> &navData);
    void loadNavAidData(const std::shared_ptr<NavAids>& shared);

 public:
    explicit XPlaneDataSource(FlightConnector* flightConnector);
    ~XPlaneDataSource() override = default;

    std::shared_ptr<Airports> loadAirports() override;
    std::shared_ptr<NavAids> loadNavAids() override;

    bool connect() override;
    void disconnect() override;
    bool isConnected() override;

    bool update() override;

    void command(const std::string& command) override;
    void setData(const std::string& dataName, float value) override;

    bool getDataInt(const std::string& dataName, int& value) override;
    bool getDataFloat(const std::string& dataName, float& value) override;
    bool getDataString(const std::string& dataName, std::string& value) override;

    void sendMessage(const std::string& message) override;

    [[nodiscard]] int getXPlaneVersion() const { return m_xPlaneVersion; }
};

}

#endif //XPFD_XPLANE_H
