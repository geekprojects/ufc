//
// Created by Ian Parker on 23/01/2024.
//

#include "adi.h"
#include "pfd/gfxutils.h"

using namespace std;
using namespace UFC;
using namespace glm;
using namespace Geek;
using namespace Geek::Gfx;

//#define PITCH_SIZE 44.0
#define PITCH_SIZE 60.0f

ADIWidget::ADIWidget(XPFlightDisplay* display, int x, int y, int w, int h)
    : FlightWidget(display, x, y, w, h)
{
    m_adiSurface = make_shared<Surface>(getWidth(), getHeight(), 4);
}

void ADIWidget::draw(AircraftState& state, std::shared_ptr<Geek::Gfx::Surface> surface)
{
    m_adiSurface->clear(0xff0088FF);

    float roll = radians(state.roll);
    float pitch = state.pitch;

    int halfWidth = getWidth() / 2;
    int halfHeight = getHeight() / 2;

    ivec2 centre(halfWidth, halfHeight);

    int pitchY = halfHeight + pitchToY(pitch);

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

    // Draw pitch marks
    for (float pitchMark = -20; pitchMark <= 20; pitchMark += 2.5f)
    {
        int markWidth = 10;
        if (fmod(fabs(pitchMark), 10.0f) < FLT_EPSILON)
        {
            markWidth = 50;
        }
        else if (fmod(fabs(pitchMark), 5.0f) < FLT_EPSILON)
        {
            markWidth = 30;
        }
        int pitchMarkY = pitchToY(pitchMark);
        auto markCentre = ivec2(0, pitchMarkY);
        ivec2 mark0 = rotate(ivec2(halfWidth, pitchY), ivec2(-markWidth * 1, pitchMarkY), roll);
        ivec2 mark1 = rotate(ivec2(halfWidth, pitchY), ivec2(+markWidth * 1, pitchMarkY), roll);
        m_adiSurface->drawLine(mark0.x, mark0.y, mark1.x, mark1.y, 0xffffffff);
    }

    if (state.flightDirector.mode > 0)
    {
        ivec2 fdCentre(
            centre.x + (int)((float)getWidth() * (-state.roll + state.flightDirector.roll) / 3.0f),
            pitchToY(-(state.pitch - state.flightDirector.roll)));
        m_adiSurface->drawLine(fdCentre.x - 20, fdCentre.y, fdCentre.x + 20, fdCentre.y, 0xffffff00);
        m_adiSurface->drawLine(fdCentre.x, fdCentre.y - 20, fdCentre.x, fdCentre.y + 20, 0xffffff00);
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

    m_adiSurface->drawRectFilled(centre.x - 2, centre.y - 2, 5, 5, 0xff000000);
    m_adiSurface->drawRect(centre.x - 2, centre.y - 2, 5, 5, 0xffffffff);

    surface->blit(getX(), getY(), m_adiSurface.get());

}

int ADIWidget::pitchToY(float pitch)
{
    return (int) ((float)getHeight() * (pitch / PITCH_SIZE));
}


