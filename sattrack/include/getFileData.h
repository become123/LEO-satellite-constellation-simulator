#ifndef GET_TLE_DATA
#define GET_TLE_DATA
#include <fstream>
#include <map>
#include <string>
#include <SGP4.h>
#include"satellite.h"
#include "groundStation.h"

namespace getFileData
{
    //獲得satellite table
    std::map<int, satellite::satellite> getSatellitesTable(std::map<int, std::map<int, bool>> &closeLinksTable, std::map<std::string, std::string> &parameterTable);

    //獲得parameterTable，其中記錄模擬所設置的各種parameter
    std::map<std::string, std::string> getParameterdata(std::string fileName);

    //獲得parameter.txt中設置的經緯度們的地面站物件(不只一個)
    std::vector<groundStation::groundStation> getInputStations(const std::map<std::string, std::string> &parameterTable);

    //獲得要關掉的Link的table(table[satId1][satId2]代表satId1與satId2之間的Link被關掉)
    std::map<int, std::map<int, bool>> getCloseLinkTable(std::string fileName);
}
#endif
