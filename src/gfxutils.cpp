//
// Created by Ian Parker on 20/01/2024.
//

#include "gfxutils.h"

using namespace std;
using namespace glm;
using namespace Geek;
using namespace Geek::Gfx;

void drawPolygon(
    Surface* surface,
    vector<ivec2> points,
    bool drawFill,
    uint32_t fill,
    bool drawOutline,
    uint32_t outline)
{
    int i;
    int n = points.size();

    if (drawFill)
    {
        int surfaceWidth = surface->getWidth();
        ivec2 last = points.front();
        points.push_back(last);

        vector<float> slope(n);
        int minY = surface->getHeight();
        int maxY = 0;
        for (i = 0; i < n; i++)
        {
            minY = glm::min(minY, points[i].y);
            maxY = glm::max(maxY, points[i].y);

            ivec2 d = points[i + 1] - points[i];
            if (d.y == 0)
            {
                slope[i] = 1.0;
            }
            else if (d.x == 0)
            {
                slope[i] = 0.0;
            }
            else if ((d.y != 0) && (d.x != 0))
            {
                slope[i] = (float) d.x / (float) d.y;
            }
        }

        minY = glm::max(minY, 0);
        maxY = glm::min(maxY, (int) surface->getHeight() - 1);

        for (int y = minY; y <= maxY; y++)
        {
            vector<int> xi;
            for (i = 0; i < n; i++)
            {
                ivec2 p0 = points[i];
                ivec2 p1 = points[i + 1];
                if ((p0.y <= y && p1.y > y) ||
                    (p0.y > y && p1.y <= y))
                {
                    int xs = (int) (p0.x + slope[i] * (y - p0.y));

                    xs = glm::clamp(xs, 0, surfaceWidth - 1);
                    xi.push_back(xs);
                }
            }
            for (int j = 0; j < xi.size(); j++)
            {
                for (i = 0; i < xi.size() - 1; i++)
                {
                    if (xi[i] > xi[i + 1])
                    {
                        int temp = xi[i];
                        xi[i] = xi[i + 1];
                        xi[i + 1] = temp;
                    }
                }
            }

            for (i = 0; i < xi.size(); i += 2)
            {
                surface->drawHorizontalLine(xi[i], y, xi[i + 1] - xi[i], fill);
            }
        }
    }
    if (drawOutline)
    {
        drawPolygonOutline(surface, points, outline, i, n);
    }
}

void drawPolygonOutline(Surface* surface, const vector<ivec2> &points, uint32_t outline, int i, int n)
{
    for (i = 0; i < n; i++)
    {
        surface->drawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, outline);
    }
}

void drawFilledPolygon(
    Surface* surface,
    vector<ivec2> points,
    uint32_t c)
{
    drawPolygon(surface, points, true, c, true, c);
}
