//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_DATASOURCE_H
#define XPFD_DATASOURCE_H

#include "display.h"

class DataSource
{
 protected:
    XPFlightDisplay* m_display;

 public:
    explicit DataSource(XPFlightDisplay* display) : m_display(display) {}
    virtual ~DataSource() = default;

    virtual bool init() = 0;
    virtual void close() = 0;
    virtual bool update() = 0;
};

#endif //XPFD_DATASOURCE_H
