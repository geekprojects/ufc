//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_ADI_H
#define XPFD_ADI_H

#include "widget.h"

class ADIWidget : public FlightWidget
{
 private:
    [[nodiscard]] float pitchToY(float pitch) const;

 public:
    ADIWidget(XPFlightDisplay* display, float x, float y, float w, float h);
    ~ADIWidget() override = default;

    void draw(UFC::AircraftState& state, const std::shared_ptr<Cairo::Context>& context) override;
};



#endif //XPFD_ADI_H
