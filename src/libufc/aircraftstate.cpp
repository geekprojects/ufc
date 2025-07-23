//
// Created by Ian Parker on 22/07/2025.
//

#include <ufc/aircraftstate.h>

#include "datadefs.h"

using namespace std;
using namespace UFC;

int AircraftState::getInt(std::string dataName) const
{
    for (DataDefinition dataDefinition : g_dataRefsInit)
    {
        if (dataDefinition.id == dataName)
        {
            return *(int32_t*)((char*)this + dataDefinition.pos);
        }
    }
    return 0;
}
