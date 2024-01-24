//
// Created by Ian Parker on 20/01/2024.
//

#ifndef XPFD_GFXUTILS_H
#define XPFD_GFXUTILS_H

#include <geek/gfx-surface.h>
#include <glm/glm.hpp>

static inline glm::ivec2 rotate(glm::ivec2 centre, glm::ivec2 pos, float angle)
{
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return centre + glm::ivec2(
        (int) (((float) pos.x * cosA) + ((float) pos.y * sinA)),
        (int) (((float) pos.x * -sinA) + ((float) pos.y * cosA)));
}

void drawPolygon(
    Geek::Gfx::Surface* surface,
    std::vector<glm::ivec2> points,
    bool drawFill,
    uint32_t fill,
    bool drawOutline,
    uint32_t outline);

void drawFilledPolygon(
    Geek::Gfx::Surface* surface,
    std::vector<glm::ivec2> points,
    uint32_t c);

void drawPolygonOutline(Geek::Gfx::Surface* surface, const std::vector<glm::ivec2> &points, uint32_t outline, int i);

#endif //XPFD_GFXUTILS_H
