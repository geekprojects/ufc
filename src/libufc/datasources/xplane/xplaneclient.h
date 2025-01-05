//
// Created by Ian Parker on 19/01/2024.
//

#ifndef UFC_XPLANECLIENT_H
#define UFC_XPLANECLIENT_H

#include <ufc/data.h>
#include <ufc/udp.h>
#include "ufc/flightconnector.h"

#include <memory>

#include <map>
#include <functional>

namespace UFC
{

struct Position
{
    double dat_lon = 0.0;		// longitude of the aircraft in X-Plane of course, in degrees
    double dat_lat = 0.0;		// latitude
    double dat_ele = 0.0;		// elevation above sea level in meters
    float y_agl_mtr = 0.0;	// elevation above the terrain in meters
    float veh_the_loc = 0.0;	// pitch, degrees
    float veh_psi_loc = 0.0;	// true heading, in degrees
    float veh_phi_loc = 0.0;	// roll, in degrees
    float vx_wrl = 0.0;		// speed in the x, EAST, direction, in meters per second
    float vy_wrl = 0.0;		// speed in the y, UP, direction, in meters per second
    float vz_wrl = 0.0;		// speed in the z, SOUTH, direction, in meters per second
    float Prad = 0.0;			// roll rate in radians per second
    float Qrad = 0.0;			// pitch rate in radians per second
    float Rrad = 0.0;			// yah rate in radians per second
};

struct XPlaneAlertMessage
{
    char header[5];
    char message1[240];	// needs to be multiple of 8 for the align to work out perfect for the copy?
    char message2[240];	// needs to be long enough to hold the strings!
    char message3[240];
    char message4[240];
};

class XPlaneClient : private Logger
{
    static std::vector<XPlaneClient*> g_clients;

    std::string m_host;
    int m_port;
    std::shared_ptr<UDPSocket> m_dataSocket;

    std::shared_ptr<UDPSocket> createConnection() const;

    Result sendRREF(const std::shared_ptr<UDPSocket> &socket, std::vector<std::pair<int, std::string>> datarefs, int freq);

    Result streamDataRefs(std::shared_ptr<UDPSocket> socket, const std::vector<std::pair<int, std::string>> &datarefs, const std::function<void(std::map<int, float>)> &, int count = 0);

 public:
    XPlaneClient();
    XPlaneClient(const std::string &host, int port);

    ~XPlaneClient() override;

    Result connect();
    void disconnect() const;
    bool isConnected() const { return m_dataSocket->isConnected(); }

    Result getPosition(Position& position);

    Result readString(const std::string &dataref, int len, std::string& value);
    Result read(const std::string& dataref, float& returnValue);
    Result readInt(const std::string& dataref, int& value)
    {
        float d;
        auto res = read(dataref, d);
        if (res != Result::SUCCESS)
        {
            return res;
        }
        value = (int)d;
        return Result::SUCCESS;
    }

    Result streamDataRefs(const std::vector<std::pair<int, std::string>> &datarefs, const std::function<void(std::map<int, float>)> &, int count = 0);

    Result sendCommand(const std::string &command);

    Result setDataRef(const std::string& string, float value);

    void sendMessage(const std::string& string);

    static void disconnectAll();
};

}

#endif
