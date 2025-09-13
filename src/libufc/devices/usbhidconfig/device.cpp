
#include <ufc/bitbuffer.h>
#include <ufc/flightconnector.h>

#include <hidapi.h>
#include <yaml-cpp/yaml.h>

#include "usbhidconfig.h"
#include "lcd.h"

using namespace std;
using namespace UFC;

#ifdef DEBUG_USBHIDCONFIG
static void hexdump(const uint8_t* pos, int len)
{
    int i;
    for (i = 0; i < len; i += 16)
    {
        int j;
        printf("%08llx: ", (uint64_t)(i));
        for (j = 0; j < 16 && (i + j) < len; j++)
        {
            printf("%02x ", (uint8_t)pos[i + j]);
        }
        for (j = 0; j < 16 && (i + j) < len; j++)
        {
            char c = pos[i + j];
            if (!isprint(c))
            {
                c = '.';
            }
            printf("%c", c);
        }
        printf("\n");
    }
}
#endif

USBHIDConfigDevice::USBHIDConfigDevice(
    FlightConnector* flightConnector,
    const std::string &name,
    uint16_t vendorId,
    uint16_t productId)
        : USBHIDDevice(flightConnector, name, vendorId, productId)
{
    m_foundId.vendorId = vendorId;
    m_foundId.productId = productId;
}

bool USBHIDConfigDevice::init()
{
    bool res = USBHIDDevice::init();
    if (!res)
    {
        return false;
    }

    hid_set_nonblocking(getDevice(), true);

    auto state = make_shared<AircraftState>();
    map<string, uint8_t> displayValues;
    for (auto& initDesc : m_init)
    {
        updateOutput(state, initDesc, displayValues);
    }
    return true;
}

void USBHIDConfigDevice::close()
{
    auto state = make_shared<AircraftState>();
    map<string, uint8_t> displayValues;
    for (auto& closeDesc : m_close)
    {
        updateOutput(state, closeDesc, displayValues);
    }
    USBHIDDevice::close();
}

map<string, uint8_t> USBHIDConfigDevice::createDisplayValues(const shared_ptr<AircraftState>& state)
{
    map<string, uint8_t> displayValues;
    if (state->getInt("autopilot/displaySpeed"))
    {
        float apSpeed = state->getFloat("autopilot/speed");
        int speed;
        if (!state->getInt("autopilot/speedMach"))
        {
            speed = (int)apSpeed;
        }
        else
        {
            speed = (int) (apSpeed * 1000.0f);
        }

        displayValues.try_emplace("display/speed[2]", getLCDDigit(speed, 2));
        displayValues.try_emplace("display/speed[1]", getLCDDigit(speed, 1));
        displayValues.try_emplace("display/speed[0]", getLCDDigit(speed, 0));
    }
    else
    {
        displayValues.try_emplace("display/speed[2]", 2);
        displayValues.try_emplace("display/speed[1]", 2);
        displayValues.try_emplace("display/speed[0]", 2);
    }

    if (state->getInt("autopilot/displayHeading"))
    {
        int heading = (int)floor(state->getFloat("autopilot/heading"));
        displayValues.try_emplace("display/heading[2]", getLCDDigit(heading, 2));
        displayValues.try_emplace("display/heading[1]", getLCDDigit(heading, 1));
        displayValues.try_emplace("display/heading[0]", getLCDDigit(heading, 0));
    }
    else
    {
        displayValues.try_emplace("display/heading[2]", 2);
        displayValues.try_emplace("display/heading[1]", 2);
        displayValues.try_emplace("display/heading[0]", 2);
    }

    if (state->getInt("autopilot/displayAltitude"))
    {
        int altitude = (int) state->getFloat("autopilot/altitude");
        displayValues.try_emplace("display/altitude[4]", getLCDDigit(altitude, 4));
        displayValues.try_emplace("display/altitude[3]", getLCDDigit(altitude, 3));
        displayValues.try_emplace("display/altitude[2]", getLCDDigit(altitude, 2));
        displayValues.try_emplace("display/altitude[1]", getLCDDigit(altitude, 1));
        displayValues.try_emplace("display/altitude[0]", getLCDDigit(altitude, 0));
    }
    else
    {
        displayValues.try_emplace("display/altitude[4]", 2);
        displayValues.try_emplace("display/altitude[3]", 2);
        displayValues.try_emplace("display/altitude[2]", 2);
        displayValues.try_emplace("display/altitude[1]", 2);
        displayValues.try_emplace("display/altitude[0]", 2);
    }

    if (state->getInt("autopilot/displayVerticalSpeed"))
    {
        float verticalSpeed = state->getFloat("autopilot/verticalSpeed");
        displayValues.try_emplace("display/negativeVerticalSpeed", (verticalSpeed < 0));

        int vs = abs((int) verticalSpeed);
        displayValues.try_emplace("display/verticalSpeed[3]", getLCDDigit(vs, 3));
        displayValues.try_emplace("display/verticalSpeed[2]", getLCDDigit(vs, 2));

        if (!state->getInt("autopilot/verticalSpeedFPAMode"))
        {
            displayValues.try_emplace("display/verticalSpeed[1]", 0x1b);
            displayValues.try_emplace("display/verticalSpeed[0]", 0x1b);
        }
        else
        {
            displayValues.try_emplace("display/verticalSpeed[1]", 0);
            displayValues.try_emplace("display/verticalSpeed[0]", 0);
        }
    }
    else
    {
        displayValues.try_emplace("display/verticalSpeed[3]", 2);
        displayValues.try_emplace("display/verticalSpeed[2]", 2);
        displayValues.try_emplace("display/verticalSpeed[1]", 2);
        displayValues.try_emplace("display/verticalSpeed[0]", 2);
    }

#ifdef DEBUG_USBHIDCONFIG
    for (auto dv : displayValues)
    {
        log(DEBUG, "Display Value: %s = %d", dv.first.c_str(), dv.second);
    }
#endif

    return displayValues;
}

void USBHIDConfigDevice::update(shared_ptr<AircraftState> state)
{
    updateInput(state);

    map<string, uint8_t> displayValues = createDisplayValues(state);

    for (auto& output : m_outputs)
    {
        updateOutput(state, output, displayValues);
    }
}

void USBHIDConfigDevice::updateOutput(
    const shared_ptr<AircraftState>& state,
    const Descriptor& output,
    const map<string, uint8_t>& displayValues)
{
    BitBuffer bitBuffer;
    if (output.hasReportId)
    {
        bitBuffer.appendByte(output.reportId);
    }
    for (auto const& field: output.fields)
    {
        switch (field.type)
        {
            case FieldType::BIT:
            {
                auto value = (uint8_t)getValue(state, field, displayValues);
                bitBuffer.appendBit(value & 1);
                //log(DEBUG, "updateOutput: BIT: %s -> value=0x%x", field.dataRef.c_str(), value);
                break;
            }
            case FieldType::BITS:
            {
                uint8_t value = getValue(state, field, displayValues);
                //log(DEBUG, "updateOutput: BITS: %s -> length=%d, value=0x%x", field.dataRef.c_str(), field.length, value);
                for (int i = 0; i < field.length; i++)
                {
                    int v = (value >> i) & 0x1;
                    bitBuffer.appendBit(v);
                }
                break;
            }

            case FieldType::BYTE:
                bitBuffer.appendByte((uint8_t)getValue(state, field, displayValues));
                break;

            case FieldType::UINT16:
            {
                uint16_t value = getValue(state, field, displayValues);
                bitBuffer.appendByte(value >> 0);
                bitBuffer.appendByte(value >> 8);
                break;
            }

            case FieldType::UINT32:
            {
                int value = getValue(state, field, displayValues);
                bitBuffer.appendByte((value >> 0) & 0xff);
                bitBuffer.appendByte((value >> 8) & 0xff);
                bitBuffer.appendByte((value >> 16) & 0xff);
                bitBuffer.appendByte((value >> 24) & 0xff);
                break;
            }

            case FieldType::DATA:
                //log(DEBUG, "updateOutput: DATA: Appending %d bytes of data", field.data.size());
                for (uint8_t value : field.data)
                {
                    bitBuffer.appendByte(value);
                }
                break;

            case FieldType::PADDING:
                bitBuffer.flushBits();
                while ((bitBuffer.size() * 8) < static_cast<size_t>(field.length))
                {
                    bitBuffer.appendByte(0);
                }
                break;
        }
    }
    bitBuffer.flushBits();

#ifdef DEBUG_USBHIDCONFIG
    log(DEBUG, "updateOutput: Writing %d bytes...", bitBuffer.size());
    hexdump(bitBuffer.data(), bitBuffer.size());
#endif

    hid_write(getDevice(), bitBuffer.data(), bitBuffer.size());
}

void USBHIDConfigDevice::updateInput(shared_ptr<AircraftState> state)
{
    uint8_t buffer[1024];
    while (true)
    {
        buffer[0] = 0x01;
        int res = hid_read(getDevice(), buffer, 1024);
        if (res <= 0)
        {
            break;
        }

        for (Descriptor& input : m_inputs)
        {
            if (!input.hasReportId || input.reportId == buffer[0])
            {
                int expectedLength = input.bitLength / 8;
                if (res >= expectedLength)
                {
                    BitBuffer bitBuffer(buffer, res);
                    if (input.hasReportId)
                    {
                        bitBuffer.readByte();
                    }
                    updateInput(state, input, bitBuffer);
                }
            }
        }
    }
}

void USBHIDConfigDevice::updateInput(shared_ptr<AircraftState> state, Descriptor &input, BitBuffer& buffer)
{
    for (auto& field: input.fields)
    {
        switch (field.type)
        {
            case FieldType::BIT:
            {
                bool value = buffer.readBit();
                if (field.previousState != value)
                {
                    log(DEBUG, "updateInput: Changed: %s -> %d", field.dataRef.c_str(), value);
                    if (value && !field.dataRef.empty())
                    {
                        log(DEBUG, "updateInput: Executing %s", field.dataRef.c_str());
                        getFlightConnector()->getDataSource()->command(field.dataRef);
                    }
                    field.previousState = value;
                }
                break;
            }

            default:
                log(ERROR, "updateInput: Unknown field type!");
                return;
        }
    }
}


int USBHIDConfigDevice::getValue(shared_ptr<AircraftState> state, const Field &field, const map<string, uint8_t>& displayValues)
{
    if (field.valueType == FieldValueType::VALUE)
    {
        return field.value;
    }
    if (field.valueType != FieldValueType::DATAREF)
    {
        log(ERROR, "getValue: Unsupported type!");
        exit(0);
    }

    string dataRef = field.dataRef;
    bool negate = false;
    if (dataRef.starts_with("!"))
    {
        negate = true;
        dataRef = dataRef.substr(1);
    }

    int value;
    if (displayValues.contains(dataRef))
    {
        value = displayValues.at(dataRef);
    }
    else
    {
        value = state->getInt(dataRef);
    }
    if (negate)
    {
        value = !value;
    }
    return value;
}

bool USBHIDConfigDevice::loadConfig(const YAML::Node &config)
{
    for (auto& node : config["init"])
    {
        Descriptor descriptor;
        descriptor.name = node.first.as<string>();
        parseDescriptor(node.second, descriptor);
        m_init.push_back(descriptor);
    }
    for (auto& node : config["close"])
    {
        Descriptor descriptor;
        descriptor.name = node.first.as<string>();
        parseDescriptor(node.second, descriptor);
        m_close.push_back(descriptor);
    }
    for (auto& node : config["input"])
    {
        Descriptor descriptor;
        descriptor.name = node.first.as<string>();
        parseDescriptor(node.second, descriptor);
        m_inputs.push_back(descriptor);
    }
    for (auto& node : config["output"])
    {
        Descriptor descriptor;
        descriptor.name = node.first.as<string>();
        parseDescriptor(node.second, descriptor);
        m_outputs.push_back(descriptor);
    }
    return true;
}

void USBHIDConfigDevice::parseDescriptor(const YAML::Node& descriptorNode, Descriptor& descriptor)
{
    auto fields = descriptorNode["fields"];
    if (descriptorNode["reportId"])
    {
        descriptor.hasReportId = true;
        descriptor.reportId = descriptorNode["reportId"].as<uint8_t>();
    }
    else
    {
        descriptor.hasReportId = false;
    }
    for (auto fieldNode : fields)
    {
        Field field;
        if (fieldNode["bit"])
        {
            field.type = FieldType::BIT;
            field.length = 1;

            parseFieldValue(field, fieldNode["bit"]);
        }
        else if (fieldNode["bits"])
        {
            auto bits = fieldNode["bits"];
            field.type = FieldType::BITS;
            field.length = bits["size"].as<int>();

            parseFieldValue(field, bits["value"]);
        }
        else if (fieldNode["byte"])
        {
            field.type = FieldType::BYTE;
            field.length = 8;

            parseFieldValue(field, fieldNode["byte"]);
        }
        else if (fieldNode["uint16"])
        {
            field.type = FieldType::UINT16;
            field.length = 16;

            parseFieldValue(field, fieldNode["uint16"]);
        }
        else if (fieldNode["uint32"])
        {
            field.type = FieldType::UINT32;
            field.length = 32;

            parseFieldValue(field, fieldNode["uint32"]);
        }
        else if (fieldNode["data"])
        {
            field.type = FieldType::DATA;
            field.length = 0;
            for (auto value : fieldNode["data"])
            {
                field.data.push_back(value.as<uint8_t>());
                field.length += 8;
            }
        }
        else if (fieldNode["padding"])
        {
            int length = fieldNode["padding"].as<int>() * 8;
            field.type = FieldType::PADDING;
            field.length = length;
        }
        descriptor.fields.push_back(field);
        descriptor.bitLength += field.length;
    }
}

void USBHIDConfigDevice::parseFieldValue(Field& field, const YAML::Node& node)
{
    auto str = node.as<string>();
    if (node.IsNull())
    {
        field.valueType = FieldValueType::VALUE;
        field.value = 0;
        return;
    }

    try
    {
        field.value = node.as<int>();
        field.valueType = FieldValueType::VALUE;
        return;
    }
    catch (YAML::BadConversion const&)
    {
        // Not an integer!
        field.value = 0;
    }

    field.valueType = FieldValueType::DATAREF;
    field.dataRef = node.as<string>();
}
