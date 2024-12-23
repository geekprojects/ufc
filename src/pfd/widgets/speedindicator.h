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

 public:
    SpeedIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h);
    ~SpeedIndicatorWidget() override = default;

    void draw(UFC::AircraftState& state, std::shared_ptr<Cairo::Context> context) override;
};

#endif //XPFD_SPEEDINDICATOR_H
