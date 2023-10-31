#ifndef GEO_H
#define GEO_H

struct GEOcoordinate 
{
    GEOcoordinate():latitude(-1), longitude(-1), altitude(-1) {}
    GEOcoordinate(double _latitude, double _longitude, double _altitude):latitude(_latitude), longitude(_longitude), altitude(_altitude) {}
    double latitude;
    double longitude;
    double altitude;
};


#endif