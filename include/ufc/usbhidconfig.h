//
// Created by Ian Parker on 22/07/2025.
//

#ifndef USBHIDCONFIG_H
#define USBHIDCONFIG_H
#include <string>
#include <yaml-cpp/yaml.h>

#include "bitbuffer.h"
#include "usbhiddevice.h"

namespace UFC
{

enum class FieldType
{
    BIT,
    BITS,
    BYTE,
    UINT16,
    UINT32,
    DATA,
    PADDING
};

enum class FieldValueType
{
    VALUE,
    DATAREF
};

struct Field
{
    FieldType type;
    FieldValueType valueType;
    int length;
    int value;
    std::string dataRef;
    std::vector<uint8_t> data;

    bool previousState = false;
};

struct Descriptor
{
    std::string name;
    bool hasReportId = false;
    uint8_t reportId = 0;
    int bitLength = 0;
    std::vector<Field> fields;
};

class USBHIDConfigDevice : public USBHIDDevice
{
    std::vector<Descriptor> m_init;
    std::vector<Descriptor> m_close;
    std::vector<Descriptor> m_inputs;
    std::vector<Descriptor> m_outputs;

    void updateOutput(const AircraftState &state, Descriptor& output, const std::map<std::string, unsigned char> &displayValues);
    void updateInput(const AircraftState &state);
    void updateInput(const AircraftState &state, Descriptor &input, BitBuffer &buffer);

    int getValue(const AircraftState &state, const Field &field, const std::map<std::string, unsigned char> &displayValues);

    void parseDescriptor(const YAML::Node &descriptorNode, Descriptor &descriptor);
    void parseFieldValue(Field &field, const YAML::Node &node);

public:
    USBHIDConfigDevice(FlightConnector* flightConnector, const std::string &name, uint16_t vendorId, uint16_t productId);

    bool loadConfig(const YAML::Node & node);

    bool init() override;
    void close() override;

    std::map<std::string, unsigned char> createDisplayValues(const AircraftState &state);

    void update(const AircraftState &state) override;
};

class USBHIDConfigManager : Logger
{
    FlightConnector* m_flightConnector;
    std::string m_baseDir;
    std::map<USBIds, USBHIDConfigDevice*> m_devices;

public:
    USBHIDConfigManager(FlightConnector* flightConnector, const std::string baseDir) :
        Logger("USBHIDConfigManager"),
        m_flightConnector(flightConnector),
        m_baseDir(baseDir)
    {}

    bool checkDevice(YAML::Node node, USBIds &id);
    void openDevice(const YAML::Node & node, USBIds id);

    void scan();
};
}

#endif //USBHIDCONFIG_H
