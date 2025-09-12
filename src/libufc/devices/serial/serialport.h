//
// Created by Ian Parker on 11/09/2025.
//

#ifndef UNIVERSALFLIGHTCONNECTOR_SERIALPORT_H
#define UNIVERSALFLIGHTCONNECTOR_SERIALPORT_H

#include <string>

#include "serial.h"
#include "ufc/logger.h"

class SerialPort : public UFC::Logger
{
    int m_fd = -1;
    std::string m_port;
    int m_baud;

    std::string m_buffer;

 public:
    SerialPort(std::string port, int baud);

    bool open();
    void close();
    [[nodiscard]] bool isOpen() const { return m_fd > 0; }

    ssize_t write(const std::string &init_str);

    int available();

    std::string readLine();

    void flush();
};


#endif //UNIVERSALFLIGHTCONNECTOR_SERIALPORT_H
