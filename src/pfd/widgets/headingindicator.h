//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_HEADINGINDICATOR_H
#define XPFD_HEADINGINDICATOR_H

#include "widget.h"

class HeadingIndicatorWidget : public FlightWidget
{
    const float m_pixelsPerDegree = 8.0f;

 public:
    HeadingIndicatorWidget(XPFlightDisplay* display, float x, float y, float w, float h);
    ~HeadingIndicatorWidget() override = default;

    static void wrapAngle(float &angle);

    void draw(UFC::AircraftState& state, const std::shared_ptr<Cairo::Context>& context) override;
};


#endif //XPFD_HEADINGINDICATOR_H
