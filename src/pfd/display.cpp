//
// Created by Ian Parker on 20/01/2024.
//

#include "display.h"

#include "pfd/widgets/adi.h"
#include "pfd/widgets/speedindicator.h"
#include "pfd/widgets/altitudeindicator.h"
#include "pfd/widgets/headingindicator.h"

#include <ufc/flightconnector.h>

#include <pangomm/init.h>
#include <pangomm/layout.h>

#include <thread>

#include "widgets/pfdwidget.h"

using namespace std;
using namespace UFC;

XPFlightDisplay::XPFlightDisplay() = default;

bool XPFlightDisplay::init()
{
    int res;
    res = SDL_Init(SDL_INIT_EVERYTHING);
    if (res != 0)
    {
        return false;
    }

    Pango::init();

    m_window = SDL_CreateWindow(
        "XP Flight Display",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        600, 1024,
        //600, 1024,
        SDL_WINDOW_OPENGL /*| SDL_WINDOW_ALLOW_HIGHDPI*/
    );

    int ww;
    int wh;
    SDL_GetWindowSize(m_window, &ww, &wh);
    SDL_GetWindowSizeInPixels(m_window, &m_screenWidth, &m_screenHeight);

    FT_Init_FreeType(&m_ftLibrary);
    //FT_New_Face (m_ftLibrary, "../data/fonts/BoeingFont.ttf", 0, &m_face);
    //FT_New_Face (m_ftLibrary, "../data/fonts/B612Mono-Regular.ttf", 0, &m_face);
    FT_New_Face (m_ftLibrary, "../data/fonts/ECAMFontRegular.ttf", 0, &m_face);
    m_fontFace = Cairo::FtFontFace::create(m_face, 0);

    m_displaySurface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, m_screenWidth, m_screenHeight);
/*
    double buttonHeight = 20;
    double instrumentAreaY = buttonHeight;
    double instrumentAreaHeight = m_screenHeight - (buttonHeight * 2);

    double adiWidth = (m_screenWidth / 10.0) * 3.0;
    double adiHeight = (instrumentAreaHeight / 3.0) * 2.0;
    double adiX = (m_screenWidth / 4.0) - (adiWidth / 2.0);
    double adiY = instrumentAreaY + ((instrumentAreaHeight / 2.0) - (adiHeight / 2.0));

    double indicatorHeight = (instrumentAreaHeight / 3.0) * 2.0;
    double indicatorWidth = ((m_screenWidth / 2.0) - adiWidth) / 2.5;
    double headingHeight = (instrumentAreaHeight - adiHeight) / 3.0;
*/
    m_widgets.push_back(make_shared<PFDWidget>(this, 0, 0, m_screenWidth, m_screenWidth));

    m_dataSource = m_flightConnector.openDefaultDataSource();
    m_dataSource->connect();

    m_running = true;
    m_updateThread = new thread(&XPFlightDisplay::updateMain, this);

    return true;
}

void XPFlightDisplay::close()
{
    m_dataSource->disconnect();

    SDL_DestroyWindow(m_window);

    m_running = false;
}

void XPFlightDisplay::draw()
{
    auto context = Cairo::Context::create(m_displaySurface);

    Cairo::FontOptions fontOptions;
    fontOptions.set_antialias(Cairo::ANTIALIAS_SUBPIXEL);
    fontOptions.set_hint_style(Cairo::FontOptions::HintStyle::FULL);
    fontOptions.set_subpixel_order(Cairo::SUBPIXEL_ORDER_RGB);
    fontOptions.set_hint_metrics(Cairo::FontOptions::HintMetrics::ON);
    context->set_font_options(fontOptions);

    drawWidgets(context);

    SDL_Surface* sdlSurface = SDL_GetWindowSurface(m_window);
    SDL_ConvertPixels(
        m_screenWidth, m_screenHeight,
        SDL_PIXELFORMAT_ARGB8888, m_displaySurface->get_data(), m_screenWidth * 4,
        sdlSurface->format->format, sdlSurface->pixels, sdlSurface->pitch);
    SDL_UpdateWindowSurface(m_window);
}

void XPFlightDisplay::drawWidgets(std::shared_ptr<Cairo::Context> context)
{
    AircraftState state = m_flightConnector.getState();
    context->save();
    context->set_source_rgb(0.0, 0.0, 0.0);
    context->rectangle(0, 0, m_screenWidth, m_screenHeight);
    context->fill();
    context->restore();

    for (auto const& widget : m_widgets)
    {
        auto widgetSurface = Cairo::Surface::create(m_displaySurface, widget->getX(), widget->getY(), widget->getWidth(), widget->getHeight());
        auto widgetContext = Cairo::Context::create(widgetSurface);
        widget->draw(state, widgetContext);

        widgetContext->set_source_rgb(1.0, 1.0, 1.0);
        widgetContext->rectangle(0, 0, widget->getWidth(), widget->getHeight());
        widgetContext->stroke();
    }
/*
    context->save();
    context->set_source_rgb(1.0, 1.0, 1.0);
    context->move_to(m_screenWidth / 2, 0);
    context->line_to(m_screenWidth / 2, m_screenHeight);
    context->set_line_width(1);
    context->stroke();
*/
#if 0
    context->save();


    context->set_source_rgb(1.0, 1.0, 1.0);

    char buf[50];
    snprintf(buf, 50, "COM1: %07.03f <-> %07.03f", (float)state.comms.com1Hz / 1000.0f, (float)state.comms.com1StandbyHz / 1000.0f);
    drawText(context, buf, (m_screenWidth / 2) + 10, 16);
    snprintf(buf, 50, "COM2: %07.03f <-> %07.03f", (float)state.comms.com2Hz / 1000.0f, (float)state.comms.com2StandbyHz / 1000.0f);
    drawText(context, buf, (m_screenWidth / 2) + 10, 32);
    snprintf(buf, 50, "NAV1: %07.03f <-> %07.03f", (float)state.comms.nav1Hz / 1000.0f, (float)state.comms.nav1StandbyHz / 1000.0f);
    drawText(context, buf, (m_screenWidth / 2) + 10, 48);
    snprintf(buf, 50, "NAV2: %07.03f <-> %07.03f", (float)state.comms.nav2Hz / 1000.0f, (float)state.comms.nav2StandbyHz / 1000.0f);
    drawText(context, buf, (m_screenWidth / 2) + 10, 64);
    context->restore();
#endif

    if (!state.connected)
    {
        uint32_t t = SDL_GetTicks();
        if ((t / 1000) % 2)
        {
            drawText(context, "No Connection", 10, 10);
        }
    }
}

void XPFlightDisplay::drawText(std::shared_ptr<Cairo::Context> context, const std::string& text, double x, double y, int fontSize)
{
    context->set_font_face(getFont());
    context->set_font_size(fontSize);
    context->move_to(x, y);
    context->show_text(text.c_str());
}

void XPFlightDisplay::updateMain() const
{
    while (m_running)
    {
        m_dataSource->update();
        SDL_Delay(100);
    }
}

