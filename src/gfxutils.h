//
// Created by Ian Parker on 20/01/2024.
//

#ifndef XPFD_GFXUTILS_H
#define XPFD_GFXUTILS_H

#include <geek/gfx-surface.h>
#include <glm/glm.hpp>

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

#endif //XPFD_GFXUTILS_H
