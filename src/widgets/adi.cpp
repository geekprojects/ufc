//
// Created by Ian Parker on 23/01/2024.
//

#include "adi.h"
#include "gfxutils.h"

using namespace std;
using namespace glm;
using namespace Geek;
using namespace Geek::Gfx;

ADIWidget::ADIWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget(display, x, y, w, h)
{
    m_adiSurface = make_shared<Surface>(m_width, m_height, 4);
}

void ADIWidget::draw(State& state, std::shared_ptr<Geek::Gfx::Surface> surface)
{
    m_adiSurface->clear(0xff0088FF);

    float roll = radians(state.roll);
    float pitch = state.pitch;

    int halfWidth = m_width / 2;
    int halfHeight = m_height / 2;

    ivec2 centre(halfWidth, halfHeight);

    int pitchY = halfHeight + (int) ((float)m_height * pitch / 44.0f);

    ivec2 horizonCentre = ivec2(halfWidth, pitchY);
    ivec2 horizon0 = rotate(horizonCentre, ivec2(-halfWidth * 2, 0), roll);
    ivec2 horizon1 = rotate(horizonCentre, ivec2(halfWidth * 2, 0), roll);

    // Draw ground
    {
        vector<ivec2> points;
        points.push_back(horizon0);
        points.push_back(horizon1);
        points.push_back(ivec2(m_width - 1, m_height - 1));
        points.push_back(ivec2(0, m_height - 1));
        drawFilledPolygon(m_adiSurface.get(), points, 0xff884400);
    }

    // Draw line at horizon
    {
        vector<ivec2> points;
        points.push_back(horizon0 + ivec2(0, -1));
        points.push_back(horizon1 + ivec2(0, -1));
        points.push_back(horizon1 + ivec2(0, 1));
        points.push_back(horizon0 + ivec2(0, 1));
        drawFilledPolygon(m_adiSurface.get(), points, 0xffffffff);
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
        drawPolygon(m_adiSurface.get(), points, true, 0xffffffff, true, 0xff000000);
    }
    {
        vector<ivec2> points;
        points.push_back(centre + ivec2(130, -5));
        points.push_back(centre + ivec2(40, -5));
        points.push_back(centre + ivec2(40, 15));
        points.push_back(centre + ivec2(50, 15));
        points.push_back(centre + ivec2(50, 5));
        points.push_back(centre + ivec2(130, 5));
        drawPolygon(m_adiSurface.get(), points, true, 0xffffffff, true, 0xff000000);
    }

    surface->blit(m_x, m_y, m_adiSurface.get());
}


