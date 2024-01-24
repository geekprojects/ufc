//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_XPLANE_H
#define XPFD_XPLANE_H

#include "datasources/datasource.h"
#include "xplaneclient.h"

#include <map>

class XPlaneDataSource : public DataSource
{
 private:
    XPlaneClient m_client;

    void updateState(const std::map<int, float>& values);

 public:
    explicit XPlaneDataSource(XPFlightDisplay* display);
    ~XPlaneDataSource() override = default;

    bool init() override;
    void close() override;

    bool update() override;

};


#endif //XPFD_XPLANE_H
