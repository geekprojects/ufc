//
// Created by Ian Parker on 19/01/2024.
//

#ifndef UFC_XPLANECLIENT_H
#define UFC_XPLANECLIENT_H

#include <ufc/data.h>

#include <netinet/in.h>

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
};// __attribute__((packed));

class XPlaneClient : Logger
{
    private:
    static std::vector<XPlaneClient*> g_clients;

    std::string m_host;
    int m_port = -1;
    int m_socket = -1;
    struct sockaddr_in m_serverAddr = {};
    int m_clientPort = -1;
    bool m_connected = false;

    std::vector<std::pair<int, std::string>> m_currentDataRefs;

    bool send(void* buffer, int len);
    std::shared_ptr<Data> receive();

    bool sendConnection();

    bool sendRREF(std::vector<std::pair<int, std::string>> datarefs, int freq);

 public:
    XPlaneClient();
    XPlaneClient(const std::string &host, int port);

    ~XPlaneClient() override;

    bool connect();
    bool disconnect();

    bool getPosition(Position& position);

    bool readString(const std::string &dataref, int len, std::string& value);
    bool read(const std::string& dataref, double& returnValue);
    bool readInt(std::string dataref, int& value)
    {
        double d;
        bool res = read(dataref, d);
        if (!res)
        {
            return false;
        }
        value = (int)d;
        return true;
    }

    bool streamDataRefs(const std::vector<std::pair<int, std::string>> &datarefs, const std::function<void(std::map<int, float>)> &, int count = 0);
    void stopDataRefs();

    void sendCommand(const std::string &command);

    void setDataRef(const std::string& string, float value);

    static void disconnectAll();
};

}

#endif //XPFD_XPLANECLIENT_H
