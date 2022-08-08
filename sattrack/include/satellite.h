#ifndef SATELLITE_H
#define SATELLITE_H
#include <vector>
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <map>
#include <SGP4.h>
#include "AER.h"


namespace satellite
{
    bool judgeAzimuth(double ISLdirAzimuth, double acceptableAzimuthDif, double otherSatAzimuth);
    bool judgeElevation(double acceptableElevationDif, double otherSatElevation);
    bool judgeRange(double acceptableRange, double otherSatRange);
    
class satellite{
public: 
    satellite(Tle _tle, SGP4 _sgp4, int _id);
    Tle getTle();
    SGP4 getSgp4();
    int getId();
    AER getAER(int second, int otherId, std::map<int, satellite> &satellites);
    int getEastSatId();
    int getWestSatId();
    int getFrontSatId();
    int getBackSatId();
    void printNeighbor();
    bool judgeEastConnectability(int second, std::map<int, satellite> &satellites, std::map<std::string, int> &parameterTable);
    bool judgeWestConnectability(int second, std::map<int, satellite> &satellites, std::map<std::string, int> &parameterTable);


private:
    Tle tle;
    SGP4 sgp4;
    int id;
    std::vector<int> neighbors;
};









}
#endif