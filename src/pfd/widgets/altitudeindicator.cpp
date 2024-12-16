//
// Created by Ian Parker on 23/01/2024.
//

#include "altitudeindicator.h"
#include "pfd/display.h"

using namespace std;
using namespace glm;

AltitudeIndicatorWidget::AltitudeIndicatorWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget( display, x, y, w, h)
{
}

void AltitudeIndicatorWidget::draw(UFC::AircraftState &state, std::shared_ptr<Cairo::Context> context)
{
    context->save();
    context->rectangle(0, 0, getWidth() - 20, getHeight());
    context->set_source_rgb(0.21, 0.21, 0.21);
    context->fill();

    auto surfaceHeight = getHeight();
    //m_altitudeSurface->clear(0xff333333);

    float altitude = state.altitude;
    //int offset = getDisplay()->getFont()->getPixelHeight() / 2;
    //int smallOffset = getDisplay()->getSmallFont()->getPixelHeight() / 2;

    int y = surfaceHeight / 2;

    int range = 700;
    char buf[50];
    for (int i = -range; i < range; i++)
    {
        // The altitude represented by this position, in hundreds of feet
        float relAltitude = altitude + ((float)i * 1.0f);
        float raltitude100 = relAltitude / 100.0f;

        // Position on screen
        float ry = (float)y - (float)(i * 0.5);

        if (raltitude100 >= 0 && glm::fract(raltitude100) < 0.01)
        {
            // Represents relAltitude whole 100 feet
            context->move_to(getWidth() - 30, ry);
            context->line_to(getWidth() - 20, ry);

            if (((int) raltitude100 % 5) == 0)// && (int)raltitude100 != actual100)
            {
                snprintf(buf, 50, "%03d", (int) raltitude100);
                context->set_source_rgb(1.0, 1.0, 1.0);
                getDisplay()->drawText(context, buf, 1, (int)ry + 6);
            }
        }
    }

    context->move_to(getWidth() - 20, 0);
    context->line_to(getWidth() - 20, getHeight());
    context->set_source_rgb(1, 1, 1);
    context->stroke();

    context->move_to(0, y - 10);
    context->line_to(getWidth() - 20, y - 10);
    context->line_to(getWidth() - 20, y - 25);
    context->line_to(getWidth(), y - 25);
    context->line_to(getWidth(), y + 25);
    context->line_to(getWidth() - 20, y + 25);
    context->line_to(getWidth() - 20, y + 10);
    context->line_to(0, y + 10);
    context->close_path();

    context->set_source_rgb(0, 0, 0);
    context->fill_preserve();

    context->set_source_rgb(1, 1, 1);
    context->stroke_preserve();
    context->save();
    context->clip_preserve();

    for (int i = - 2; i < 2; i++)
    {
        float relAltitude = altitude - ((float)i * 10.0f);
        float hundreds = floor(fmod((relAltitude / 10.0), 10.0f)) * 10.0f;
        snprintf(buf, 50, "%02d", (int)hundreds);
        float offset = (fmod(relAltitude, 10) / 10.0) * 16.0;
        getDisplay()->drawText(context, buf, getWidth() - 20, y + 4 + ((float)i * 16.0) + offset);
    }
        snprintf(buf, 50, "%03d", (int) altitude / 100);
        getDisplay()->drawText(context, buf, 4, y + 4);
    context->restore();


#if 0
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
#endif
    context->restore();
}
