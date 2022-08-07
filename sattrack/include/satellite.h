#ifndef SATELLITE_H
#define SATELLITE_H
#include <vector>
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include "AER.h"


namespace satellite
{
    
class satellite{
public: 
    satellite(Tle _tle, SGP4 _sgp4, int _id);
    Tle getTle();
    SGP4 getSgp4();
    int getId();
    AER getAER(int second, satellite other);
    int getEastSat();
    int getWestSat();
    int getFrontSat();
    int getBackSat();
    void printNeighbor();


private:
    Tle tle;
    SGP4 sgp4;
    int id;
    std::vector<int> neighbors;
};









}
#endif