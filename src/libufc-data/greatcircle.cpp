/*
 * Calculate Great Circle paths between two points
 *
 * This code has been adapted from the Leaflet.SmoothGeodesic library by
 * Hunter Evanoff - hunter@evanoff.dev
 * (MIT License)
 */

#include "ufc/data/greatcircle.h"

#include <cmath>
#include <vector>

using namespace std;
using namespace UFC;

constexpr auto D2R = M_PI / 180.0;
constexpr auto R2D = 180.0 / M_PI;

// from http://www.gdal.org/ogr2ogr.html
// (starting with GDAL 1.10) offset from dateline in degrees
// (default long. = +/- 10deg, geometries within 170deg to -170deg will be split)
constexpr auto dfDateLineOffset = 10.0;
constexpr auto dfLeftBorderX = 180.0 - dfDateLineOffset;
constexpr auto dfRightBorderX = -180.0 + dfDateLineOffset;
constexpr auto dfDiffSpace = 360.0 - dfDateLineOffset;

GreatCircle::GreatCircle(const Coordinate &start, const Coordinate &end)
{
    m_start.latitude = start.latitude * D2R;
    m_start.longitude = start.longitude * D2R;
    m_end.latitude = end.latitude * D2R;
    m_end.longitude = end.longitude * D2R;
    auto w = m_start.longitude - m_end.longitude;
    auto h = m_start.latitude - m_end.latitude;
    auto z =
        pow(sin(h / 2.0), 2) +
        cos(m_start.latitude) *
        cos(m_end.latitude) *
        pow(sin(w / 2.0), 2);
    m_g = 2.0 * asin(sqrt(z));
}

Coordinate GreatCircle::interpolate(double f) const
{
    auto A = sin((1.0 - f) * m_g) / sin(m_g);
    auto B = sin(f * m_g) / sin(m_g);
    auto x =
        A * cos(m_start.latitude) * cos(m_start.longitude) +
        B * cos(m_end.latitude) * cos(m_end.longitude);
    auto y =
        A * cos(m_start.latitude) * sin(m_start.longitude) +
        B * cos(m_end.latitude) * sin(m_end.longitude);
    auto z = A * sin(m_start.latitude) + B * sin(m_end.latitude);
    auto lat = R2D * atan2(z, sqrt(pow(x, 2.0) + pow(y, 2.0)));
    auto lon = R2D * atan2(y, x);
    return Coordinate{lat, lon};
}

vector<Coordinate> GreatCircle::arc(int numPoints) const
{
    const vector<Coordinate> firstPass = arcFirstPass(numPoints);

    double dfMaxSmallDiffLong;
    bool bHasBigDiff = calculateLongitudeDifferences(firstPass, dfMaxSmallDiffLong);

    vector<vector<Coordinate>> poMulti;
    if (bHasBigDiff && dfMaxSmallDiffLong < dfDateLineOffset)
    {
        handleBigDifference(firstPass, poMulti);
    }
    else
    {
        vector<Coordinate> poNewLS0;
        poNewLS0.reserve(firstPass.size());
        for (const auto & firstPas : firstPass)
        {
            poNewLS0.push_back(firstPas);
        }
        poMulti.push_back(poNewLS0);
    }

    return aggregateArc(poMulti);
}

vector<Coordinate> GreatCircle::arcFirstPass(int numPoints) const
{
    vector<Coordinate> firstPass;
    if (numPoints == 0 || numPoints <= 2)
    {
        firstPass.push_back(m_start);
        firstPass.push_back(m_end);
    }
    else
    {
        auto delta = 1.0 / (numPoints - 1);
        for (int i = 0; i < numPoints; ++i)
        {
            auto step = delta * i;
            auto pair = interpolate(step);
            firstPass.push_back(pair);
        }
    }
    return firstPass;
}

bool GreatCircle::calculateLongitudeDifferences(const vector<Coordinate> &firstPass, double &dfMaxSmallDiffLong)
{
    auto bHasBigDiff = false;
    dfMaxSmallDiffLong = 0.0;

    // https://github.com/OSGeo/gdal/blob/7bfb9c452a59aac958bff0c8386b891edf8154ca/gdal/ogr/ogrgeometryfactory.cpp#L2342
    for (size_t j = 1; j < firstPass.size(); ++j)
    {
        auto dfPrevX = firstPass[j - 1].longitude;
        auto dfX = firstPass[j].longitude;
        auto dfDiffLong = abs(dfX - dfPrevX);
        if (
            dfDiffLong > dfDiffSpace &&
            ((dfX > dfLeftBorderX && dfPrevX < dfRightBorderX) ||
             (dfPrevX > dfLeftBorderX && dfX < dfRightBorderX))
        )
        {
            bHasBigDiff = true;
        }
        else if (dfDiffLong > dfMaxSmallDiffLong)
        {
            dfMaxSmallDiffLong = dfDiffLong;
        }
    }
    return bHasBigDiff;
}

void GreatCircle::handleBigDifference(const vector<Coordinate> &firstPass, vector<vector<Coordinate>> poMulti)
{
    vector<Coordinate> poNewLS;
    for (size_t k = 0; k < firstPass.size(); ++k)
    {
        auto dfX0 = firstPass[k].longitude;
        if (k > 0 && abs(dfX0 - firstPass[k - 1].longitude) > dfDiffSpace)
        {
            auto dfX1 = firstPass[k - 1].longitude;
            auto dfY1 = firstPass[k - 1].latitude;
            auto dfX2 = firstPass[k].longitude;
            auto dfY2 = firstPass[k].latitude;

            if (
                dfX1 > -180 &&
                dfX1 < dfRightBorderX &&
                dfX2 == 180 &&
                k + 1 < firstPass.size() &&
                firstPass[k - 1].longitude > -180 &&
                firstPass[k - 1].longitude < dfRightBorderX
            )
            {
                poNewLS.emplace_back(firstPass[k].latitude, -180);
                k++;
                poNewLS.emplace_back(firstPass[k].latitude, firstPass[k].longitude);
                poMulti.push_back(poNewLS);
                continue;
            }

            if (
                dfX1 > dfLeftBorderX &&
                dfX1 < 180 &&
                dfX2 == -180 &&
                k + 1 < firstPass.size() &&
                firstPass[k - 1].longitude > dfLeftBorderX &&
                firstPass[k - 1].longitude < 180
            )
            {
                poNewLS.emplace_back(firstPass[k].latitude, 180);
                k++;
                poNewLS.emplace_back(firstPass[k].latitude, firstPass[k].longitude);
                poMulti.push_back(poNewLS);
                continue;
            }

            if (dfX1 < dfRightBorderX && dfX2 > dfLeftBorderX)
            {
                // swap dfX1, dfX2
                auto tmpX = dfX1;
                dfX1 = dfX2;
                dfX2 = tmpX;
                // swap dfY1, dfY2
                auto tmpY = dfY1;
                dfY1 = dfY2;
                dfY2 = tmpY;
            }
            if (dfX1 > dfLeftBorderX && dfX2 < dfRightBorderX)
            {
                dfX2 += 360;
            }

            if (dfX1 <= 180 && dfX2 >= 180 && dfX1 < dfX2)
            {
                auto dfRatio = (180 - dfX1) / (dfX2 - dfX1);
                auto dfY = dfRatio * dfY2 + (1 - dfRatio) * dfY1;
                poNewLS.emplace_back(
                    dfY,
                    firstPass[k - 1].longitude > dfLeftBorderX ? 180.0f : -180.0f);
                poMulti.push_back(poNewLS);

                vector<Coordinate> ls2;
                ls2.emplace_back(
                    dfY,
                    firstPass[k - 1].longitude > dfLeftBorderX ? -180.0f : 180.0f);
                ls2.emplace_back(firstPass[k].latitude, dfX0);
                poMulti.push_back(ls2);
            }
            else
            {
                poMulti.push_back(poNewLS);
                vector<Coordinate> ls2;
                ls2.emplace_back(firstPass[k].latitude, dfX0);
                poMulti.push_back(ls2);
            }
        }
        else
        {
            poNewLS.push_back(firstPass[k]);
            poMulti.push_back(poNewLS);
        }
    }
}

vector<Coordinate> GreatCircle::aggregateArc(const vector<vector<Coordinate>> &poMulti)
{
    vector<Coordinate> arc;
    for (auto const& poLine : poMulti)
    {
        for (auto const& point : poLine)
        {
            arc.push_back(point);
        }
    }
    return arc;
}
