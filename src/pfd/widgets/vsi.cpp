//
// Created by Ian Parker on 06/01/2025.
//

#include "vsi.h"

#include <glm/common.hpp>

#include "pfd/display.h"

void VSIWidget::draw(UFC::AircraftState& state, const std::shared_ptr<Cairo::Context>& context)
{
    float cy = getHeight()/ 2.0;

    auto s = glm::sign(state.verticalSpeed);
    auto vsAbs = glm::abs(state.verticalSpeed);
    vsAbs = glm::min(vsAbs, 6250.0f);

    float y_1000 = getHeight()/2.0f*33.0f/75.0f;
    //float y_500 = y_1000 / 2.0f;
    float y_2000 = getHeight()/2.0f*55.0f/75.0f;
    //float y_1500 = y_1000 + (y_2000-y_1000)/2.0f;
    float y_6000 = getHeight()/2.0f*70.0f/75.0f;
    //float y_4000 = y_2000 + (y_6000-y_2000)/2.0f;

    context->set_source_rgb(0.29, 0.29, 0.29);
    context->move_to(0, 0);
    context->line_to(20, 0);
    context->line_to(getWidth() - 20, 50);
    context->line_to(getWidth() - 20, getHeight() - 50);
    context->line_to(20, getHeight());
    context->line_to(0, getHeight());
    context->fill();

    context->set_source_rgb(1.0, 1.0, 1.0);
    getDisplay()->drawText(context, "1", 5, cy + y_1000);
    getDisplay()->drawText(context, "1", 5, cy - y_1000);
    getDisplay()->drawText(context, "2", 5, cy + y_2000);
    getDisplay()->drawText(context, "2", 5, cy - y_2000);
    getDisplay()->drawText(context, "6", 5, cy + y_6000);
    getDisplay()->drawText(context, "6", 5, cy - y_6000);

    float vsY;
    if ( vsAbs > 2000.0f ) {
        vsY = y_2000 + (vsAbs-2000.0f)*(y_6000-y_2000)/4000.0f;
    } else if ( vsAbs > 1000 ) {
        vsY = y_1000 + (vsAbs-1000.0f)*(y_2000-y_1000)/1000.0f;
    } else {
        vsY = (vsAbs)*(y_1000)/1000.0f;
    }

    if (vsAbs > 6000)
    {
        context->set_source_rgb(1.0, 0.0, 0.0);
    }
    else
    {
        context->set_source_rgb(0.0, 1.0, 0.0);
    }
    context->set_line_width(2);
    context->move_to(getWidth() / 2.0, cy - vsY * s);
    context->line_to(getWidth() - 5.0, cy);
    context->stroke();

    if (vsAbs > 200)
    {
        char buf[50];
        snprintf(buf, 50, "%02d", (int)(vsAbs / 100.0f));
        getDisplay()->drawText(context, buf, 0, getHeight() - 20);
    }
}
