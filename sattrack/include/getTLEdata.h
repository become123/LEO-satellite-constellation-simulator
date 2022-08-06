#ifndef GET_TLE_DATA
#define GET_TLE_DATA
#include<fstream>
#include<unordered_map>
#include<SGP4.h>
#include"satellite.h"

namespace getTLEdata
{
    std::unordered_map<int, satellite::satellite> getTLEdata(std::string fileName);
}
#endif
