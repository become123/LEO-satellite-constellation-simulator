#ifndef SATELLITE_H
#define SATELLITE_H
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include "AER.h"


namespace satellite
{
    
class satellite{
public:    
    satellite(Tle _tle, SGP4 _sgp4);
    Tle getTle();
    SGP4 getSgp4();

    AER getAER(int second, satellite other);

private:
    Tle tle;
    SGP4 sgp4;
};









}
#endif