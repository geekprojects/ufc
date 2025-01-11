//
// Created by Ian Parker on 23/01/2024.
//

#include "speedindicator.h"
#include "pfd/display.h"

#include <glm/glm.hpp>

using namespace std;

constexpr float pixelsPerKn = 5.0f;

SpeedIndicatorWidget::SpeedIndicatorWidget(XPFlightDisplay* display, float x, float y, float w, float h)
    : FlightWidget(display, x, y, w, h)
{
    //int surfaceHeight = getHeight() * 2;
    //m_speedSurface = make_shared<Surface>(getWidth(), surfaceHeight, 4);
}

void SpeedIndicatorWidget::draw(UFC::AircraftState &state, const std::shared_ptr<Cairo::Context>& context)
{
    float tapeWidth = getWidth() * 0.6f;
    context->rectangle(10, 0, tapeWidth, getHeight());
    context->set_source_rgb(0.29, 0.29, 0.29);
    context->fill();

#if 0
    context->set_source_rgb(1.0, 1.0, 1.0);
    context->move_to(tapeWidth, 0);
    context->line_to(tapeWidth, getHeight());
    context->stroke();
#endif

    float speed = state.indicatedAirspeed;
    float offset = glm::fract(state.indicatedAirspeed);

    float y = getHeight() / 2.0f;

    char buf[50];
    float i = -100;
    while (i < 100)
    {
        float rspeed = speed + i;
        float ry = (float)y - (i * pixelsPerKn);
        if (rspeed >= 0 &&
            ((int)rspeed % 10) == 0 &&
            glm::fract(ry) < FLT_EPSILON )
        {
            ry += offset * pixelsPerKn;
            //m_speedSurface->drawHorizontalLine(getWidth() - 20, (int)ry, 10, 0xffffffff);
            context->move_to(tapeWidth - 15, ry);
            context->line_to(tapeWidth, ry);
            context->set_source_rgb(1.0, 1.0, 1.0);
            context->stroke();

            if (((int) rspeed % 20) == 0)
            {
                snprintf(buf, 50, "%03d", (int) rspeed);
                //getDisplay()->getFont()->write(m_speedSurface.get(), 10, (int)ry - offset, buf, 0xffffffff);
                context->set_source_rgb(1.0, 1.0, 1.0);
                context->set_font_face(getDisplay()->getFont());
                context->set_font_size(18);
                Cairo::TextExtents extents;
                context->get_text_extents(buf, extents);
                context->move_to(10, ry + (extents.height / 2.0f));
                context->show_text(buf);
            }
        }
        i += 0.25f;
    }

    context->set_source_rgb(1.0, 1.0, 0.0);
    drawArrow(context, tapeWidth + 5, y, 7);
    context->move_to(tapeWidth - 15, y);
    context->line_to(tapeWidth + 10, y);
    context->stroke();
    context->move_to(0, y);
    context->line_to(10, y);
    context->stroke();

    context->set_source_rgb(1.0, 1.0, 1.0);
    float apSpeedY = speedToY(state.autopilot.speed - speed);
    if (apSpeedY > -10 && apSpeedY < getHeight() - 10)
    {
        drawArrow(context, tapeWidth, apSpeedY, 10);
    }

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

float SpeedIndicatorWidget::speedToY(float rspeed) const
{
    float y = getHeight() / 2.0f;
    return y - (rspeed * pixelsPerKn);
}

void SpeedIndicatorWidget::drawArrow(const std::shared_ptr<Cairo::Context>& context, float x, float y, float size)
{
    context->move_to(x, y);
    context->line_to(x + size, y - size);
    context->line_to(x + size, y + size);
    context->fill();
}

