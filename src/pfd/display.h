//
// Created by Ian Parker on 20/01/2024.
//

#ifndef XPFD_DISPLAY_H
#define XPFD_DISPLAY_H

#include <SDL.h>

#include <geek/gfx-surface.h>
#include <geek/fonts.h>

#include <thread>

#include <ufc/flightconnector.h>
#include "pfd/widgets/widget.h"

class XPFlightDisplay
{
 private:
    SDL_Window* m_window = nullptr;
    int m_screenWidth = 0;
    int m_screenHeight = 0;

    bool m_running = false;

    std::vector<std::shared_ptr<FlightWidget>> m_widgets;

    std::shared_ptr<Geek::Gfx::Surface> m_displaySurface;
    std::shared_ptr<Geek::FontManager> m_fontManager;

    Geek::FontHandle* m_font = nullptr;
    Geek::FontHandle* m_fontSmall = nullptr;
    Geek::FontHandle* m_largeFont = nullptr;

    UFC::FlightConnector m_flightConnector;
    std::shared_ptr<UFC::DataSource> m_dataSource;

    std::thread* m_updateThread = nullptr;

    void updateMain();

 public:
    XPFlightDisplay();

    bool init();

    void close();

    void draw();

    Geek::FontHandle* getFont() const { return m_font; }
    Geek::FontHandle* getSmallFont() const { return m_fontSmall; }

    void drawWidgets();
};


#endif //XPFD_DISPLAY_H
