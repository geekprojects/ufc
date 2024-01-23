//
// Created by Ian Parker on 23/01/2024.
//

#include "altitudeindicator.h"

#include <glm/glm.hpp>

using namespace std;
using namespace glm;
using namespace Geek;
using namespace Geek::Gfx;

AltitudeIndicatorWidget::AltitudeIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget( display, x, y, w, h)
{
    int surfaceHeight = m_height * 2;
    m_altitudeSurface = make_shared<Surface>(m_width, surfaceHeight, 4);
}

void AltitudeIndicatorWidget::draw(State &state, std::shared_ptr<Geek::Gfx::Surface> surface)
{
    int surfaceHeight = m_altitudeSurface->getHeight();
    m_altitudeSurface->clear(0xff333333);

    float altitude = state.altitude;
    int offset = m_display->getFont()->getPixelHeight() / 2;
    int smallOffset = m_display->getSmallFont()->getPixelHeight() / 2;

    int y = surfaceHeight / 2;

    int range = 700;
    wchar_t buf[50];
    for (int i = -range; i < range; i++)
    {
        // The altitude represented by this position, in hundreds of feet
        float relAltitude = altitude + (float)(i * 1.0f);
        float raltitude100 = relAltitude / 100.0f;

        // Position on screen
        float ry = (float)y - (float)(i * 0.5);

        //printf("%0.2f: altitude1000=%0.2f, ry=%0.2f\n", altitude, altitude1000, ry);

        if (raltitude100 >= 0 && glm::fract(raltitude100) < 0.01)
        {
            // Represents relAltitude whole 100 feet
            m_altitudeSurface->drawHorizontalLine(m_width - 10, (int)ry, 10, 0xffffffff);

            if (((int) raltitude100 % 5) == 0)// && (int)raltitude100 != actual100)
            {
                swprintf(buf, 50, L"%03d", (int) raltitude100);
                m_display->getFont()->write(m_altitudeSurface.get(), 13, (int)ry - offset, buf, 0xffffffff);
            }
        }
    }

    int hh = m_display->getSmallFont()->getPixelHeight() * 10;
    int hundredsWidth;
    int hundredsHeight;
    if (m_altitudeHundredsSurface == nullptr)
    {
        hundredsWidth = m_display->getSmallFont()->getPixelWidth(L"00") + 2;
        hundredsHeight = hh * 3;
        m_altitudeHundredsSurface = make_shared<Surface>(hundredsWidth, hundredsHeight, 4);
        m_altitudeHundredsSurface->clear(0xff000000);
        for (int i = 0; i < 30; i++)
        {
            swprintf(buf, 50, L"%02d", (100 - ((i * 10) % 100)) % 100);
            m_display->getSmallFont()->write(m_altitudeHundredsSurface.get(), 0, (i * m_display->getSmallFont()->getPixelHeight()), buf, 0xff00ff00);
        }
    }
    else
    {
        hundredsWidth = m_altitudeHundredsSurface->getWidth();
        hundredsHeight = m_altitudeHundredsSurface->getHeight();
    }


    int actualSpeedHeight = 30;
    m_altitudeSurface->drawRectFilled(10, y - (actualSpeedHeight / 2), m_width-10, actualSpeedHeight, 0xff000000);
    swprintf(buf, 50, L"%03d", (int) floor(altitude / 100));
    m_display->getFont()->write(m_altitudeSurface.get(), 13, (int)y - offset, buf, 0xff00ff00);

    float p = fmod(altitude, 100.0f) / 100.0f;
    m_altitudeSurface->blit(
        m_width - hundredsWidth,
        (int)y - (actualSpeedHeight / 2),
        m_altitudeHundredsSurface.get(),
        0,
        (p * (m_display->getSmallFont()->getPixelHeight() * -10)) + (hh * 2) - (smallOffset - 0),
        hundredsWidth,
        actualSpeedHeight
    );

    m_altitudeSurface->drawRect(10, y - (actualSpeedHeight / 2), m_width-10, actualSpeedHeight, 0xffFFFF00);

    surface->blit(
        m_x,
        m_y,
        m_altitudeSurface.get(),
        0,
        (surfaceHeight / 2) - (m_height / 2),
        m_width,
        m_height);
    surface->drawRect(m_x, m_y, m_width, m_height, 0xffffffff);

    surface->drawRectFilled(m_x, m_y, m_width, 30, 0xff000000);
    surface->drawRect(m_x, m_y, m_width, 30, 0xffffffff);

    swprintf(buf, 50, L"%05d", (int)altitude);
    m_display->getFont()->write(surface.get(), m_x + 10, m_y + 2, buf, 0xffffffff);

    surface->drawRectFilled(m_x, m_y + m_height - 30, m_width, 30, 0xff000000);
    surface->drawRect(m_x, m_y + m_height - 30, m_width, 30, 0xffffffff);
    swprintf(buf, 50, L"%+d", (int)state.verticalSpeed);
    m_display->getFont()->write(surface.get(), m_x + 10, m_y + m_height - 28 , buf, 0xffffffff);

    float bar = state.barometerHG * 1013.0f / 29.92f;
    if ((state.barometerHG * 100) == 2992)
    {
        swprintf(buf, 50, L"STD");
    }
    else
    {
        swprintf(buf, 50, L"QNH %d", (int)bar);
    }
    m_display->getSmallFont()->write(surface.get(), m_x + 2, m_y + m_height + 2 , buf, 0xffffffff);
}
