//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_WIDGET_H
#define XPFD_WIDGET_H

#include <geek/gfx-surface.h>

#include "state.h"

class XPFlightDisplay;

class FlightWidget
{
 private:
    XPFlightDisplay* m_display;
    int m_x;
    int m_y;
    int m_width;
    int m_height;

 public:
    FlightWidget(XPFlightDisplay* flightDisplay, int x, int y, int w, int h) :
        m_display(flightDisplay),
        m_x(x),
        m_y(y),
        m_width(w),
        m_height(h) {}
    virtual ~FlightWidget() = default;

    virtual void draw(State& state, std::shared_ptr<Geek::Gfx::Surface> surface) = 0;

    [[nodiscard]] XPFlightDisplay* getDisplay() const { return m_display; }
    [[nodiscard]] int getX() const { return m_x; }
    [[nodiscard]] int getY() const { return m_y; }
    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }
};

#endif //XPFD_WIDGET_H
