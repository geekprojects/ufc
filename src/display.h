//
// Created by Ian Parker on 20/01/2024.
//

#ifndef XPFD_DISPLAY_H
#define XPFD_DISPLAY_H

#include <SDL.h>

#include <geek/gfx-surface.h>
#include <geek/fonts.h>

#include <thread>

class XPlaneClient;

struct State
{
    float indicatedAirspeed = 0.0f;
    float indicatedMach = 0.0f;
    float roll = 0.0f;
    float pitch = 0.0f;
    float altitude = 0.0f;
    float verticalSpeed = 0.0f;
    float magHeading = 0.0f;
    float barometerHG = 0.0f;
};

class FlightWidget
{

};

class XPFlightDisplay
{
 private:
    SDL_Window* m_window = nullptr;
    int m_screenWidth = 0;
    int m_screenHeight = 0;

    XPlaneClient* m_client = nullptr;
    State m_state;
    bool m_running = false;

    Geek::Gfx::Surface* m_displaySurface = nullptr;
    Geek::Gfx::Surface* m_adiSurface = nullptr;
    Geek::Gfx::Surface* m_speedSurface = nullptr;
    Geek::Gfx::Surface* m_altitudeSurface = nullptr;
    Geek::Gfx::Surface* m_altitudeHundredsSurface = nullptr;
    Geek::Gfx::Surface* m_headingSurface = nullptr;
    Geek::FontManager* m_fontManager;
    Geek::FontHandle* m_font;
    Geek::FontHandle* m_fontSmall;
    Geek::FontHandle* m_largeFont;

    std::thread* m_updateThread;
    std::mutex m_updateMutex;

    void updateMain();
    void updateState(std::map<int, float> values);

    void drawADI(
        int adiX,
        int adiY,
        int adiWidth,
        int adiHeight);
    void drawSpeedIndicator(int x, int y, int w, int h);
    void drawAltitudeIndicator(int x, int y, int w, int h);
    void drawHeading(int x, int y, int w, int h);

 public:
    XPFlightDisplay();

    bool init();

    void close();

    void draw();

};


#endif //XPFD_DISPLAY_H
