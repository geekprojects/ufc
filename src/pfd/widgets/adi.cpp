//
// Created by Ian Parker on 23/01/2024.
//

#include "adi.h"

#include "pfd/display.h"

using namespace std;
using namespace UFC;
using namespace glm;

#define PITCH_SIZE 60.0f

ADIWidget::ADIWidget(XPFlightDisplay* display, int x, int y, int ww, int wh)
    : FlightWidget(display, x, y, ww, wh)
{
    int w = getWidth() * 2;
    int h = getHeight() * 2;
    m_adiSurface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, w, h);

    auto context = Cairo::Context::create(m_adiSurface);

    context->set_source_rgb(0, 0.5, 1.0);
    context->rectangle(0, 0, w, h);
    context->fill();

    context->set_source_rgb(0.5, 0.2, 0);
    context->rectangle(0, h / 2.0, w, h / 2.0);
    context->fill();

    context->set_source_rgb(1.0, 1.0, 1.0);
    context->move_to(0, h / 2.0);
    context->line_to(w, h / 2.0);
    context->set_line_width(2);
    context->stroke();

    context->move_to((w / 2.0), (h / 2.0) - 5);
    context->line_to((w / 2.0), (h / 2.0) + 5);
    context->set_line_width(1);
    context->stroke();


    m_adiSurface->write_to_png("pfd.png");
}

void ADIWidget::draw(AircraftState& state, std::shared_ptr<Cairo::Context> context)
{
    float roll = radians(state.roll);
    float pitch = state.pitch;

    float halfWidth = getWidth() / 2.0;
    float halfHeight = getHeight() / 2.0;

    int pitchY = pitchToY(pitch);
    int diagonal = hypot(getWidth(), getHeight());

    context->save();
    context->arc(halfWidth, halfHeight, halfHeight, 0, 2*M_PI);
    context->clip();

    context->save();

    // Translate and rotate so that the origin is correct for pitch and roll
    context->translate(halfWidth, halfHeight);
    context->rotate(-roll);
    context->translate(0, pitchY);

    // Draw ground
    context->set_source_rgb(0.5, 0.2, 0);
    context->rectangle(-diagonal, 0, 2 * diagonal, getHeight());
    context->fill();

    // Draw sky
    context->set_source_rgb(0, 0.5, 1.0);
    context->rectangle(-diagonal, -getHeight(), 2 * diagonal, getHeight());
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

    for (float pitchMark = -20; pitchMark <= 20; pitchMark += 2.5f)
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
        int pitchMarkY = pitchToY(pitchMark) + pitchY;
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
            context->set_font_size(10);
            context->move_to(-(markWidth + 20), pitchMarkY + 5);
            context->show_text(buf);
            context->move_to((markWidth + 5), pitchMarkY + 5);
            context->show_text(buf);
        }
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

int ADIWidget::pitchToY(float pitch) const
{
    return (int) ((float)getHeight() * (pitch / PITCH_SIZE));
}


