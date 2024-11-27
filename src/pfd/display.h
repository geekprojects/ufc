//
// Created by Ian Parker on 20/01/2024.
//

#ifndef XPFD_DISPLAY_H
#define XPFD_DISPLAY_H

#include <SDL.h>
#include <glm/glm.hpp>

#include <cairomm/cairomm.h>

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

    std::shared_ptr<Cairo::ImageSurface> m_displaySurface;

    FT_Library m_ftLibrary = nullptr;
    FT_Face m_face = nullptr;
    std::shared_ptr<Cairo::FtFontFace> m_fontFace;

    UFC::FlightConnector m_flightConnector;
    std::shared_ptr<UFC::DataSource> m_dataSource;

    std::thread* m_updateThread = nullptr;

    void updateMain() const;

 public:
    XPFlightDisplay();

    bool init();

    void close();

    void draw();

    std::shared_ptr<Cairo::FtFontFace> getFont() { return m_fontFace; }

    void drawWidgets(std::shared_ptr<Cairo::Context> context);

    void drawText(std::shared_ptr<Cairo::Context> surface, const std::string &text, double x, double y, int fontSize = 16);
};


#endif //XPFD_DISPLAY_H
