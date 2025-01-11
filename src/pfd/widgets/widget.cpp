//
// Created by Ian Parker on 06/01/2025.
//

#include "widget.h"

using namespace std;
using namespace UFC;

void FlightWidget::draw(AircraftState& state, const shared_ptr<Cairo::Context>& context)
{
    for (auto const& widget : m_children)
    {
        auto childSurface = Cairo::Surface::create(
            context->get_target(),
            widget->getX(),
            widget->getY(),
            widget->getWidth(),
            widget->getHeight());
        auto childContext = Cairo::Context::create(childSurface);

        widget->draw(state, childContext);
#if 1
        childContext->set_source_rgb(1.0, 1.0, 1.0);
        childContext->rectangle(0, 0, widget->getWidth(), widget->getHeight());
        childContext->stroke();
#endif
    }
}
