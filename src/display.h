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
class XPFlightDisplay;
class ADIWidget;
class SpeedIndicatorWidget;
class AltitudeIndicatorWidget;
class HeadingIndicatorWidget;

struct State
{
    bool connected = false;
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
 protected:
    XPFlightDisplay* m_display;
    int m_x;
    int m_y;
    int m_width;
    int m_height;

 public:
    FlightWidget(XPFlightDisplay* flightDisplay, int x, int y, int w, int h) :
        m_display(flightDisplay),
        m_x(x),
        m_y(y),
        m_width(w),
        m_height(h) {}
    virtual ~FlightWidget() = default;

    virtual void draw(State& state, std::shared_ptr<Geek::Gfx::Surface> surface) = 0;
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

    std::shared_ptr<ADIWidget> m_adiWidget;
    std::shared_ptr<SpeedIndicatorWidget> m_speedIndicatorWidget;
    std::shared_ptr<AltitudeIndicatorWidget> m_altitudeIndicatorWidget;
    std::shared_ptr<HeadingIndicatorWidget> m_headingIndicatorWidget;

    std::shared_ptr<Geek::Gfx::Surface> m_displaySurface;
    std::shared_ptr<Geek::FontManager> m_fontManager;

    Geek::FontHandle* m_font = nullptr;
    Geek::FontHandle* m_fontSmall = nullptr;
    Geek::FontHandle* m_largeFont = nullptr;

    std::thread* m_updateThread = nullptr;
    std::mutex m_updateMutex;

    void updateMain();
    void updateState(std::map<int, float> values);

 public:
    XPFlightDisplay();

    bool init();

    void close();

    void draw();

    Geek::FontHandle* getFont() const { return m_font; }
    Geek::FontHandle* getSmallFont() const { return m_fontSmall; }
};


#endif //XPFD_DISPLAY_H
