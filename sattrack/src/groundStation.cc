
#include "groundStation.h"
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>

namespace groundStation
{
    groundStation::groundStation(const double latitude,const double longitude,const double altitude): obs(latitude, longitude, altitude){}
}