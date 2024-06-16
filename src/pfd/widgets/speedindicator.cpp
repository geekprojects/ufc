//
// Created by Ian Parker on 23/01/2024.
//

#include "speedindicator.h"
#include "pfd/display.h"
#include "pfd/gfxutils.h"

using namespace std;
using namespace glm;
using namespace Geek;
using namespace Geek::Gfx;

SpeedIndicatorWidget::SpeedIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget(display, x, y, w, h)
{
    int surfaceHeight = getHeight() * 2;
    m_speedSurface = make_shared<Surface>(getWidth(), surfaceHeight, 4);
}

void SpeedIndicatorWidget::draw(UFC::AircraftState &state, std::shared_ptr<Geek::Gfx::Surface> surface)
{
    int surfaceHeight = getHeight() * 2;
    m_speedSurface->clear(0xff333333);

    float speed = state.indicatedAirspeed;
    int offset = getDisplay()->getFont()->getPixelHeight() / 2;

    int y = surfaceHeight / 2;

    wchar_t buf[50];
    float i = -100;
    while (i < 100)
    {
        float rspeed = speed + i;
        float ry = ((float)y - (i * 5.0f));
        if (rspeed >= 0 &&
            ((int)rspeed % 10) == 0 &&
            glm::fract(ry) < FLT_EPSILON)
        {
            m_speedSurface->drawHorizontalLine(getWidth() - 20, (int)ry, 10, 0xffffffff);
            if (((int) rspeed % 20) == 0)
            {
                swprintf(buf, 50, L"%03d", (int) rspeed);
                getDisplay()->getFont()->write(m_speedSurface.get(), 10, (int)ry - offset, buf, 0xffffffff);
            }
        }
        i += 0.25f;
    }

    vector<glm::ivec2> points;
    points.emplace_back(getWidth() - 10, y);
    points.emplace_back(getWidth(), y - 10);
    points.emplace_back(getWidth(), y + 10);
    drawFilledPolygon(m_speedSurface.get(), points, 0xffffffff);

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
}
