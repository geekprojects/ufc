//
// Created by Ian Parker on 23/01/2024.
//

#include "headingindicator.h"
#include "pfd/display.h"

using namespace std;

HeadingIndicatorWidget::HeadingIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget( display, x, y, w, h)
{
    //m_headingSurface = make_shared<Surface>(m_surfaceWidth, getHeight(), 4);
    drawCompass();
}

void HeadingIndicatorWidget::drawCompass()
{
#if 0
    wchar_t buf[50];

    m_headingSurface->clear(0xff333333);

    for (int i = 0; i < 360; i += 5)
    {
        if ((i % 10) == 0)
        {
            int n = i / 10;
            if (n == 0)
            {
                swprintf(buf, 50, L"N");
            }
            else if (n == 9)
            {
                swprintf(buf, 50, L"E");
            }
            else if (n == 18)
            {
                swprintf(buf, 50, L"S");
            }
            else if (n == 27)
            {
                swprintf(buf, 50, L"W");
            }
            else
            {
                swprintf(buf, 50, L"%d", n);
            }
            /* XXX TODO
            int w = getDisplay()->getFont()->width(buf);
            for (int j = 0; j < 3; j++)
            {
                int x = (n * m_spacing) + (j * 36 * m_spacing);
                getDisplay()->getFont()->write(m_headingSurface.get(), x - (w / 2), 25, buf, 0xffffffff);
                m_headingSurface->drawLine(x, 0, x, 20, 0xffffffff);
            }
            */
        }
        else if ((i % 5) == 0)
        {
            int n = i / 10;
            for (int j = 0; j < 3; j++)
            {
                int x = (m_spacing / 2) + (n * m_spacing) + (j * 36 * m_spacing);
                m_headingSurface->drawLine(x, 0, x, 10, 0xffffffff);
            }
        }
    }
#endif
}

void HeadingIndicatorWidget::draw(UFC::AircraftState &state, std::shared_ptr<Cairo::Context> context)
{
    context->save();
    context->rectangle(0, 0, getWidth(), getHeight());
    context->set_source_rgb(0.21, 0.21, 0.21);
    context->fill_preserve();
    context->set_source_rgb(1, 1, 1);
    context->stroke();

    float halfAngleWidth = ((getWidth() / (float)m_spacing) / 2.0f);

    float relAngle = -halfAngleWidth;
    float offset = fmod(state.magHeading, 0.5);
    while (relAngle < halfAngleWidth)
    {
        float angle = state.magHeading + relAngle - offset;
        float x = (halfAngleWidth + relAngle + offset) * m_spacing;

        context->move_to(x, 0);
        context->line_to(x, 10);
        context->stroke();

        char buf[50];
        snprintf(buf, 50, "%d", (int)(angle / 10.0));
        getDisplay()->drawText(context, buf, x, 15);

        relAngle += 0.5;
    }


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
