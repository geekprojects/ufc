//
// Created by Ian Parker on 06/01/2025.
//

#ifndef VSI_H
#define VSI_H
#include "widget.h"


class VSIWidget : public FlightWidget
{
 public:
    VSIWidget(XPFlightDisplay* flightDisplay, float x, float y, float w, float h)
     : FlightWidget(flightDisplay, x, y, w, h)
    { }
    ~VSIWidget() override = default;

    void draw(UFC::AircraftState& state, const std::shared_ptr<Cairo::Context>& surface) override;
};



#endif //VSI_H
