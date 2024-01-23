//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_HEADINGINDICATOR_H
#define XPFD_HEADINGINDICATOR_H

#include "display.h"

class HeadingIndicatorWidget : public FlightWidget
{
 private:
    std::shared_ptr<Geek::Gfx::Surface> m_headingSurface;

    const int m_spacing = 80;
    const int m_width36 = 36 * m_spacing;
    const int m_surfaceWidth = m_width36 * 3;

    void drawCompass();

 public:
    HeadingIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h);
    ~HeadingIndicatorWidget() override = default;

    void draw(State& state, std::shared_ptr<Geek::Gfx::Surface> surface) override;
};


#endif //XPFD_HEADINGINDICATOR_H
