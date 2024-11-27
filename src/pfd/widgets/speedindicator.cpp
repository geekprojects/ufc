//
// Created by Ian Parker on 23/01/2024.
//

#include "speedindicator.h"
#include "pfd/display.h"

#include <glm/glm.hpp>

using namespace std;

SpeedIndicatorWidget::SpeedIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget(display, x, y, w, h)
{
    //int surfaceHeight = getHeight() * 2;
    //m_speedSurface = make_shared<Surface>(getWidth(), surfaceHeight, 4);
}

void SpeedIndicatorWidget::draw(UFC::AircraftState &state, std::shared_ptr<Cairo::Context> context)
{
    context->rectangle(0, 0, getWidth() - 10, getHeight());
    context->set_source_rgb(0.21, 0.21, 0.21);
    context->fill();

    context->set_source_rgb(1.0, 1.0, 1.0);
    context->move_to(getWidth() - 10, 0);
    context->line_to(getWidth() - 10, getHeight());
    context->stroke();

    float speed = state.indicatedAirspeed;
    int offset = 8;//getDisplay()->getFont()->getPixelHeight() / 2;

    int y = getHeight() / 2;

    char buf[50];
    float i = -100;
    while (i < 100)
    {
        float rspeed = speed + i;
        float ry = ((float)y - (i * 5.0f));
        if (rspeed >= 0 &&
            ((int)rspeed % 10) == 0 &&
            glm::fract(ry) < FLT_EPSILON)
        {
            //m_speedSurface->drawHorizontalLine(getWidth() - 20, (int)ry, 10, 0xffffffff);
            context->move_to(getWidth() - 20, ry);
            context->line_to(getWidth() - 10, ry);
            context->set_source_rgb(1.0, 1.0, 1.0);
            context->stroke();

            if (((int) rspeed % 20) == 0)
            {
                snprintf(buf, 50, "%03d", (int) rspeed);
                //getDisplay()->getFont()->write(m_speedSurface.get(), 10, (int)ry - offset, buf, 0xffffffff);
                    context->set_source_rgb(1.0, 1.0, 1.0);
                    context->set_font_face(getDisplay()->getFont());
                    context->set_font_size(16);
                    context->move_to(10, ry + offset);
                    context->show_text(buf);
            }
        }
        i += 0.25f;
    }

    // Draw arrow
    context->move_to(getWidth() - 10, y);
    context->line_to(getWidth(), y - 10);
    context->line_to(getWidth(), y + 10);
    context->set_source_rgb(1.0, 1.0, 1.0);
    context->fill();
#if 0
    context->rectangle(0, 0, getWidth(), 30);
    context->set_source_rgb(0.0, 0.0, 0.0);
    context->fill_preserve();
    context->set_source_rgb(1.0, 1.0, 1.0);
    context->set_line_width(1);
    context->stroke();
    snprintf(buf, 50, "%03d", (int)speed);
    context->set_font_face(getDisplay()->getFont());
    context->set_font_size(16);
    context->move_to(10, 18);
    context->show_text(buf);
#endif


#if 0

    surface->blit(
        getX(),
        getY(),
        m_speedSurface.get(),
        0,
        (surfaceHeight / 2) - (getHeight() / 2),
        getWidth(),
        getHeight());
    surface->drawRect(getX(), getY(), getWidth(), getHeight(), 0xffffffff);

    surface->drawRectFilled(getX(), getY(), getWidth(), 30, 0xff000000);
    surface->drawRect(getX(), getY(), getWidth(), 30, 0xffffffff);

    swprintf(buf, 50, L"%03d", (int)speed);
    getDisplay()->getFont()->write(surface.get(), getX() + 10, getY() + 4, buf, 0xffffffff);

    surface->drawRectFilled(getX(), getY() + getHeight() - 30, getWidth(), 30, 0xff000000);
    surface->drawRect(getX(), getY() + getHeight() - 30, getWidth(), 30, 0xffffffff);

    swprintf(buf, 512, L"%0.3fM", state.indicatedMach);
    getDisplay()->getFont()->write(surface.get(), 7, getY() + getHeight() - 28 , buf, 0xffffffff);
#endif
}
