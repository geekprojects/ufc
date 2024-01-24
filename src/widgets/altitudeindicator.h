//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_ALTITUDEINDICATOR_H
#define XPFD_ALTITUDEINDICATOR_H

#include "widgets/widget.h"

class AltitudeIndicatorWidget : public FlightWidget
{
 private:
    std::shared_ptr<Geek::Gfx::Surface> m_altitudeSurface;
    std::shared_ptr<Geek::Gfx::Surface> m_altitudeHundredsSurface;

 public:
    AltitudeIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h);
    ~AltitudeIndicatorWidget() override = default;

    void draw(State& state, std::shared_ptr<Geek::Gfx::Surface> surface) override;
};


#endif //XPFD_ALTITUDEINDICATOR_H
