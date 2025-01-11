//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_WIDGET_H
#define XPFD_WIDGET_H

#include <ufc/aircraftstate.h>

#include <cairomm/cairomm.h>

class XPFlightDisplay;

class FlightWidget
{
 private:
    XPFlightDisplay* m_display;
    FlightWidget* m_parent = nullptr;
    float m_x;
    float m_y;
    float m_width;
    float m_height;

    std::vector<std::shared_ptr<FlightWidget>> m_children;

 protected:
    void addChild(const std::shared_ptr<FlightWidget>& child)
    {
        m_children.push_back(child);
        child->m_parent = this;
    }

 public:
    FlightWidget(XPFlightDisplay* flightDisplay, float x, float y, float w, float h) :
        m_display(flightDisplay),
        m_x(x),
        m_y(y),
        m_width(w),
        m_height(h) {}
    virtual ~FlightWidget() = default;

    virtual void draw(UFC::AircraftState& state, const std::shared_ptr<Cairo::Context>& surface);

    [[nodiscard]] XPFlightDisplay* getDisplay() const { return m_display; }
    [[nodiscard]] FlightWidget* getParent() const { return m_parent; }
    [[nodiscard]] float getX() const { return m_x; }
    [[nodiscard]] float getY() const { return m_y; }
    [[nodiscard]] float getWidth() const { return m_width; }
    [[nodiscard]] float getHeight() const { return m_height; }
};

#endif //XPFD_WIDGET_H
