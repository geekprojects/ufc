//
// Created by Ian Parker on 06/01/2025.
//

#include "annunciators.h"

void AnnunciatorsWidget::draw(UFC::AircraftState& state, const std::shared_ptr<Cairo::Context>& context)
{
    float divWidth = getWidth() / 5.0;

    for (int i = 0; i < 4; i++)
    {
        context->set_source_rgb(1, 1, 1);
        context->move_to(divWidth * (i + 1.0f), 3);
        context->line_to(divWidth * (i + 1.0f), getHeight() - 3);
        context->stroke();
    }
}
