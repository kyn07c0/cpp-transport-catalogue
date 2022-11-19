#include "geo.h"

namespace geo {
    
inline const int RADIUS_EARTH = 6371000;

double ComputeDistance(Coordinates from, Coordinates to)
{
    using namespace std;
    if(from == to)
    {
        return 0;
    }

    return acos(sin(from.lat * DR) * sin(to.lat * DR)
                + cos(from.lat * DR) * cos(to.lat * DR)
                * cos(abs(from.lng - to.lng) * DR))
                * RADIUS_EARTH;
}

} // namespace geo
