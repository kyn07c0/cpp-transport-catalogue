#pragma once

#include <cmath>

namespace geo
{
    const double DR = M_PI / 180.;

    struct Coordinates
    {
        double lat;
        double lng;
        bool operator==(const Coordinates& other) const
        {
            return lat == other.lat && lng == other.lng;
        }

        bool operator!=(const Coordinates& other) const
        {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);

} // namespace geo
