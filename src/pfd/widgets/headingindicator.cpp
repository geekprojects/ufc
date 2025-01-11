//
// Created by Ian Parker on 23/01/2024.
//

#include "headingindicator.h"
#include "pfd/display.h"

using namespace std;

HeadingIndicatorWidget::HeadingIndicatorWidget(XPFlightDisplay* display, float x, float y, float w, float h)
    : FlightWidget( display, x, y, w, h)
{
}

void HeadingIndicatorWidget::wrapAngle(float& angle)
{
    while (angle < 0.0f || angle > 360.0f)
    {
        if (angle < 0.0f)
        {
            angle += 360.0f;
        }
        else if (angle > 360.0f)
        {
            angle -= 360.0f;
        }
    }
}

void HeadingIndicatorWidget::draw(UFC::AircraftState &state, const std::shared_ptr<Cairo::Context>& context)
{
    context->save();
    context->rectangle(0, 20, getWidth(), getHeight() - 20);
    context->set_source_rgb(0.29, 0.29, 0.29);
    context->fill_preserve();
    context->set_source_rgb(1, 1, 1);
    context->stroke();

    float halfAngleWidth = floor(((float)getWidth() / 1.5f) / m_pixelsPerDegree);
    float cx = (float)getWidth() / 2.0f;

    float heading = state.magHeading;
    //wrapAngle(heading);

    float relAngle = -halfAngleWidth;
    float offset = glm::fract(heading);

    context->set_font_face(getDisplay()->getFont());
    context->set_font_size(16);

    while (relAngle < halfAngleWidth)
    {
        float angle = heading + relAngle;
        wrapAngle(angle);

        float a = relAngle;
        if (angle > 0)
        {
            a -= offset;
        }
        float x = cx + (a * m_pixelsPerDegree);

        int iangle = (int)floor(angle);
        if ((iangle % 10) == 0)
        {
            char buf[50];
            snprintf(buf, 50, "%d", iangle / 10);
            Cairo::TextExtents extents;
            context->get_text_extents(buf, extents);
            context->move_to(x - (extents.width / 2.0), 50);
            context->show_text(buf);

            context->move_to(x, 20);
            context->line_to(x, 35);
            context->stroke();
        }
        else if ((iangle % 5) == 0)
        {
            context->move_to(x, 20);
            context->line_to(x, 30);
            context->stroke();
        }

        relAngle += 1.0f;
    }

    relAngle = state.magHeading - state.autopilot.heading;
    float bugX = cx - (relAngle * m_pixelsPerDegree);
    if (bugX >= 0 && bugX < getWidth())
    {
        context->move_to(bugX, 0);
        context->line_to(bugX, 40);
        context->stroke();
    }

    context->set_source_rgb(1.0, 1.0, 0.0);
    context->set_line_width(2);
    context->move_to(cx, 10);
    context->line_to(cx, 25);
    context->stroke();

    context->restore();
#if 0
    surface->blit(
        getX(),
        getY(),
        m_headingSurface.get(),
        m_width36 + (int)((float)m_width36 * (fmod(state.magHeading, 360.f) / 360.0f)) - (getWidth() / 2),
        0,
        getWidth(),
        getHeight());

    surface->drawRect(getX(), getY(), getWidth(), getHeight(), 0xffffffff);
    surface->drawLine(
        getX() + (getWidth() / 2),
        getY(),
        getX() + (getWidth() / 2),
        getY() + getHeight(),
        0xffffffff);

    wchar_t buf[50];
    swprintf(buf, 50, L"%0.2f", state.magHeading);
    getDisplay()->getSmallFont()->write(surface.get(), getX() + 10, getY() + getHeight() - getDisplay()->getSmallFont()->getPixelHeight(), buf, 0xffffffff);
#endif
}
