//
// Created by Ian Parker on 11/09/2025.
//

#include "serialport.h"

#include <sys/fcntl.h>
#include <sys/termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifndef TIOCINQ
#ifdef FIONREAD
#define TIOCINQ FIONREAD
#else
#define TIOCINQ 0x541B
#endif
#endif


using namespace std;
using namespace UFC;

SerialPort::SerialPort(std::string port, int baud) :
    Logger("SerialPort"),
    m_port(port),
    m_baud(baud)
{
}

bool SerialPort::open()
{
    m_fd = ::open(m_port.c_str(), O_RDWR);// | O_NOCTTY | O_SYNC);
    if (m_fd < 0)
    {
        printf("SerialPort::open: Failed to open port %s: %s\n", m_port.c_str(), strerror(errno));
        return false;
    }

    struct termios tty;
    if (tcgetattr(m_fd, &tty) != 0)
    {
        log(ERROR, "open: tcgetattr: %s", strerror(errno));
        close();
        return false;
    }

    cfsetospeed(&tty, m_baud);
    cfsetispeed(&tty, m_baud);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit characters
    tty.c_iflag &= ~IGNBRK; // disable break processing
    tty.c_lflag = 0; // no signaling chars, no echo, no

    // canonical processing
    tty.c_oflag = 0; // no remapping, no delays
    tty.c_cc[VMIN] = 0; // read doesn't block
    tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls,

    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(m_fd, TCSANOW, &tty) != 0)
    {
        log(ERROR, "open: tcsetattr: %s", strerror(errno));
        close();
        return false;
    }

    return true;
}

void SerialPort::close()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

ssize_t SerialPort::write(const std::string &init_str)
{
    return ::write(m_fd, init_str.c_str(), init_str.length());
}

int SerialPort::available()
{
    if (m_buffer.find('\n') != std::string::npos)
    {
        // There's still at least 1 line in the buffer!
        return true;
    }

    int count = 0;
    int res = ioctl (m_fd, TIOCINQ, &count);
    if (res < 0)
    {
        return 0;
    }

    return count;
}

std::string SerialPort::readLine()
{
    char buffer[65536];
    ssize_t res;
    res = ::read(m_fd, buffer, 65536);
    if (res < 0)
    {
        return "";
    }

    string strbuffer(buffer, res);
    m_buffer += strbuffer;
    //log(DEBUG, "readString: buffer: <%s>", m_buffer.c_str());

    auto idx = m_buffer.find('\n');
    if (idx == string::npos)
    {
        // No complete line in buffer
        log(DEBUG, "readString:  -> No complete line found");
        return "";
    }

    string line = m_buffer.substr(0, idx);
    //log(DEBUG, "readString:  -> line: <%s>", line.c_str());
    m_buffer = m_buffer.substr(idx + 1);
    return line;
}

void SerialPort::flush()
{
    tcflush(m_fd, TCIFLUSH);
}
