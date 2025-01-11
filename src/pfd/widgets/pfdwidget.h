//
// Created by Ian Parker on 06/01/2025.
//

#ifndef PFDWIDGET_H
#define PFDWIDGET_H

#include "widget.h"

class HeadingIndicatorWidget;
class AltitudeIndicatorWidget;
class SpeedIndicatorWidget;
class ADIWidget;

class PFDWidget : public FlightWidget
{
 public:
    PFDWidget(XPFlightDisplay* display, float x, float y, float w, float h);
    ~PFDWidget() override = default;
};

#endif //PFDWIDGET_H
