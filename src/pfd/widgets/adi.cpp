//
// Created by Ian Parker on 23/01/2024.
//

#include "adi.h"

#include "pfd/display.h"

using namespace std;
using namespace UFC;
using namespace glm;

constexpr float PITCH_SIZE = 30.0f;

ADIWidget::ADIWidget(XPFlightDisplay* display, float x, float y, float ww, float wh)
    : FlightWidget(display, x, y, ww, wh)
{
}

void ADIWidget::draw(AircraftState& state, const std::shared_ptr<Cairo::Context>& topContext)
{
    float roll = radians(state.roll);
    float pitch = state.pitch;

    float halfWidth = getWidth() / 2.0f;
    float halfHeight = getHeight() / 2.0f;

    float pitchY = pitchToY(pitch);
    float diagonal = hypot(getWidth(), getHeight());

    // 78/84
    float adiWidth = getWidth() * 0.94;
    float adiHeight = getHeight() * 0.94;

    auto adiSurface = Cairo::Surface::create(
    topContext->get_target(),
    getWidth() * 0.03,
    getHeight() * 0.03,
    adiWidth,
    adiHeight);
    auto context = Cairo::Context::create(adiSurface);
    context->save();
#if 1
    context->arc(adiWidth / 2.0, adiHeight / 2.0, adiHeight / 2.0f, 0, 2*M_PI);
#else
    double x = 0;        /* parameters like cairo_rectangle */
    double y = 0;
    const double width = getWidth();
    const double height = getHeight();
    const double aspect = 1.0;     /* aspect ratio */
    const double corner_radius = height / 20.0;   /* and corner curvature radius */

    const double radius = corner_radius / aspect;
    const double degrees = M_PI / 180.0;

    context->begin_new_sub_path();
    context->arc(width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    context->arc(x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    context->arc(x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    context->arc(x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    context->close_path();
#endif

    context->clip();

    context->save();

    // Translate and rotate so that the origin is correct for pitch and roll
    context->translate(halfWidth, halfHeight);
    context->rotate(-roll);
    context->translate(0, pitchY);

    // Draw ground
    context->set_source_rgb(0.5, 0.2, 0);
    context->rectangle(-diagonal, 0, 2 * diagonal, getHeight() * 2);
    context->fill();

    // Draw sky
    context->set_source_rgb(0, 0.5, 1.0);
    context->rectangle(-diagonal, -getHeight() * 2, 2 * diagonal, getHeight() * 2);
    context->fill();

    // Draw horizon
    context->set_source_rgb(1.0, 1.0, 1.0);
    context->move_to(-diagonal, 0);
    context->line_to(diagonal, 0);
    context->set_line_width(2);
    context->stroke();

    context->restore();

    // Draw pitch marks
    context->save();
    context->translate(halfWidth, halfHeight);
    context->rotate(-roll);
    context->set_source_rgb(1.0, 1.0, 1.0);

    float pitchMark = -20;
    while (pitchMark <= 20)
    {
        bool print = false;
        int markWidth = 10;
        float pitchMarkAbs = fabs(pitchMark);
        if (fmod(pitchMarkAbs, 10.0f) < FLT_EPSILON)
        {
            markWidth = 50;
            print = pitchMarkAbs >= FLT_EPSILON;
        }
        else if (fmod(pitchMarkAbs, 5.0f) < FLT_EPSILON)
        {
            markWidth = 30;
        }

        float pitchMarkY = pitchToY(pitchMark) + pitchY;
        context->move_to(-markWidth, pitchMarkY);
        context->line_to(markWidth, pitchMarkY);
        context->set_line_width(1);
        context->stroke();

        if (print)
        {
            char buf[50];
            snprintf(buf, 50, "%d", (int)abs(pitchMark));
            context->set_source_rgb(1.0, 1.0, 1.0);
            context->set_font_face(getDisplay()->getFont());
            context->set_font_size(20);
            Cairo::TextExtents extents;
            context->get_text_extents(buf, extents);
            context->move_to(-(markWidth + extents.width + 5), pitchMarkY + (extents.height / 2.0f));
            context->show_text(buf);
            context->move_to((markWidth + 5), pitchMarkY + (extents.height / 2.0f));
            context->show_text(buf);
        }
        pitchMark += 2.5f;
    }
    context->restore();

#if 0
    char buf[50];
    snprintf(buf, 50, "Pitch: %0.2f, Roll: %0.2f\n", state.pitch, state.roll);
    context->set_source_rgb(1.0, 1.0, 1.0);
    context->set_font_face(getDisplay()->getFont());
    context->set_font_size(12);
    context->move_to(5, 12);
    context->show_text(buf);
#endif

    // Draw wings
    context->save();
    context->translate(halfWidth, halfHeight);

    context->move_to(-130, -0);
    context->line_to(-40, 0);
    context->line_to(-40, 15);
    context->set_source_rgb(1.0, 1.0, 1.0);
    context->set_line_width(5);
    context->set_line_cap(Cairo::Context::LineCap::ROUND);
    context->stroke_preserve();
    context->set_source_rgb(0.0, 0.0, 0.0);
    context->set_line_width(3);
    context->stroke();

    context->move_to(130, -0);
    context->line_to(40, 0);
    context->line_to(40, 15);
    context->set_source_rgb(1.0, 1.0, 1.0);
    context->set_line_width(5);
    context->stroke_preserve();
    context->set_source_rgb(0.0, 0.0, 0.0);
    context->set_line_width(3);
    context->stroke_preserve();

    context->restore();


#if 0

    if (state.flightDirector.mode > 0)
    {
        ivec2 fdCentre(
            centre.x + (int)((float)getWidth() * (-state.roll + state.flightDirector.roll) / 3.0f),
            pitchToY(-(state.pitch - state.flightDirector.roll)));
        m_adiSurface->drawLine(fdCentre.x - 20, fdCentre.y, fdCentre.x + 20, fdCentre.y, 0xffffff00);
        m_adiSurface->drawLine(fdCentre.x, fdCentre.y - 20, fdCentre.x, fdCentre.y + 20, 0xffffff00);
    }
#endif
}

float ADIWidget::pitchToY(float pitch) const
{
    return getHeight() * (pitch / PITCH_SIZE);
}


