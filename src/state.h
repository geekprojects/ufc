//
// Created by Ian Parker on 20/01/2024.
//

#ifndef XPFD_STATE_H
#define XPFD_STATE_H

struct State
{
    bool connected = false;
    float indicatedAirspeed = 0.0f;
    float indicatedMach = 0.0f;
    float roll = 0.0f;
    float pitch = 0.0f;
    float altitude = 0.0f;
    float verticalSpeed = 0.0f;
    float magHeading = 0.0f;
    float barometerHG = 0.0f;
};

#endif //XPFD_STATE_H
