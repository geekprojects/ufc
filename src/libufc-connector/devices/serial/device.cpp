//

#include <regex>
#include <unistd.h>

#include "serial.h"
#include "serialport.h"
#include "ufc/utils.h"

using namespace std;
using namespace UFC;

SerialConfigDevice::SerialConfigDevice(FlightConnector* m_flightConnector, const YAML::Node &config, const SerialPortInfo &serialPort) :
    Device(m_flightConnector, "SerialDevice"),
    m_config(config),
    m_serialPort(serialPort)
{
}

bool SerialConfigDevice::open()
{
    log(DEBUG, "open: Opening...");

    // 8N1
    /*
    m_serial.setBaudrate(9600);
    m_serial.setBytesize(serial::eightbits);
    m_serial.setParity(serial::parity_none);
    m_serial.setStopbits(serial::stopbits_one);

    serial::Timeout to = serial::Timeout::simpleTimeout(200);
    m_serial.setTimeout(to);

    m_serial.setPort(m_serialPort.port);

    m_serial.open();
    m_serial.flush();
    */
    m_serial = make_shared<SerialPort>(m_serialPort.port, 9600);
    return m_serial->open();
}

bool SerialConfigDevice::detect()
{
    open();

    usleep(10000);

    bool found = false;
    if (m_config["detect"])
    {
        string expected = StringUtils::trim(m_config["detect"].as<std::string>());

        int attempt = 0;
        while (attempt < 10)
        {
            attempt++;
            string line = readLine();
            line = StringUtils::trim(line);

            log(DEBUG, "checkDevice: line: %s == %s", line.c_str(), expected.c_str());
            if (line == expected)
            {
                found = true;
                break;
            }
        }
    }
    if (!found)
    {
        close();
        return false;
    }

    log(DEBUG, "detect: Found device!");

    return true;
}

bool SerialConfigDevice::init()
{
    if (!m_serial->isOpen())
    {
        open();
    }

    for (auto initCommand : m_config["init"])
    {
        if (initCommand["string"])
        {
            auto initStr = initCommand["string"].as<string>();
            initStr = StringUtils::trim(initStr);
            log(DEBUG, "init: Sending: %s", initStr.c_str());
            initStr += "\n";
            auto written = m_serial->write(initStr);
            if (written != (ssize_t)initStr.length())
            {
                log(WARN, "init: Only written %lu bytes, %lu expected", written, initStr.length());
            }
        }
    }

    for (auto input: m_config["input"])
    {
        SerialInput serialInput;
        auto regexStr = input["regex"].as<string>();
        serialInput.regex = regex(regexStr);
        if (input["reply"])
        {
            serialInput.reply = input["reply"].as<string>();
        }
        if (input["matches"])
        {
            serialInput.hasCommands = true;
            serialInput.idIdx = input["id"].as<int>();
            serialInput.valueIdx = input["value"].as<int>();

            for (auto match: input["matches"])
            {
                SerialInputMatch inputMatch;
                inputMatch.id = match["id"].as<string>();
                inputMatch.value = match["value"].as<int>();
                auto commandNode = match["command"];
                if (commandNode.IsSequence())
                {
                    for (auto command: commandNode)
                    {
                        inputMatch.commands.push_back(command.as<string>());
                    }
                }
                else
                {
                    inputMatch.commands.push_back(commandNode.as<string>());
                }
                serialInput.matches.push_back(inputMatch);
            }
        }
        m_inputs.push_back(serialInput);
    }

    return true;
}

void SerialConfigDevice::close()
{
    m_serial->close();
}

void SerialConfigDevice::update(shared_ptr<AircraftState> state)
{
    while (m_serial->available() > 0)
    {
        string line = readLine();
        //log(DEBUG, "update: line: %s", line.c_str());
        if (!line.empty())
        {
            line = StringUtils::trim(line);

            for (auto const& input: m_inputs)
            {
                smatch cm;
                string::const_iterator lineIt(line.cbegin());

                while (regex_search(lineIt, line.cend(), cm, input.regex))
                {
                    handleInput(input, cm);
                    lineIt = cm.suffix().first;
                }
            }
        }
    }

    for (auto output : m_config["outputs"])
    {
        string line;
        for (auto step: output["parts"])
        {
            if (step["string"])
            {
                line += step["string"].as<string>();
            }
            if (step["data"])
            {
                string dataRef = step["data"].as<string>();
                int value = state->getInt(dataRef);
//                log(DEBUG, "dataRef=%s, value=%d", dataRef.c_str(), value);
                line += to_string(value);
            }
        }
        //log(DEBUG, "update: output: %s", line.c_str());
        line += "\n";
        size_t res = m_serial->write(line);
        if (res != line.length())
        {
            log(WARN, "update: Only written %lu bytes, %lu expected", res, line.length());
        }
    }
    m_serial->flush();
}

void SerialConfigDevice::handleInput(const SerialInput& input, const smatch& cm)
{
    if (!input.reply.empty())
    {
        log(DEBUG, "update:  -> Replying with: %s", input.reply.c_str());
        m_serial->write(input.reply + "\n");
    }

    if (!input.matches.empty())
    {
        string name = cm[input.idIdx].str();
        int value = atoi(cm[input.valueIdx].str().c_str());

        bool changed = true;
        if (m_values.contains(name))
        {
            changed = false;
            if (m_values[name] != value)
            {
                changed = true;
                m_values[name] = value;
            }
        }
        else
        {
            m_values[name] = value;
        }

        if (changed)
        {
            log(DEBUG, "update: Value %s has changed to %d", name.c_str(), value);
            for (auto const& match: input.matches)
            {
                if (match.id == name && match.value == value)
                {
                    for (auto command: match.commands)
                    {
                        log(DEBUG, "update: Sending command: %s", command.c_str());

                        getFlightConnector()->getDataSource()->command(command);
                    }
                    break;
                }
            }
        }
    }
}

std::string SerialConfigDevice::readLine()
{
    return m_serial->readLine();
    /*
    while (m_serial->available() > 0)
    {
        string line = m_serial->readLine();
            log(DEBUG, "readLine:  -> <%s>", line.c_str());
        m_buffer += line;
        if (!line.ends_with('\n') && !line.ends_with('\r'))
        {
            log(DEBUG, "readLine:  -> Not a complete line?");
            continue;
        }
        m_buffer = StringUtils::trim(m_buffer);

        string result = m_buffer;
            log(DEBUG, "readLine: Result: <%s>", result.c_str());
        m_buffer = "";
        return result;
    }
    log(DEBUG, "readLine: Complete line is not available: buffer=%s\n", m_buffer.c_str());
    return "";
    */
}


