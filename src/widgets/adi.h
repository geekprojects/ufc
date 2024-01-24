//
// Created by Ian Parker on 23/01/2024.
//

#ifndef XPFD_ADI_H
#define XPFD_ADI_H

#include "widgets/widget.h"

class ADIWidget : public FlightWidget
{
 private:
    std::shared_ptr<Geek::Gfx::Surface> m_adiSurface = nullptr;

 public:
    ADIWidget(XPFlightDisplay* display, int x, int y, int w, int h);
    ~ADIWidget() override = default;

    void draw(State& state, std::shared_ptr<Geek::Gfx::Surface> surface) override;
};



#endif //XPFD_ADI_H
