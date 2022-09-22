#ifndef GOUND_STATION_H
#define GOUND_STATION_H
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>



namespace groundStation
{
    
    class groundStation{
    public:
        groundStation(const double latitude, const double longitude, const double altitude);

    private:
        Observer obs;
    };
    

}
#endif