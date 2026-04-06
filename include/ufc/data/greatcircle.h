/*
* Calculate Great Circle paths between two points
 *
 * This code has been adapted from the Leaflet.SmoothGeodesic library by
 * Hunter Evanoff - hunter@evanoff.dev
 * (MIT License)
 */

#ifndef BLACKBOX_GREATCIRCLE_H
#define BLACKBOX_GREATCIRCLE_H

#include "ufc/utils/geoutils.h"

namespace UFC
{
class GreatCircle
{
    Coordinate m_start;
    Coordinate m_end;
    double m_g;

    [[nodiscard]] Coordinate interpolate(double f) const;

    [[nodiscard]] std::vector<Coordinate> arcFirstPass(int numPoints) const;

    static bool calculateLongitudeDifferences(const std::vector<Coordinate> &firstPass, double &dfMaxSmallDiffLong);
    static void handleBigDifference(const std::vector<Coordinate> &firstPass, std::vector<std::vector<Coordinate>> poMulti);


    static std::vector<Coordinate> aggregateArc(const std::vector<std::vector<Coordinate>> &poMulti);

public:
    GreatCircle(const Coordinate &start, const Coordinate &end);

    [[nodiscard]] std::vector<Coordinate> arc(int numPoints) const;
};
}

#endif //BLACKBOX_GREATCIRCLE_H
