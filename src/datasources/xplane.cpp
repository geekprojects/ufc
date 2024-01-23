//
// Created by Ian Parker on 23/01/2024.
//

#include "xplane.h"

using namespace std;

XPlaneDataSource::XPlaneDataSource(XPFlightDisplay* display)
    : DataSource(display)
{
}

bool XPlaneDataSource::init()
{
    return m_client.connect();
}

void XPlaneDataSource::close()
{
    m_client.disconnect();
}

bool XPlaneDataSource::update()
{
    vector<pair<int, string>> datarefs;
    datarefs.push_back(make_pair(1, "sim/flightmodel/position/indicated_airspeed"));
    datarefs.push_back(make_pair(2, "sim/flightmodel/position/theta"));
    datarefs.push_back(make_pair(3, "sim/flightmodel/position/phi"));
    datarefs.push_back(make_pair(4, "sim/cockpit2/gauges/indicators/altitude_ft_pilot"));
    datarefs.push_back(make_pair(5, "sim/cockpit2/gauges/indicators/vvi_fpm_pilot"));
    datarefs.push_back(make_pair(6, "sim/cockpit2/gauges/indicators/mach_pilot"));
    datarefs.push_back(make_pair(7, "sim/flightmodel/position/mag_psi"));
    datarefs.push_back(make_pair(8, "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot"));

    return m_client.streamDataRefs(datarefs, [this](map<int, float> values)
    {
        updateState(values);
    });
}

void XPlaneDataSource::updateState(std::map<int, float> values)
{
    State state = m_display->getState();

    state.connected = true;
    for (auto it : values)
    {
        float value = it.second;
        switch (it.first)
        {
            case 1:
                state.indicatedAirspeed = value;
                break;
            case 2:
                state.pitch = value;
                break;
            case 3:
                state.roll = value;
                break;
            case 4:
                state.altitude = value;
                break;
            case 5:
                state.verticalSpeed = value;
                break;
            case 6:
                state.indicatedMach = value;
                break;
            case 7:
                state.magHeading = value;
                break;
            case 8:
                state.barometerHG = value;
                break;
        }
    }
    m_display->updateState(state);
}

