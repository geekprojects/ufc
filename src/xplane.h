//
// Created by Ian Parker on 19/01/2024.
//

#ifndef XPFD_XPLANE_H
#define XPFD_XPLANE_H

#include <geek/core-data.h>
#include <netinet/in.h>

#include <map>
#include <functional>

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
} __attribute__((packed));

class XPlaneClient
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

    bool send(void* buffer, int len) const;
    std::shared_ptr<Geek::Data> receive() const;

    bool sendConnection();

    bool sendRREF(std::vector<std::pair<int, std::string>> datarefs, int freq);

 public:
    XPlaneClient();
    ~XPlaneClient();

    bool connect();
    bool disconnect();

    bool getPosition(Position& position);

    bool streamDataRefs(std::vector<std::pair<int, std::string>> datarefs, std::function<void(std::map<int, float>)>);
    void stopDataRefs();

    static void disconnectAll();
};

#endif //XPFD_XPLANE_H
