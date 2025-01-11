//
// Created by Ian Parker on 06/01/2025.
//

#include "pfdwidget.h"

#include "adi.h"
#include "altitudeindicator.h"
#include "annunciators.h"
#include "headingindicator.h"
#include "speedindicator.h"
#include "vsi.h"

using namespace std;

PFDWidget::PFDWidget(XPFlightDisplay* display, float x, float y, float w, float h) : FlightWidget( display, x, y, w, h)
{
    addChild(make_shared<AnnunciatorsWidget>(display, 0, 0, w, h * 0.14));

    addChild(make_shared<SpeedIndicatorWidget>(display, 0, h * 0.175, w * 0.178, h * 0.66));
    addChild(make_shared<ADIWidget>(display, w * 0.178, h * 0.175, w * 0.503, h * 0.57));
    addChild(make_shared<AltitudeIndicatorWidget>(display, w * 0.734, h * 0.175, w * 0.172, h * 0.66));
    addChild(make_shared<VSIWidget>(display, w * 0.906, h * 0.175, w * 0.094, h * 0.66));

    addChild(make_shared<HeadingIndicatorWidget>(display, w * 0.178, h * 0.87, w * 0.503, h * 0.12));
}
