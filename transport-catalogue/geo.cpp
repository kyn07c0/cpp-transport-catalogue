#include "geo.h"

namespace geo {

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
                * 6371000;
}

} // namespace geo
