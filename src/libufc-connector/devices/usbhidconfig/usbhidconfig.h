//
// Created by Ian Parker on 22/07/2025.
//

#ifndef USBHIDCONFIG_H
#define USBHIDCONFIG_H

#include <string>
#include <yaml-cpp/yaml.h>

#include <../../../include/ufc/utils/bitbuffer.h>
#include <ufc/flightconnector.h>
#include <../../../include/ufc/devices/usbhiddevice.h>

namespace UFC
{

#define DIGIT_DASH '-'
#define DIGIT_SPACE ' '
#define DIGIT_S 'S'
#define DIGIT_T 'T'
#define DIGIT_D 'D'

enum class FieldType
{
    BIT,
    BITS,
    BYTE,
    UINT16,
    UINT32,
    DATA,
    DIGIT,
    PADDING
};

enum class FieldValueType
{
    VALUE,
    DATAREF,
    LUA
};

struct Field
{
    FieldType type;
    FieldValueType valueType;
    int length;
    int value;
    std::string dataRef;
    std::vector<uint8_t> data;
    std::string format;
    std::string lua;

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

    std::shared_ptr<UFCLua> m_lua = nullptr;

    uint8_t formatDigit(uint8_t uint8, const std::string & string);

    void updateOutput(const std::shared_ptr<AircraftState> &state, const Descriptor &output, const std::map<std::string, unsigned char> &
                      displayValues);
    void updateInput(std::shared_ptr<AircraftState> state);
    void updateInput(std::shared_ptr<AircraftState> state, Descriptor &input, BitBuffer &buffer);

    int getValue(std::shared_ptr<AircraftState> state, const Field &field, const std::map<std::string, unsigned char> &displayValues);

    void parseDescriptor(const YAML::Node &descriptorNode, Descriptor &descriptor);

    static void parseFieldValue(Field &field, const YAML::Node &node);

public:
    USBHIDConfigDevice(FlightConnector* flightConnector, const std::string &name, uint16_t vendorId, uint16_t productId);

    bool loadConfig(const YAML::Node & node);

    bool init() override;

    void clear();

    void close() override;

    std::map<std::string, unsigned char> createDisplayValues(const std::shared_ptr<AircraftState> &state);

    void update(std::shared_ptr<AircraftState> state) override;
};

class USBHIDConfigManager : private Logger
{
    FlightConnector* m_flightConnector;
    std::map<USBIds, std::shared_ptr<USBHIDConfigDevice>> m_devices;

public:
    explicit USBHIDConfigManager(FlightConnector* flightConnector);

    bool checkDevice(YAML::Node node, USBIds &id);
    void openDevice(const YAML::Node & node, USBIds id);

    void scan();
};
}

#endif //USBHIDCONFIG_H
