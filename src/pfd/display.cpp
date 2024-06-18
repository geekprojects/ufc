//
// Created by Ian Parker on 20/01/2024.
//

#include "display.h"

#include "pfd/widgets/adi.h"
#include "pfd/widgets/speedindicator.h"
#include "pfd/widgets/altitudeindicator.h"
#include "pfd/widgets/headingindicator.h"

#include <ufc/flightconnector.h>

#include <thread>

using namespace std;
using namespace UFC;
using namespace Geek;
using namespace Geek::Gfx;

XPFlightDisplay::XPFlightDisplay() = default;

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

    int ww;
    int wh;
    SDL_GetWindowSize(m_window, &ww, &wh);
    SDL_GetWindowSizeInPixels(m_window, &m_screenWidth, &m_screenHeight);

    float ratio = (float)m_screenWidth / (float)ww;

    m_fontManager = make_shared<FontManager>();
    m_fontManager->init();
    m_fontManager->scan("../data/fonts");
    m_font = m_fontManager->openFont("B612 Mono", "Regular", (int)(14 * ratio));
    m_fontSmall = m_fontManager->openFont("B612 Mono", "Regular", (int)(12 * ratio));
    m_largeFont = m_fontManager->openFont("B612 Mono", "Regular", (int)(36 * ratio));

    m_displaySurface = make_shared<Surface>(m_screenWidth, m_screenHeight, 4);

    int buttonHeight = 20;
    int instrumentAreaY = buttonHeight;
    int instrumentAreaHeight = m_screenHeight - (buttonHeight * 2);

    int adiWidth = (m_screenWidth / 6) * 2;
    int adiHeight = (instrumentAreaHeight / 3) * 2;
    int adiX = (m_screenWidth / 4) - (adiWidth / 2);
    int adiY = instrumentAreaY + ((instrumentAreaHeight / 2) - (adiHeight / 2));

    int indicatorHeight = (instrumentAreaHeight / 3) * 2;
    int indicatorWidth = ((m_screenWidth / 2) - adiWidth) / 3;
    int headingHeight = (instrumentAreaHeight - adiHeight) / 3;

    m_widgets.push_back(make_shared<ADIWidget>(this, adiX, adiY, adiWidth, adiHeight));
    m_widgets.push_back(make_shared<SpeedIndicatorWidget>(this, 5, adiY, indicatorWidth, indicatorHeight));
    m_widgets.push_back(make_shared<AltitudeIndicatorWidget>(this, (m_screenWidth / 2) - (indicatorWidth + 5), adiY, indicatorWidth, indicatorHeight));
    m_widgets.push_back(make_shared<HeadingIndicatorWidget>(this, adiX, instrumentAreaY + (instrumentAreaHeight - headingHeight), adiWidth, headingHeight));

    m_dataSource = m_flightConnector.openDefaultDataSource();
    m_dataSource->init();

    m_running = true;
    m_updateThread = new thread(&XPFlightDisplay::updateMain, this);

    return true;
}

void XPFlightDisplay::close()
{
    m_dataSource->close();

    SDL_DestroyWindow(m_window);

    m_running = false;
}

void XPFlightDisplay::draw()
{
    drawWidgets();

    SDL_Surface* sdlSurface = SDL_GetWindowSurface(m_window);
    SDL_ConvertPixels(
        m_screenWidth, m_screenHeight,
        SDL_PIXELFORMAT_ARGB8888, m_displaySurface->getData(), m_screenWidth * 4,
        sdlSurface->format->format, sdlSurface->pixels, sdlSurface->pitch);
    SDL_UpdateWindowSurface(m_window);
}

void XPFlightDisplay::drawWidgets()
{
    AircraftState state = m_flightConnector.getState();
    m_displaySurface->clear(0x0);

    for (auto const& widget : m_widgets)
    {
        widget->draw(state, m_displaySurface);
    }

    m_displaySurface->drawLine(m_screenWidth / 2, 0, m_screenWidth / 2, m_screenHeight, 0xffffffff);

    wchar_t buf[50];
    swprintf(buf, 50, L"COM1: %03.03f <-> %0.03f", state.comms.com1Hz / 100.0f, state.comms.com1StandbyHz / 100.0f);
    m_fontSmall->write(m_displaySurface.get(), (m_screenWidth / 2) + 10, 5, buf, 0xffffffff);
    swprintf(buf, 50, L"COM2: %03.03f <-> %0.03f", state.comms.com2Hz / 100.0f, state.comms.com2StandbyHz / 100.0f);
    m_fontSmall->write(m_displaySurface.get(), (m_screenWidth / 2) + 10, 20, buf, 0xffffffff);
    swprintf(buf, 50, L"NAV1: %03.03f <-> %0.03f", state.comms.nav1Hz / 100.0f, state.comms.nav1StandbyHz / 100.0f);
    m_fontSmall->write(m_displaySurface.get(), (m_screenWidth / 2) + 10, 35, buf, 0xffffffff);
    swprintf(buf, 50, L"NAV2: %03.03f <-> %0.03f", state.comms.nav2Hz / 100.0f, state.comms.nav2StandbyHz / 100.0f);
    m_fontSmall->write(m_displaySurface.get(), (m_screenWidth / 2) + 10, 50, buf, 0xffffffff);

    if (!state.connected)
    {
        uint32_t t = SDL_GetTicks();
        if ((t / 1000) % 2)
        {
            wstring str = L"No Connection";
            int w = m_largeFont->width(str);
            int h = m_largeFont->getPixelHeight();
            m_largeFont->write(
                m_displaySurface.get(),
                (m_screenWidth / 2) - (w / 2),
                (m_screenHeight / 2) - (h / 2),
                str,
                0xffffffff);
        }
    }
}

void XPFlightDisplay::updateMain()
{
    while (m_running)
    {
        m_dataSource->update();
        SDL_Delay(100);
    }
}

