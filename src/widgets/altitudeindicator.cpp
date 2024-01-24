//
// Created by Ian Parker on 23/01/2024.
//

#include "altitudeindicator.h"
#include "display.h"

#include <glm/glm.hpp>

using namespace std;
using namespace glm;
using namespace Geek;
using namespace Geek::Gfx;

AltitudeIndicatorWidget::AltitudeIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget( display, x, y, w, h)
{
    int surfaceHeight = getHeight() * 2;
    m_altitudeSurface = make_shared<Surface>(getWidth(), surfaceHeight, 4);
}

void AltitudeIndicatorWidget::draw(State &state, std::shared_ptr<Geek::Gfx::Surface> surface)
{
    auto surfaceHeight = (int)m_altitudeSurface->getHeight();
    m_altitudeSurface->clear(0xff333333);

    float altitude = state.altitude;
    int offset = getDisplay()->getFont()->getPixelHeight() / 2;
    int smallOffset = getDisplay()->getSmallFont()->getPixelHeight() / 2;

    int y = surfaceHeight / 2;

    int range = 700;
    wchar_t buf[50];
    for (int i = -range; i < range; i++)
    {
        // The altitude represented by this position, in hundreds of feet
        float relAltitude = altitude + ((float)i * 1.0f);
        float raltitude100 = relAltitude / 100.0f;

        // Position on screen
        float ry = (float)y - (float)(i * 0.5);

        //printf("%0.2f: altitude1000=%0.2f, ry=%0.2f\n", altitude, altitude1000, ry);

        if (raltitude100 >= 0 && glm::fract(raltitude100) < 0.01)
        {
            // Represents relAltitude whole 100 feet
            m_altitudeSurface->drawHorizontalLine(getWidth() - 10, (int)ry, 10, 0xffffffff);

            if (((int) raltitude100 % 5) == 0)// && (int)raltitude100 != actual100)
            {
                swprintf(buf, 50, L"%03d", (int) raltitude100);
                getDisplay()->getFont()->write(m_altitudeSurface.get(), 13, (int)ry - offset, buf, 0xffffffff);
            }
        }
    }

    int hh = getDisplay()->getSmallFont()->getPixelHeight() * 10;
    int hundredsWidth;
    if (m_altitudeHundredsSurface == nullptr)
    {
        hundredsWidth = getDisplay()->getSmallFont()->getPixelWidth(L"00") + 2;
        int hundredsHeight = hh * 3;
        m_altitudeHundredsSurface = make_shared<Surface>(hundredsWidth, hundredsHeight, 4);
        m_altitudeHundredsSurface->clear(0xff000000);
        for (int i = 0; i < 30; i++)
        {
            swprintf(buf, 50, L"%02d", (100 - ((i * 10) % 100)) % 100);
            getDisplay()->getSmallFont()->write(m_altitudeHundredsSurface.get(), 0, (i * getDisplay()->getSmallFont()->getPixelHeight()), buf, 0xff00ff00);
        }
    }
    else
    {
        hundredsWidth = (int)m_altitudeHundredsSurface->getWidth();
    }

    int actualSpeedHeight = 30;
    m_altitudeSurface->drawRectFilled(10, y - (actualSpeedHeight / 2), getWidth()-10, actualSpeedHeight, 0xff000000);
    swprintf(buf, 50, L"%03d", (int) floor(altitude / 100));
    getDisplay()->getFont()->write(m_altitudeSurface.get(), 13, y - offset, buf, 0xff00ff00);

    float p = fmod(altitude, 100.0f) / 100.0f;
    m_altitudeSurface->blit(
        getWidth() - hundredsWidth,
        y - (actualSpeedHeight / 2),
        m_altitudeHundredsSurface.get(),
        0,
        (int)(p * (float)getDisplay()->getSmallFont()->getPixelHeight() * -10.0f) + (hh * 2) - (smallOffset - 0),
        hundredsWidth,
        actualSpeedHeight
    );

    m_altitudeSurface->drawRect(10, y - (actualSpeedHeight / 2), getWidth()-10, actualSpeedHeight, 0xffFFFF00);

    surface->blit(
        getX(),
        getY(),
        m_altitudeSurface.get(),
        0,
        (surfaceHeight / 2) - (getHeight() / 2),
        getWidth(),
        getHeight());
    surface->drawRect(getX(), getY(), getWidth(), getHeight(), 0xffffffff);

    surface->drawRectFilled(getX(), getY(), getWidth(), 30, 0xff000000);
    surface->drawRect(getX(), getY(), getWidth(), 30, 0xffffffff);

    swprintf(buf, 50, L"%05d", (int)altitude);
    getDisplay()->getFont()->write(surface.get(), getX() + 10, getY() + 2, buf, 0xffffffff);

    surface->drawRectFilled(getX(), getY() + getHeight() - 30, getWidth(), 30, 0xff000000);
    surface->drawRect(getX(), getY() + getHeight() - 30, getWidth(), 30, 0xffffffff);
    swprintf(buf, 50, L"%+d", (int)state.verticalSpeed);
    getDisplay()->getFont()->write(surface.get(), getX() + 10, getY() + getHeight() - 28 , buf, 0xffffffff);

    float bar = state.barometerHG * 1013.0f / 29.92f;
    if ((state.barometerHG * 100) == 2992)
    {
        swprintf(buf, 50, L"STD");
    }
    else
    {
        swprintf(buf, 50, L"QNH %d", (int)bar);
    }
    getDisplay()->getSmallFont()->write(surface.get(), getX() + 2, getY() + getHeight() + 2 , buf, 0xffffffff);
}
