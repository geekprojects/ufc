//
// Created by Ian Parker on 06/01/2025.
//

#ifndef ANNUNCIATORS_H
#define ANNUNCIATORS_H

#include "widget.h"

class AnnunciatorsWidget : public FlightWidget
{
 public:
    AnnunciatorsWidget(XPFlightDisplay* flightDisplay, float x, float y, float w, float h)
     : FlightWidget(flightDisplay, x, y, w, h)
    {
    }

    ~AnnunciatorsWidget() override = default;

    void draw(UFC::AircraftState& state, const std::shared_ptr<Cairo::Context>& surface) override;
};



#endif //ANNUNCIATORS_H
