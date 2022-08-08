#ifndef GET_TLE_DATA
#define GET_TLE_DATA
#include<fstream>
#include<map>
#include<SGP4.h>
#include"satellite.h"

namespace getFileData
{
    std::map<int, satellite::satellite> getTLEdata(std::string fileName);
    std::map<std::string, std::string> getParameterdata(std::string fileName);
}
#endif
