//
// Created by Ian Parker on 23/01/2024.
//

#include "speedindicator.h"
#include "gfxutils.h"

using namespace std;
using namespace glm;
using namespace Geek;
using namespace Geek::Gfx;

SpeedIndicatorWidget::SpeedIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget(display, x, y, w, h)
{
    int surfaceHeight = m_height * 2;
    m_speedSurface = make_shared<Surface>(m_width, surfaceHeight, 4);
}

void SpeedIndicatorWidget::draw(State &state, std::shared_ptr<Geek::Gfx::Surface> surface)
{
    int surfaceHeight = m_height * 2;
    m_speedSurface->clear(0xff333333);

    float speed = state.indicatedAirspeed;
    int offset = m_display->getFont()->getPixelHeight() / 2;

    int y = surfaceHeight / 2;

    wchar_t buf[50];
    for (float i = -100; i < 100; i += 0.25)
    {
        float rspeed = speed + i;
        float ry = ((float)y - (i * 5.0f));
        if (rspeed >= 0 && ((int)rspeed % 10) == 0 && (ry - floor(ry)) < FLT_EPSILON)
        {
            m_speedSurface->drawHorizontalLine(m_width - 20, (int)ry, 10, 0xffffffff);
            if (((int) rspeed % 20) == 0)
            {
                swprintf(buf, 50, L"%03d", (int) rspeed);
                m_display->getFont()->write(m_speedSurface.get(), 10, (int)ry - offset, buf, 0xffffffff);
            }
        }
    }

    vector<glm::ivec2> points;
    points.push_back(ivec2(m_width - 10, y));
    points.push_back(ivec2(m_width, y - 10));
    points.push_back(ivec2(m_width, y + 10));
    drawFilledPolygon(m_speedSurface.get(), points, 0xffffffff);

    surface->blit(
        m_x,
        m_y,
        m_speedSurface.get(),
        0,
        (surfaceHeight / 2) - (m_height / 2),
        m_width,
        m_height);
    surface->drawRect(m_x, m_y, m_width, m_height, 0xffffffff);

    surface->drawRectFilled(m_x, m_y, m_width, 30, 0xff000000);
    surface->drawRect(m_x, m_y, m_width, 30, 0xffffffff);

    swprintf(buf, 50, L"%03d", (int)speed);
    m_display->getFont()->write(surface.get(), m_x + 10, m_y + 4, buf, 0xffffffff);

    surface->drawRectFilled(m_x, m_y + m_height - 30, m_width, 30, 0xff000000);
    surface->drawRect(m_x, m_y + m_height - 30, m_width, 30, 0xffffffff);

    swprintf(buf, 512, L"%0.3fM", state.indicatedMach);
    m_display->getFont()->write(surface.get(), 7, m_y + m_height - 28 , buf, 0xffffffff);
}
