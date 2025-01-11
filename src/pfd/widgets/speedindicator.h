//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_SPEEDINDICATOR_H
#define XPFD_SPEEDINDICATOR_H

#include "widget.h"

class SpeedIndicatorWidget : public FlightWidget
{
 private:
    //std::shared_ptr<Geek::Gfx::Surface> m_speedSurface;

    float speedToY(float rspeed) const;

    void drawArrow(const std::shared_ptr<Cairo::Context>& context, float x, float y, float size);

 public:
    SpeedIndicatorWidget(XPFlightDisplay* display, float x, float y, float w, float h);
    ~SpeedIndicatorWidget() override = default;

    void draw(UFC::AircraftState& state, const std::shared_ptr<Cairo::Context>& context) override;
};

#endif //XPFD_SPEEDINDICATOR_H
