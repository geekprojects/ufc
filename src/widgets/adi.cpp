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
    m_adiSurface = make_shared<Surface>(getWidth(), getHeight(), 4);
}

void ADIWidget::draw(State& state, std::shared_ptr<Geek::Gfx::Surface> surface)
{
    m_adiSurface->clear(0xff0088FF);

    float roll = radians(state.roll);
    float pitch = state.pitch;

    int halfWidth = getWidth() / 2;
    int halfHeight = getHeight() / 2;

    ivec2 centre(halfWidth, halfHeight);

    int pitchY = halfHeight + (int) ((float)getHeight() * pitch / 44.0f);

    auto horizonCentre = ivec2(halfWidth, pitchY);
    ivec2 horizon0 = rotate(horizonCentre, ivec2(-halfWidth * 2, 0), roll);
    ivec2 horizon1 = rotate(horizonCentre, ivec2(halfWidth * 2, 0), roll);

    // Draw ground
    {
        vector<ivec2> points;
        points.push_back(horizon0);
        points.push_back(horizon1);
        points.emplace_back(getWidth() - 1, getHeight() - 1);
        points.emplace_back(0, getHeight() - 1);
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

    surface->blit(getX(), getY(), m_adiSurface.get());
}


