//
// Created by Ian Parker on 20/08/2025.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_SERIAL_H
#define UNIVERSALFLIGHTCONNECTOR_SERIAL_H

#include <regex>
#include <ufc/flightconnector.h>
#include <ufc/device.h>

class SerialPort;

namespace UFC
{

enum class SerialPortType
{
    USB,
    UNKNOWN
};

struct SerialInputMatch
{
    std::string id;
    int value;
    std::vector<std::string> commands;
};

struct SerialInput
{
    std::regex regex;

    std::string reply;

    bool hasCommands;
    int idIdx;
    int valueIdx;

    std::vector<SerialInputMatch> matches;
};

struct SerialPortInfo
{
    SerialPortType type = SerialPortType::UNKNOWN;

    /*! Address of the serial port (this can be passed to the constructor of Serial). */
    std::string port;

    /*! Human readable description of serial device if available. */
    std::string description;

    int vendorId = 0;
    int productId = 0;
    std::string serialNumber;
    std::string hardwareId;
};

class SerialConfigDevice : public Device
{
    YAML::Node m_config;
    SerialPortInfo m_serialPort;

    //serial::Serial m_serial;
    std::shared_ptr<SerialPort> m_serial = nullptr;

    std::string m_buffer;

    std::vector<SerialInput> m_inputs;
    std::map<std::string, int> m_values;

 public:
    SerialConfigDevice(FlightConnector* m_flightConnector, const YAML::Node &config, const SerialPortInfo &serialPort);

    bool detect() override;

    bool open();
    bool init() override;

    void close() override;

    void handleInput(const SerialInput &input, const std::smatch &cm);

    void update(const AircraftState &state) override;

    std::string readLine();

    std::string getPort() const
    {
        return m_serialPort.port;
    }
};

class SerialConfigManager : public Logger
{
    FlightConnector* m_flightConnector;

    std::map<std::string, std::shared_ptr<SerialConfigDevice>> m_devices;

    static std::vector<SerialPortInfo> listPorts();

 public:
    explicit SerialConfigManager(FlightConnector* flightConnector);

    std::shared_ptr<SerialConfigDevice> checkDevice(const YAML::Node &node, const std::vector<SerialPortInfo> &ports);

    void scan();

};

}

#endif //UNIVERSALFLIGHTCONNECTOR_SERIAL_H
