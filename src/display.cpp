//
// Created by Ian Parker on 20/01/2024.
//

#include "display.h"
#include "xplane.h"

#include <thread>
#include <mutex>

#include "widgets/adi.h"
#include "widgets/speedindicator.h"
#include "widgets/altitudeindicator.h"
#include "widgets/headingindicator.h"

using namespace std;
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
    m_fontManager->scan("../data");
    m_font = m_fontManager->openFont("B612 Mono", "Regular", (int)(14 * ratio));
    m_fontSmall = m_fontManager->openFont("B612 Mono", "Regular", (int)(12 * ratio));
    m_largeFont = m_fontManager->openFont("B612 Mono", "Regular", (int)(36 * ratio));

    m_displaySurface = make_shared<Surface>(m_screenWidth, m_screenHeight, 4);

    int adiWidth = (m_screenWidth / 6) * 2;
    int adiHeight = (m_screenHeight / 3) * 2;
    int adiX = (m_screenWidth / 4) - (adiWidth / 2);
    int adiY = (m_screenHeight / 2) - (adiHeight / 2);

    int indicatorHeight = (m_screenHeight / 3) * 2;
    int indicatorWidth = ((m_screenWidth / 2) - adiWidth) / 3;//(m_screenWidth - adiWidth) / 3;
    int headingHeight = (m_screenHeight - adiHeight) / 3;

    m_adiWidget = make_shared<ADIWidget>(this, adiX, adiY, adiWidth, adiHeight);
    m_speedIndicatorWidget = make_shared<SpeedIndicatorWidget>(this, 5, adiY, indicatorWidth, indicatorHeight);
    m_altitudeIndicatorWidget = make_shared<AltitudeIndicatorWidget>(this, (m_screenWidth / 2) - (indicatorWidth + 5), adiY, indicatorWidth, indicatorHeight);
    m_headingIndicatorWidget = make_shared<HeadingIndicatorWidget>(this, adiX, m_screenHeight - headingHeight, adiWidth, headingHeight);

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


void XPFlightDisplay::draw()
{
    m_updateMutex.lock();
    m_displaySurface->clear(0x0);


    m_adiWidget->draw(m_state, m_displaySurface);
    m_speedIndicatorWidget->draw(m_state, m_displaySurface);
    m_altitudeIndicatorWidget->draw(m_state, m_displaySurface);
    m_headingIndicatorWidget->draw(m_state, m_displaySurface);

    m_displaySurface->drawLine(m_screenWidth / 2, 0, m_screenWidth / 2, m_screenHeight, 0xffffffff);

    if (!m_state.connected)
    {
        uint32_t t = SDL_GetTicks();
        if ((t / 1000) % 2)
        {
            wstring str = L"No Connection";
            int w = m_largeFont->width(str);
            int h = m_largeFont->getPixelHeight();
            m_largeFont->write(m_displaySurface.get(), (m_screenWidth / 2) - (w / 2), (m_screenHeight / 2) - (h / 2), str, 0xffffffff);
        }
    }

    m_updateMutex.unlock();

    SDL_Surface* sdlSurface = SDL_GetWindowSurface(m_window);
    SDL_ConvertPixels(
        m_screenWidth, m_screenHeight,
        SDL_PIXELFORMAT_ARGB8888, m_displaySurface->getData(), m_screenWidth * 4,
        sdlSurface->format->format, sdlSurface->pixels, sdlSurface->pitch);
    SDL_UpdateWindowSurface(m_window);
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
    scoped_lock lock(m_updateMutex);

    m_state.connected = true;
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
}

