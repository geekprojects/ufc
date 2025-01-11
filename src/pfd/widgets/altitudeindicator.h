//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_ALTITUDEINDICATOR_H
#define XPFD_ALTITUDEINDICATOR_H

#include "widget.h"

class AltitudeIndicatorWidget : public FlightWidget
{
 private:
    //std::shared_ptr<Geek::Gfx::Surface> m_altitudeSurface;
    //std::shared_ptr<Geek::Gfx::Surface> m_altitudeHundredsSurface;

 public:
    AltitudeIndicatorWidget(XPFlightDisplay* display, float x, float y, float w, float h);
    ~AltitudeIndicatorWidget() override = default;

    void draw(UFC::AircraftState& state, const std::shared_ptr<Cairo::Context>& context) override;
};


#endif //XPFD_ALTITUDEINDICATOR_H
