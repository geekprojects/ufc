
#include <ufc/utils/bitbuffer.h>
#include <ufc/flightconnector.h>

#include <hidapi.h>
#include <yaml-cpp/yaml.h>

#include "usbhidconfig.h"
#include "lcd.h"
#include "../../lua.h"

#include "ufc/utils/utils.h"

using namespace std;
using namespace UFC;

#ifdef DEBUG_USBHIDCONFIG
static void hexdump(const uint8_t* pos, int len)
{
    int offset;
    for (offset = 0; offset < len; offset += 16)
    {
        int byteIndex;
        printf("%08llx: ", (uint64_t)(offset));
        for (byteIndex = 0; byteIndex < 16 && (offset + byteIndex) < len; byteIndex++)
        {
            printf("%02x ", (uint8_t)pos[offset + byteIndex]);
        }
        for (byteIndex = 0; byteIndex < 16 && (offset + byteIndex) < len; byteIndex++)
        {
            char c = pos[offset + byteIndex];
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

    m_lua = make_shared<UFCLua>(flightConnector);
}

bool USBHIDConfigDevice::init()
{
    bool res = USBHIDDevice::init();
    if (!res)
    {
        return false;
    }

    hid_set_nonblocking(getDevice(), true);

    if (!m_initScript.empty())
    {
        log(DEBUG, "init: Calling init script...");
        m_lua->execute(m_initScript);
    }

    for (const Descriptor& descriptor : m_init)
    {
        log(DEBUG, "init: Sending init descriptor: %s", descriptor.name.c_str());
        updateOutput({}, descriptor, {});
    }

#if 0
    if (!m_fmcFontFile.empty())
    {
        string fontFile = getFlightConnector()->getDataPath() + "/" + m_fmcFontFile;
        log(DEBUG, "init: Loading font file: %s", fontFile.c_str());
        auto font = YAML::LoadFile(fontFile);
        auto data = font["font"]["data"];
        int idx = 0;
        for (auto dataNode : data)
        {
            auto values = dataNode.second;
            log(DEBUG, "init: Sending font data %d...", idx + 1);
            BitBuffer bitBuffer;
            log(DEBUG, "init: type=%d, isArray=%d", values.Type(), values.IsSequence());
            //bitBuffer.appendByte(0xf0);
            for (auto valueNode : values)
            {
                bitBuffer.appendByte(valueNode.as<uint8_t>());
            }
            sendBuffer(0xf0, bitBuffer);
            ///hid_write(getDevice(), bitBuffer.data(), bitBuffer.size());
            idx++;
        }
    }
#endif

    clear();

    return true;
}

#if 0
void Font::convertGlyphDataForHardware(std::vector<std::vector<unsigned char>> &data, unsigned char hardwareIdentifier, FMCHardwareType hardwareType) {
    for (auto &row : data) {
        for (size_t i = 0; i + 1 < row.size(); i++) {
            if (row[i] == 0x32 && row[i + 1] == 0xbb) { // Sniffed packets always have the MCDU identifier
                row[i] = hardwareIdentifier;
                row[i + 1] = 0xbb;
            }
        }

        // The text-grid origin lives in an 8-param control block laid out as:
        //   .. 08 00 00 00 <left> 00 <top> 00 0e 00 18 00
        // (0e/18 = the 14x24 character grid). Its position within the row is
        // not fixed: most fonts (737, 744, default, xcrafts) prefix an extra
        // control block, pushing the values past the indices the old fixed
        // check used, so the margin was only ever patched for the Airbus/VGA
        // fonts. Locate the block by signature instead (sniffed defaults are the
        // MCDU values: left 0x34, top 0x25). The position itself comes from the
        // per-hardware screen-layout config so it lives in one place; the FMC also
        // re-asserts it via setScreenPosition after the upload.
        //const FMCScreenLayout layout = FMCHardwareMapping::ScreenLayoutForHardware(hardwareType);
        for (size_t i = 0; i + 11 < row.size(); i++) {
            if (row[i] == 0x08 && row[i + 1] == 0x00 && row[i + 2] == 0x00 && row[i + 3] == 0x00 &&
                row[i + 4] == 0x34 && row[i + 5] == 0x00 && row[i + 6] == 0x25 && row[i + 7] == 0x00 &&
                row[i + 8] == 0x0e && row[i + 9] == 0x00 && row[i + 10] == 0x18 && row[i + 11] == 0x00) {
                row[i + 4] = static_cast<unsigned char>(36 + layout.x); // left
                row[i + 6] = static_cast<unsigned char>(20 + layout.y); // top
                }
        }
    }
}
#endif

void USBHIDConfigDevice::close()
{
    clear();
    USBHIDDevice::close();
}

void USBHIDConfigDevice::clear()
{
    const auto state = make_shared<AircraftState>();
    update(state);
}

map<string, AircraftValue> USBHIDConfigDevice::createDisplayValues(const shared_ptr<AircraftState> &state)
{
    map<string, AircraftValue> displayValues;
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

        displayValues.try_emplace("display/speed[2]", getDigit(speed, 2));
        displayValues.try_emplace("display/speed[1]", getDigit(speed, 1));
        displayValues.try_emplace("display/speed[0]", getDigit(speed, 0));
    }
    else
    {
        displayValues.try_emplace("display/speed[2]", DIGIT_DASH);
        displayValues.try_emplace("display/speed[1]", DIGIT_DASH);
        displayValues.try_emplace("display/speed[0]", DIGIT_DASH);
    }

    if (state->getInt("autopilot/displayHeading"))
    {
        auto heading = static_cast<int>(state->getFloat("autopilot/heading"));
        displayValues.try_emplace("display/heading[2]", getDigit(heading, 2));
        displayValues.try_emplace("display/heading[1]", getDigit(heading, 1));
        displayValues.try_emplace("display/heading[0]", getDigit(heading, 0));
    }
    else
    {
        displayValues.try_emplace("display/heading[2]", DIGIT_DASH);
        displayValues.try_emplace("display/heading[1]", DIGIT_DASH);
        displayValues.try_emplace("display/heading[0]", DIGIT_DASH);
    }

    if (state->getInt("autopilot/displayAltitude"))
    {
        auto altitude = static_cast<int>(state->getFloat("autopilot/altitude"));
        displayValues.try_emplace("display/altitude[4]", getDigit(altitude, 4));
        displayValues.try_emplace("display/altitude[3]", getDigit(altitude, 3));
        displayValues.try_emplace("display/altitude[2]", getDigit(altitude, 2));
        displayValues.try_emplace("display/altitude[1]", getDigit(altitude, 1));
        displayValues.try_emplace("display/altitude[0]", getDigit(altitude, 0));
    }
    else
    {
        displayValues.try_emplace("display/altitude[4]", DIGIT_DASH);
        displayValues.try_emplace("display/altitude[3]", DIGIT_DASH);
        displayValues.try_emplace("display/altitude[2]", DIGIT_DASH);
        displayValues.try_emplace("display/altitude[1]", DIGIT_DASH);
        displayValues.try_emplace("display/altitude[0]", DIGIT_DASH);
    }

    if (state->getInt("autopilot/displayVerticalSpeed"))
    {
        float verticalSpeed = state->getFloat("autopilot/verticalSpeed");
        displayValues.try_emplace("display/negativeVerticalSpeed", (verticalSpeed < 0));

        int vs = abs((int) verticalSpeed);
        displayValues.try_emplace("display/verticalSpeed[3]", getDigit(vs, 3));
        displayValues.try_emplace("display/verticalSpeed[2]", getDigit(vs, 2));

        if (!state->getInt("autopilot/verticalSpeedFPAMode"))
        {
            displayValues.try_emplace("display/verticalSpeed[1]", 0x1b);
            displayValues.try_emplace("display/verticalSpeed[0]", 0x1b);
        }
        else
        {
            displayValues.try_emplace("display/verticalSpeed[1]", DIGIT_DASH);
            displayValues.try_emplace("display/verticalSpeed[0]", DIGIT_DASH);
        }
    }
    else
    {
        displayValues.try_emplace("display/verticalSpeed[3]", DIGIT_DASH);
        displayValues.try_emplace("display/verticalSpeed[2]", DIGIT_DASH);
        displayValues.try_emplace("display/verticalSpeed[1]", DIGIT_DASH);
        displayValues.try_emplace("display/verticalSpeed[0]", DIGIT_DASH);
    }

    if (!state->getInt("aircraft/barometer/pilot/std"))
    {
        int mode = state->getInt("aircraft/barometer/pilot/mode");
        float qnh = state->getFloat("aircraft/barometer/pilot/in_hg");
        if (mode == 1)
        {
            qnh *= 33.86389f;
        }
        else
        {
            qnh *= 100.0f;
        }
        auto qnhInt = static_cast<int>(roundf(qnh));
        displayValues.try_emplace("display/qnh[3]", getDigit(qnhInt, 3));
        displayValues.try_emplace("display/qnh[2]", getDigit(qnhInt, 2));
        displayValues.try_emplace("display/qnh[1]", getDigit(qnhInt, 1));
        displayValues.try_emplace("display/qnh[0]", getDigit(qnhInt, 0));
    }
    else
    {
        displayValues.try_emplace("display/qnh[3]", DIGIT_SPACE);
        displayValues.try_emplace("display/qnh[2]", DIGIT_S);
        displayValues.try_emplace("display/qnh[1]", DIGIT_T);
        displayValues.try_emplace("display/qnh[0]", DIGIT_D);
    }

    return displayValues;
}

void USBHIDConfigDevice::update(shared_ptr<AircraftState> state)
{
    if (getFlightConnector()->getDataSource() == nullptr)
    {
        return;
    }
    updateInput();

    map<string, AircraftValue> displayValues = createDisplayValues(state);

    for (auto const& output : m_outputs)
    {
        updateOutput(state, output, displayValues);
    }

    if (m_hasFMC)
    {
        updateFMC(state);
    }
}

/*
 * Format a number in to something understood by a device.
 * The formats are hardcoded, this should be moved to the config yaml files!
 */
uint8_t USBHIDConfigDevice::formatDigit(uint8_t number, const std::string& format)
{
    if (format == "winwing1")
    {
        switch (number)
        {
            /*
             *    --40--
             *  04|    |20
             *    --02--
             *  01|    |10
             *    --08--
             */
            using enum LCDDigit;
            case 0: return static_cast<uint8_t>(NUMBER_0);
            case 1: return static_cast<uint8_t>(NUMBER_1);
            case 2: return static_cast<uint8_t>(NUMBER_2);
            case 3: return static_cast<uint8_t>(NUMBER_3);
            case 4: return static_cast<uint8_t>(NUMBER_4);
            case 5: return static_cast<uint8_t>(NUMBER_5);
            case 6: return static_cast<uint8_t>(NUMBER_6);
            case 7: return static_cast<uint8_t>(NUMBER_7);
            case 8: return static_cast<uint8_t>(NUMBER_8);
            case 9: return static_cast<uint8_t>(NUMBER_9);
            case DIGIT_DASH: return 2;
            default: return 0;
        }
    }

    if (format == "winwing2")
    {
        switch (number)
        {
            /*
             *    --10--
             *  01|    |20
             *    --02--
             *  04|    |40
             *    --08--
             */
            case 0: return 0x7d;
            case 1: return 0x60;
            case 2: return 0x3e;
            case 3: return 0x7a;
            case 4: return 0x63;
            case 5: return 0x5b;
            case 6: return 0x5f;
            case 7: return 0x70;
            case 8: return 0x7f;
            case 9: return 0x7b;
            case DIGIT_DASH: return 2;
            case DIGIT_SPACE: return 0x0;
            case DIGIT_S: return 0x5b;
            case DIGIT_T: return 0x0f;
            case DIGIT_D: return 0x6e;
            default: return 0;
        }
    }
    return 0;
}

void USBHIDConfigDevice::updateValue(
    const shared_ptr<AircraftState>& state,
    const Descriptor& output,
    const map<string, AircraftValue>& displayValues,
    BitBuffer& bitBuffer)
{
    for (auto const& field: output.fields)
    {
        switch (field.type)
        {
            case FieldType::BIT:
            {
                const auto value = static_cast<uint8_t>(getValue(state, field, displayValues));
                bitBuffer.appendBit(value & 1);
                break;
            }
            case FieldType::BITS:
            {
                const auto value = static_cast<uint8_t>(getValue(state, field, displayValues));
                for (int i = 0; i < field.length; i++)
                {
                    int v = (value >> i) & 0x1;
                    bitBuffer.appendBit(v);
                }
                break;
            }

            case FieldType::BYTE:
            {
                const auto value = static_cast<uint8_t>(getValue(state, field, displayValues));
                bitBuffer.appendByte(value);
                break;
            }

            case FieldType::UINT16:
            {
                const auto value = static_cast<uint16_t>(getValue(state, field, displayValues));
                bitBuffer.appendByte((value >> 0) & 0xff);
                bitBuffer.appendByte((value >> 8) & 0xff);
                break;
            }

            case FieldType::UINT32:
            {
                const auto value = static_cast<uint32_t>(getValue(state, field, displayValues));
                bitBuffer.appendByte((value >> 0) & 0xff);
                bitBuffer.appendByte((value >> 8) & 0xff);
                bitBuffer.appendByte((value >> 16) & 0xff);
                bitBuffer.appendByte((value >> 24) & 0xff);
                break;
            }

            case FieldType::DATA:
                for (const uint8_t value : field.data)
                {
                    bitBuffer.appendByte(value);
                }
                break;

            case FieldType::DIGIT:
            {
                auto value = static_cast<uint8_t>(getValue(state, field, displayValues));
                value = formatDigit(value, field.format);
                for (int i = 0; i < field.length; i++)
                {
                    bitBuffer.appendBit((value >> i) & 0x1);
                }
                break;
            }
            case FieldType::UTF8:
            {
                int value = getValue(state, field, displayValues);
                wstring charstr;
                charstr += static_cast<wchar_t>(value);
                string utf8char = wstring2utf8(charstr);
                for (size_t i = 0; i < utf8char.length(); i++)
                {
                    bitBuffer.appendByte(utf8char[i]);
                }
                break;
            }

            case FieldType::PADDING:
                bitBuffer.flushBits();
                while ((bitBuffer.size() * 8) < static_cast<size_t>(field.length))
                {
                    bitBuffer.appendByte(0);
                }
                break;
        }
    }
}

void USBHIDConfigDevice::updateOutput(
    const shared_ptr<AircraftState>& state,
    const Descriptor& output,
    const map<string, AircraftValue>& displayValues)
{
    BitBuffer bitBuffer;
    if (output.hasReportId)
    {
        bitBuffer.appendByte(output.reportId);
    }
    updateValue(state, output, displayValues, bitBuffer);
    bitBuffer.flushBits();

#ifdef DEBUG_USBHIDCONFIG
    log(DEBUG, "updateOutput: Writing %d bytes...", bitBuffer.size());
    hexdump(bitBuffer.data(), bitBuffer.size());
#endif

    hid_write(getDevice(), bitBuffer.data(), bitBuffer.size());
}

void USBHIDConfigDevice::updateInput()
{
    uint8_t buffer[1024];
    constexpr uint8_t kDefaultInputReportId = 0x01;
    while (true)
    {
        buffer[0] = kDefaultInputReportId;
        int res = hid_read(getDevice(), buffer, sizeof(buffer));
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
                    updateInput(input, bitBuffer);
                }
            }
        }
    }
}

void USBHIDConfigDevice::updateInput(Descriptor &input, BitBuffer &buffer)
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

void USBHIDConfigDevice::populateValue(
    const wstring& text,
    const string& valueName,
    const size_t col,
    map<string, AircraftValue>& values)
{
    if (text.length() > col)
    {
        values[valueName] = static_cast<int>(text.at(col));
    }
    else
    {
        values[valueName] = ' ';
    }
}

void USBHIDConfigDevice::sendBuffer(uint8_t reportId, const BitBuffer& bitBuffer)
{
    size_t pos = 0;
    while (pos < bitBuffer.size())
    {
        auto len = bitBuffer.size() - pos;
        if (len > 63)
        {
            len = 63;
        }
        std::vector<uint8_t> packet;
        packet.push_back(reportId);
        for (size_t i = 0; i < len; i++)
        {
            packet.push_back(bitBuffer.data()[i + pos]);
        }

        // Pad to 64 bytes, if necessary
        while (packet.size() < 64)
        {
            packet.push_back(0);
        }
        hid_write(getDevice(), packet.data(), packet.size());
        pos += len;
    }
}

void USBHIDConfigDevice::updateFMC(const shared_ptr<AircraftState>& state)
{
    BitBuffer bitBuffer;
    int idx = 0;
    for (int row = 0; row < 14; row++)
    {
        wstring text = state->getString("fmc/0/line" + to_string(row + 1) + "/text");
        wstring textColour = state->getString("fmc/0/line" + to_string(row + 1) + "/textColour");
        wstring backgroundColour = state->getString("fmc/0/line" + to_string(row + 1) + "/backgroundColour");
        wstring small = state->getString("fmc/0/line" + to_string(row + 1) + "/small");

        for (size_t col = 0; col < 24; ++col, ++idx)
        {
            map<string, AircraftValue> values;

            populateValue(text, "character", col, values);
            populateValue(textColour, "textColour", col, values);
            populateValue(backgroundColour, "backgroundColour", col, values);
            populateValue(small, "small", col, values);
            updateValue(state, m_fmcPageDescriptor, values, bitBuffer);
        }
    }
    bitBuffer.flushBits();

    //log(DEBUG, "updateFMC: Buffer: %d bytes", bitBuffer.size());
    //hexdump(bitBuffer.data(), bitBuffer.size());
    sendBuffer(0xf2, bitBuffer);
}

int USBHIDConfigDevice::getValue(
    const shared_ptr<AircraftState> &state,
    const Field &field,
    const map<string, AircraftValue>& displayValues)
{
    if (field.valueType == FieldValueType::LUA)
    {
        return static_cast<int>(m_lua->execute("usbhid-" + field.id, field.lua, displayValues));
    }
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
        value = displayValues.at(dataRef).getInt();
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
        auto name = node.first.as<string>();
        if (name == "lua")
        {
            m_initScript = node.second.as<string>();
            log(DEBUG, "loadConfig: init script: %s", m_initScript.c_str());
        }
        else
        {
            Descriptor descriptor;
            descriptor.name = node.first.as<string>();
            parseDescriptor(node.second, descriptor);
            m_init.push_back(descriptor);
        }
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

    if (config["fmc"])
    {
        m_hasFMC = true;
        auto character = config["fmc"]["page"]["character"];
        parseDescriptor(character, m_fmcPageDescriptor);

        // TODO: Make this come from a data ref
        m_fmcFontFile = config["fmc"]["fonts"]["airbus"].as<string>();
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

    int idx = 0;
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
        else if (fieldNode["utf8"])
        {
            field.type = FieldType::UTF8;
            field.length = 8;

            parseFieldValue(field, fieldNode["utf8"]);
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
            log(DEBUG, "parseDescriptor: data: %d bytes", field.length / 8);
        }
        else if (fieldNode["digit"])
        {
            auto bits = fieldNode["digit"];
            field.type = FieldType::DIGIT;
            field.length = bits["size"].as<int>();
            if (bits["format"])
            {
                field.format = bits["format"].as<string>();
            }

            if (bits["value"])
            {
                parseFieldValue(field, bits["value"]);
            }
        }
        else if (fieldNode["padding"])
        {
            int length = fieldNode["padding"].as<int>() * 8;
            field.type = FieldType::PADDING;
            field.length = length;
        }

        field.id = descriptor.name + "-";
        if (!field.dataRef.empty())
        {
            field.id += field.dataRef;
        }
        else
        {
            field.id += to_string(idx);
        }
        descriptor.fields.push_back(field);
        descriptor.bitLength += field.length;
        idx++;
    }
}

void USBHIDConfigDevice::parseFieldValue(Field& field, const YAML::Node& node)
{
    if (!node.IsScalar() && node["lua"])
    {
        field.valueType = FieldValueType::LUA;
        field.lua = node["lua"].as<string>();
        field.value = 0;
        return;
    }

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
