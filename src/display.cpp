//
// Created by Ian Parker on 20/01/2024.
//

#include "display.h"
#include "xplane.h"
#include "gfxutils.h"

#include <glm/glm.hpp>

#include <thread>

using namespace std;
using namespace glm;
using namespace Geek;
using namespace Geek::Gfx;

XPFlightDisplay::XPFlightDisplay()
{

}

bool XPFlightDisplay::init()
{
    int res;
    res = SDL_Init(SDL_INIT_EVERYTHING);
    if (res != 0)
    {
        return false;
    }

    m_window = SDL_CreateWindow(
        "XP Flight Display",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 600,
        //600, 1024,
        SDL_WINDOW_OPENGL /*| SDL_WINDOW_ALLOW_HIGHDPI*/
    );

    //SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    int ww;
    int wh;
    SDL_GetWindowSize(m_window, &ww, &wh);
    SDL_GetWindowSizeInPixels(m_window, &m_screenWidth, &m_screenHeight);

    float ratio = (float)m_screenWidth / (float)ww;

    m_fontManager = new FontManager();
    m_fontManager->init();
    m_fontManager->scan("../data");
    m_font = m_fontManager->openFont("B612 Mono", "Regular", 14 * ratio);
    m_fontSmall = m_fontManager->openFont("B612 Mono", "Regular", 12 * ratio);
    m_largeFont = m_fontManager->openFont("B612 Mono", "Regular", 48 * ratio);

    m_displaySurface = new Surface(m_screenWidth, m_screenHeight, 4);

    m_client = new XPlaneClient();
    m_client->connect();

    m_running = true;
    m_updateThread = new thread(&XPFlightDisplay::updateMain, this);

    return true;
}

void XPFlightDisplay::close()
{
    SDL_DestroyWindow(m_window);
    m_client->disconnect();
    m_running = false;
}

glm::ivec2 rotate(ivec2 centre, ivec2 pos, float angle)
{
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return centre + ivec2(
        (int) (((float) pos.x * cosA) + ((float) pos.y * sinA)),
        (int) (((float) pos.x * -sinA) + ((float) pos.y * cosA)));
}

void XPFlightDisplay::draw()
{
    m_updateMutex.lock();
    m_displaySurface->clear(0x0);

    /*
    Position position;
    bool res = false;
    //res = m_client->getPosition(position);
    if (!res)
    {
        m_missedData++;
        if (m_missedData > 5)
        {
            uint32_t t = SDL_GetTicks();
            if ((t / 1000) % 2)
            {
                m_largeFont->write(m_displaySurface, 50, 50, L"No Connection to X-Plane", 0xffffffff);
            }
        }
        position = m_lastPosition;
    }
    else
    {
        m_lastPosition = position;
        m_missedData = 0;
    }
     */
    int adiWidth = (m_screenWidth / 6) * 2;
    int adiHeight = (m_screenHeight / 3) * 2;

    //int adiX = (m_screenWidth / 2) - (adiWidth / 2);
    int adiX = (m_screenWidth / 4) - (adiWidth / 2);
    int adiY = (m_screenHeight / 2) - (adiHeight / 2);

    drawADI(adiX, adiY, adiWidth, adiHeight);

    int indicatorHeight = (m_screenHeight / 3) * 2;
    int indicatorWidth = ((m_screenWidth / 2) - adiWidth) / 3;//(m_screenWidth - adiWidth) / 3;
    drawSpeedIndicator(5, adiY, indicatorWidth, indicatorHeight);
    drawAltitudeIndicator((m_screenWidth / 2) - (indicatorWidth + 5), adiY, indicatorWidth, indicatorHeight);

    int headingHeight = (m_screenHeight - adiHeight) / 3;
    drawHeading(adiX, m_screenHeight - headingHeight, adiWidth, headingHeight);

    m_displaySurface->drawLine(m_screenWidth / 2, 0, m_screenWidth / 2, m_screenHeight, 0xffffffff);

    m_updateMutex.unlock();

    SDL_Surface* sdlSurface = SDL_GetWindowSurface(m_window);
    SDL_ConvertPixels(
        m_screenWidth, m_screenHeight,
        SDL_PIXELFORMAT_ARGB8888, m_displaySurface->getData(), m_screenWidth * 4,
        sdlSurface->format->format, sdlSurface->pixels, sdlSurface->pitch);
    SDL_UpdateWindowSurface(m_window);
}

//float roll = 0.0;
void XPFlightDisplay::drawADI(
    int adiX,
    int adiY,
    int adiWidth,
    int adiHeight)
{
    if (m_adiSurface == nullptr)
    {
        m_adiSurface = new Surface(adiWidth, adiHeight, 4);
    }
    m_adiSurface->clear(0xff0088FF);

    float roll = radians(m_state.roll);
    float pitch = m_state.pitch;

    int halfWidth = adiWidth / 2;
    int halfHeight = adiHeight / 2;

    ivec2 centre(halfWidth, halfHeight);

    int pitchY = halfHeight + (int) ((float) adiHeight * pitch / 44.0f);

    ivec2 horizonCentre = ivec2(halfWidth, pitchY);
    ivec2 horizon0 = rotate(horizonCentre, ivec2(-halfWidth * 2, 0), roll);
    ivec2 horizon1 = rotate(horizonCentre, ivec2(halfWidth * 2, 0), roll);

    // Draw ground
    {
        vector<ivec2> points;
        points.push_back(horizon0);
        points.push_back(horizon1);
        points.push_back(ivec2(adiWidth - 1, adiHeight - 1));
        points.push_back(ivec2(0, adiHeight - 1));
        drawFilledPolygon(m_adiSurface, points, 0xff884400);
    }

    // Draw line at horizon
    {
        vector<ivec2> points;
        points.push_back(horizon0 + ivec2(0, -1));
        points.push_back(horizon1 + ivec2(0, -1));
        points.push_back(horizon1 + ivec2(0, 1));
        points.push_back(horizon0 + ivec2(0, 1));
        drawFilledPolygon(m_adiSurface, points, 0xffffffff);
    }

    // Draw wing thing
    //int wwidth = adiWidth / 4;
    {
        vector<ivec2> points;
        points.push_back(centre + ivec2(-130, -5));
        points.push_back(centre + ivec2(-40, -5));
        points.push_back(centre + ivec2(-40, 15));
        points.push_back(centre + ivec2(-50, 15));
        points.push_back(centre + ivec2(-50, 5));
        points.push_back(centre + ivec2(-130, 5));
        drawPolygon(m_adiSurface, points, true, 0xffffffff, true, 0xff000000);
    }
    {
        vector<ivec2> points;
        points.push_back(centre + ivec2(130, -5));
        points.push_back(centre + ivec2(40, -5));
        points.push_back(centre + ivec2(40, 15));
        points.push_back(centre + ivec2(50, 15));
        points.push_back(centre + ivec2(50, 5));
        points.push_back(centre + ivec2(130, 5));
        drawPolygon(m_adiSurface, points, true, 0xffffffff, true, 0xff000000);
    }

    m_displaySurface->blit(adiX, adiY, m_adiSurface);
}

void XPFlightDisplay::drawSpeedIndicator(int drawX, int drawY, int drawWidth, int drawHeight)
{
    int surfaceHeight = drawHeight * 2;
    if (m_speedSurface == nullptr)
    {
        m_speedSurface = new Surface(drawWidth, surfaceHeight, 4);
    }
    m_speedSurface->clear(0xff333333);

    float speed = m_state.indicatedAirspeed;
    int offset = m_font->getPixelHeight() / 2;

    int y = surfaceHeight / 2;

    wchar_t buf[50];
    for (float i = -100; i < 100; i += 0.25)
    {
        float rspeed = speed + i;
        float ry = ((float)y - (i * 5.0f));
        if (rspeed >= 0 && ((int)rspeed % 10) == 0 && (ry - floor(ry)) < FLT_EPSILON)
        {
                m_speedSurface->drawHorizontalLine(drawWidth - 20, (int)ry, 10, 0xffffffff);
                if (((int) rspeed % 20) == 0)
                {
                    swprintf(buf, 50, L"%03d", (int) rspeed);
                    m_font->write(m_speedSurface, 10, (int)ry - offset, buf, 0xffffffff);
                }
        }
    }

    vector<glm::ivec2> points;
    points.push_back(ivec2(drawWidth - 10, y));
    points.push_back(ivec2(drawWidth, y - 10));
    points.push_back(ivec2(drawWidth, y + 10));
    drawFilledPolygon(m_speedSurface, points, 0xffffffff);

    m_displaySurface->blit(
        drawX,
        drawY,
        m_speedSurface,
        0,
        (surfaceHeight / 2) - (drawHeight / 2),
        drawWidth,
        drawHeight);
    m_displaySurface->drawRect(drawX, drawY, drawWidth, drawHeight, 0xffffffff);

    m_displaySurface->drawRectFilled(drawX, drawY, drawWidth, 30, 0xff000000);
    m_displaySurface->drawRect(drawX, drawY, drawWidth, 30, 0xffffffff);

    swprintf(buf, 50, L"%03d", (int)speed);
    m_font->write(m_displaySurface, drawX + 10, drawY + 4, buf, 0xffffffff);

    m_displaySurface->drawRectFilled(drawX, drawY + drawHeight - 30, drawWidth, 30, 0xff000000);
    m_displaySurface->drawRect(drawX, drawY + drawHeight - 30, drawWidth, 30, 0xffffffff);

    swprintf(buf, 512, L"%0.3fM", m_state.indicatedMach);
    m_font->write(m_displaySurface, 7, drawY + drawHeight - 28 , buf, 0xffffffff);
}

void XPFlightDisplay::drawAltitudeIndicator(int drawX, int drawY, int drawWidth, int drawHeight)
{
    int surfaceHeight = drawHeight * 2;
    if (m_altitudeSurface == nullptr)
    {
        m_altitudeSurface = new Surface(drawWidth, surfaceHeight, 4);
    }
    m_altitudeSurface->clear(0xff333333);

    float altitude = m_state.altitude;
    int offset = m_font->getPixelHeight() / 2;
    int smallOffset = m_fontSmall->getPixelHeight() / 2;

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
            m_altitudeSurface->drawHorizontalLine(drawWidth - 10, (int)ry, 10, 0xffffffff);

            if (((int) raltitude100 % 5) == 0)// && (int)raltitude100 != actual100)
            {
                swprintf(buf, 50, L"%03d", (int) raltitude100);
                m_font->write(m_altitudeSurface, 13, (int)ry - offset, buf, 0xffffffff);
            }
        }
    }

    int hh = m_fontSmall->getPixelHeight() * 10;
    int hundredsWidth;
    int hundredsHeight;
    if (m_altitudeHundredsSurface == nullptr)
    {
        hundredsWidth = m_fontSmall->getPixelWidth(L"00") + 2;
        hundredsHeight = hh * 3;
        m_altitudeHundredsSurface = new Surface(hundredsWidth, hundredsHeight, 4);
        m_altitudeHundredsSurface->clear(0xff000000);
        for (int i = 0; i < 30; i++)
        {
            swprintf(buf, 50, L"%02d", (100 - ((i * 10) % 100)) % 100);
            m_fontSmall->write(m_altitudeHundredsSurface, 0, (i * m_fontSmall->getPixelHeight()), buf, 0xff00ff00);
        }
    }
    else
    {
        hundredsWidth = m_altitudeHundredsSurface->getWidth();
        hundredsHeight = m_altitudeHundredsSurface->getHeight();
    }


    int actualSpeedHeight = 30;
    m_altitudeSurface->drawRectFilled(10, y - (actualSpeedHeight / 2), drawWidth-10, actualSpeedHeight, 0xff000000);
    swprintf(buf, 50, L"%03d", (int) floor(altitude / 100));
    m_font->write(m_altitudeSurface, 13, (int)y - offset, buf, 0xff00ff00);

    float p = fmod(altitude, 100.0f) / 100.0f;
    m_altitudeSurface->blit(
        drawWidth - hundredsWidth,
        (int)y - (actualSpeedHeight / 2),
        m_altitudeHundredsSurface,
        0,
        (p * (m_fontSmall->getPixelHeight() * -10)) + (hh * 2) - (smallOffset - 0),
        hundredsWidth,
        actualSpeedHeight
        );

    m_altitudeSurface->drawRect(10, y - (actualSpeedHeight / 2), drawWidth-10, actualSpeedHeight, 0xffFFFF00);

    m_displaySurface->blit(
        drawX,
        drawY,
        m_altitudeSurface,
        0,
        (surfaceHeight / 2) - (drawHeight / 2),
        drawWidth,
        drawHeight);
    m_displaySurface->drawRect(drawX, drawY, drawWidth, drawHeight, 0xffffffff);

    m_displaySurface->drawRectFilled(drawX, drawY, drawWidth, 30, 0xff000000);
    m_displaySurface->drawRect(drawX, drawY, drawWidth, 30, 0xffffffff);

    swprintf(buf, 50, L"%05d", (int)altitude);
    m_font->write(m_displaySurface, drawX + 10, drawY + 2, buf, 0xffffffff);

    m_displaySurface->drawRectFilled(drawX, drawY + drawHeight - 30, drawWidth, 30, 0xff000000);
    m_displaySurface->drawRect(drawX, drawY + drawHeight - 30, drawWidth, 30, 0xffffffff);
    swprintf(buf, 50, L"%+d", (int)m_state.verticalSpeed);
    m_font->write(m_displaySurface, drawX + 10, drawY + drawHeight - 28 , buf, 0xffffffff);

    float bar = m_state.barometerHG * 1013.0f / 29.92f;
    if ((m_state.barometerHG * 100) == 2992)
    {
        swprintf(buf, 50, L"STD");
    }
    else
    {
    swprintf(buf, 50, L"QNH %d", (int)bar);
    }
    m_fontSmall->write(m_displaySurface, drawX + 2, drawY + drawHeight + 2 , buf, 0xffffffff);
}

void XPFlightDisplay::drawHeading(int drawX, int drawY, int drawWidth, int drawHeight)
{
    wchar_t buf[50];

    int spacing = 80;
    int width36 = 36 * spacing;
    int surfaceWidth = width36 * 3;
    if (m_headingSurface == nullptr)
    {
        m_headingSurface = new Surface(surfaceWidth, drawHeight, 4);
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
                int w = m_font->width(buf);
                for (int j = 0; j < 3; j++)
                {
                    int x = (n * spacing) + (j * 36 * spacing);
                    m_font->write(m_headingSurface, x - (w / 2), 25, buf, 0xffffffff);
                    m_headingSurface->drawLine(x, 0, x, 20, 0xffffffff);
                }
            }
            else if ((i % 5) == 0)
            {
                int n = i / 10;
                for (int j = 0; j < 3; j++)
                {
                    int x = (spacing / 2) + (n * spacing) + (j * 36 * spacing);
                    m_headingSurface->drawLine(x, 0, x, 10, 0xffffffff);
                }
            }
        }
    }

    //int x = surfaceWidth / 2;

    m_displaySurface->blit(
        drawX,
        drawY,
        m_headingSurface,
        width36 + (width36 * (fmod(m_state.magHeading, 360.f) / 360.0f)) - (drawWidth / 2),
        0,
        drawWidth,
        drawHeight);

    m_displaySurface->drawRect(drawX, drawY, drawWidth, drawHeight, 0xffffffff);
    m_displaySurface->drawLine(
        drawX + (drawWidth / 2),
        drawY,
        drawX + (drawWidth / 2),
        drawY + drawHeight,
        0xffffffff);

    swprintf(buf, 50, L"%0.2f", m_state.magHeading);
    m_fontSmall->write(m_displaySurface, drawX + 10, drawY + drawHeight - m_fontSmall->getPixelHeight(), buf, 0xffffffff);

    //m_state.magHeading += 0.1;
}

void XPFlightDisplay::updateMain()
{
    vector<pair<int, string>> datarefs;
    datarefs.push_back(make_pair(1, "sim/flightmodel/position/indicated_airspeed"));
    datarefs.push_back(make_pair(2, "sim/flightmodel/position/theta"));
    datarefs.push_back(make_pair(3, "sim/flightmodel/position/phi"));
    datarefs.push_back(make_pair(4, "sim/cockpit2/gauges/indicators/altitude_ft_pilot"));
    datarefs.push_back(make_pair(5, "sim/cockpit2/gauges/indicators/vvi_fpm_pilot"));
    datarefs.push_back(make_pair(6, "sim/cockpit2/gauges/indicators/mach_pilot"));
    datarefs.push_back(make_pair(7, "sim/flightmodel/position/mag_psi"));
    datarefs.push_back(make_pair(8, "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot"));

    m_client->streamDataRefs(datarefs, [this](map<int, float> values)
    {
        updateState(values);
    });
}

void XPFlightDisplay::updateState(std::map<int, float> values)
{
    m_updateMutex.lock();
    for (auto it : values)
    {
        float value = it.second;
        switch (it.first)
        {
            case 1:
                m_state.indicatedAirspeed = value;
                break;
            case 2:
                m_state.pitch = value;
                break;
            case 3:
                m_state.roll = value;
                break;
            case 4:
                m_state.altitude = value;
                break;
            case 5:
                m_state.verticalSpeed = value;
                break;
            case 6:
                m_state.indicatedMach = value;
                break;
            case 7:
                m_state.magHeading = value;
                break;
            case 8:
                m_state.barometerHG = value;
                break;
        }
    }
    m_updateMutex.unlock();
}

