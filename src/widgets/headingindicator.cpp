//
// Created by Ian Parker on 23/01/2024.
//

#include "headingindicator.h"

using namespace std;
using namespace Geek;
using namespace Geek::Gfx;

HeadingIndicatorWidget::HeadingIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget( display, x, y, w, h)
{
    m_headingSurface = make_shared<Surface>(m_surfaceWidth, m_height, 4);
    drawCompass();
}

void HeadingIndicatorWidget::drawCompass()
{
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
            int w = m_display->getFont()->width(buf);
            for (int j = 0; j < 3; j++)
            {
                int x = (n * m_spacing) + (j * 36 * m_spacing);
                m_display->getFont()->write(m_headingSurface.get(), x - (w / 2), 25, buf, 0xffffffff);
                m_headingSurface->drawLine(x, 0, x, 20, 0xffffffff);
            }
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
}

void HeadingIndicatorWidget::draw(State &state, std::shared_ptr<Geek::Gfx::Surface> surface)
{
    surface->blit(
        m_x,
        m_y,
        m_headingSurface.get(),
        m_width36 + (m_width36 * (fmod(state.magHeading, 360.f) / 360.0f)) - (m_width / 2),
        0,
        m_width,
        m_height);

    surface->drawRect(m_x, m_y, m_width, m_height, 0xffffffff);
    surface->drawLine(
        m_x + (m_width / 2),
        m_y,
        m_x + (m_width / 2),
        m_y + m_height,
        0xffffffff);

    wchar_t buf[50];
    swprintf(buf, 50, L"%0.2f", state.magHeading);
    m_display->getSmallFont()->write(surface.get(), m_x + 10, m_y + m_height - m_display->getSmallFont()->getPixelHeight(), buf, 0xffffffff);
}
