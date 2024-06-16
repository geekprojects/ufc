//
// Created by Ian Parker on 10/06/2024.
//

#include "arduinotest.h"

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/termios.h>

#include <ufc/flightconnector.h>

UFC_DEVICE(ArduinoTest, ArduinoTest)

using namespace std;
using namespace UFC;

const char* DEVICE_FILE = "/dev/cu.usbmodem2101";

static void updateThread(ArduinoTest* arduinoTest)
{
    arduinoTest->readMain();
}

int set_interface_attribs(int fd, int speed, int parity)
{
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
        printf("error %d from tcgetattr\n", errno);
        return -1;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN] = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        printf("error %d from tcsetattr\n", errno);
        return -1;
    }
    return 0;
}

void set_blocking(int fd, int should_block)
{
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0)
    {
        printf("error %d from tggetattr\n", errno);
        return;
    }

    tty.c_cc[VMIN] = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        printf("error %d setting term attributes\n", errno);
    }
}

inline void ltrim(std::string &s)
{
    s.erase(
        s.begin(), std::find_if(
            s.begin(), s.end(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }));
}

// trim from end (in place)
inline void rtrim(std::string &s)
{
    s.erase(
        std::find_if(
            s.rbegin(), s.rend(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }).base(), s.end());
}

inline void trim(std::string &s)
{
    rtrim(s);
    ltrim(s);
}

ArduinoTest::ArduinoTest(UFC::FlightConnector* flightConnector) : Device(flightConnector, "ArduinoTest")
{
}

ArduinoTest::~ArduinoTest()
{
    if (m_fd != -1)
    {
        close(m_fd);
    }
}

bool ArduinoTest::detect()
{
    int res = access(DEVICE_FILE, R_OK | W_OK);
    return res == 0;
}

bool ArduinoTest::init()
{
    m_fd = open(DEVICE_FILE, O_RDWR | O_NOCTTY | O_SYNC);
    if (m_fd == -1)
    {
        return false;
    }

    set_interface_attribs(m_fd, B115200, 0);
    set_blocking(m_fd, 0);

    m_readThread = new thread(updateThread, this);

    return true;
}

std::string ArduinoTest::getName()
{
    return "ArduinoDevice";
}

void ArduinoTest::update(UFC::AircraftState state)
{
    if (m_fd == -1)
    {
        return;
    }

    writeNumber(0, (int)state.autopilot.speed);
    writeNumber(2, (int)state.autopilot.altitude);
}

void ArduinoTest::writeNumber(int id, int value)
{
    char buffer[1024];
    snprintf(buffer, 1024, "NUMBER:%d:%d\n", id, value);
    //printf("ArduinoTest::update: %s", buffer);
    int len = strlen(buffer);
    write(m_fd, buffer, len);
}

void ArduinoTest::readMain()
{
    string buffer;
    char buf[1024];
    while (true)
    {
        int res = read(m_fd, buf, 1024);
        if (res < 0)
        {
            printf("mcpd: Error: Got %d\n", res);
            break;
        }
        if (res > 0)
        {
            buffer += string(buf, res);

            int idx;
            while ((idx = buffer.find('\n')) != -1)
            {
                string line = buffer.substr(0, idx);
                trim(line);
                buffer = buffer.substr(idx + 1);
                if (line.length() > 0)
                {
                    printf("mcpd: line: %s\n", line.c_str());
                    handleLine(line);
                }
            }
        }
    }
}

vector<string> splitString(string line, char splitChar)
{
    vector<string> parts;

    while (line.length() > 0)
    {
        size_t pos = line.find(splitChar);
        if (pos == string::npos)
        {
            pos = line.length();
            if (pos == 0)
            {
                break;
            }
        }
        if (pos >= 1)
        {
            string part = line.substr(0, pos);
            parts.push_back(part);
        }
        if (pos == line.length())
        {
            break;
        }
        line = line.substr(pos + 1);
    }

    return parts;
}

void ArduinoTest::handleLine(std::string line)
{
    vector<string> message = splitString(line, ':');

    if (message[0] == "EVENT")
    {
        if (message[1] == "ROTARY")
        {
            int rotary = atoi(message[2].c_str());
            int direction = atoi(message[3].c_str());
            if (rotary == 0 && direction == 0)
            {
                m_flightConnector->getDataSource()->command(AUTOPILOT_AIRSPEED_DOWN);
            }
            else if (rotary == 0 && direction == 1)
            {
                m_flightConnector->getDataSource()->command(AUTOPILOT_AIRSPEED_UP);
            }
            else if (rotary == 1 && direction == 0)
            {
                m_flightConnector->getDataSource()->command(AUTOPILOT_HEADING_DOWN);
            }
            else if (rotary == 1 && direction == 1)
            {
                m_flightConnector->getDataSource()->command(AUTOPILOT_HEADING_UP);
            }
            else if (rotary == 2 && direction == 0)
            {
                m_flightConnector->getDataSource()->command(AUTOPILOT_ALTITUDE_DOWN);
            }
            else if (rotary == 2 && direction == 1)
            {
                m_flightConnector->getDataSource()->command(AUTOPILOT_ALTITUDE_UP);
            }
        }
        else if (message[2] == "BUTTON")
        {
            int button = atoi(message[2].c_str());
            int state = atoi(message[3].c_str());
            switch (button)
            {
                case 0:
                    if (state == 1)
                    {
                        m_flightConnector->getDataSource()->command(AUTOPILOT_ALTITUDE_UP);
                    }
                break;
            }

        }
    }
}

