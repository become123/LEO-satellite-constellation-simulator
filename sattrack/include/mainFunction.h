#ifndef MAIN_FUNCTION_H
#define MAIN_FUNCTION_H
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include "getFileData.h"
#include "satellite.h"
#include "AER.h"

#include <iostream>
#include <iomanip>
#include<map>
#include<fstream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <bitset>
#include <set>

namespace mainFunction
{

    //印出設定的parameter
    void printParameter(std::map<std::string, std::string> &parameterTable);

    //印出每一個衛星的四個鄰居衛星編號
    void printAllSatNeighborId(std::map<int, satellite::satellite> &satellites);

    //印出編號observerId衛星觀察編號otherId衛星一天中的AER數值到sattrack/output.txt
    void printAERfile(int observerId, int otherId, std::map<int, satellite::satellite> &satellites, std::string outputFileName);

    //印出星網的跨軌道與同軌道的衛星距離資訊到sattrack/output.txt
    void printDistanceInformation(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出編號observerId衛星觀察右方衛星一天中的AER差異數值(用於判斷可否連線)到sattrack/output.txt
    void printRightSatAERdiff(int observerId, std::map<int, satellite::satellite> &satellites, std::string OutputFileName);

    //印出編號observerId衛星觀察左方衛星一天中的AER差異數值(用於判斷可否連線)到sattrack/output.txt
    void printLeftSatAERdiff(int observerId, std::map<int, satellite::satellite> &satellites, std::string OutputFileName);

    //印出編號observerId衛星一天中對飛行方向右方衛星的連線狀態(單向)到sattrack/output.txt中
    void printRightConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出編號observerId衛星一天中對飛行方向左方衛星的連線狀態(單向)到sattrack/output.txt中
    void printLeftConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出每顆衛星在一天中，左右ISL的一天total可建立連線秒數(雙向皆通才可建立連線)到sattrack/output.txt中
    void printAllIslConnectionInfoFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出某個特定時刻，行星群的連線狀態(totalSatCount*totalSatCount)到sattrack/output.txt中
    void printConstellationStateFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)到sattrack/output.txt中，並且在文件最後印出hopCount的統計數據
    void printConstellationHopCountFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)到sattrack/output.txt中，並且在terminal中印出由observerId的衛星到otherId衛星的路徑
    void printConstellationHopCountFileAndOutputCertainPath(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出某個特定時刻，行星群的shortest path狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過多少距離才會抵達另一個衛星)到sattrack/output.txt中
    void printConstellationDistanceFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出某個特定時刻，行星群的shortest path狀態，並且在terminal中印出由observerId的衛星到otherId衛星的路徑
    void printConstellationDistanceAndOutputCertainPath(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);    

    //印出根據parameter.txt設置位置的地面站，與星群中每一個衛星一天中有那些時間是可以連線的
    void printStationAllSatConnectionTime(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出自parameter.txt中的startTime到endTime每秒的衛星間方位角關係及角度差較小的ISL裝置設置角度
    void printConstellationISLdeviceInfo(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出根據parameter.txt設置位置的地面站，與星群中每一個衛星一天中有那些時間是可以連線的
    void printStationCoverSatsPerSecond(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出不同緯度的地面站86400秒中，有幾秒是有被衛星覆蓋的
    void printDifferentLatitudeCoverTimeOfDay(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出不同緯度的地面站86400秒中，有幾秒是有被至少n顆衛星覆蓋的
    void printDifferentLatitudeNSatCoverTimeOfDay(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);
    
    //印出不同緯度的地面站86400秒中，平均/最小/最大的可連線衛星數量是多少
    void printDifferentLatitudeConnectedCountOfDay(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出地面站對各個衛星一天中對星群中各個衛星的時間總合，總連線時間最長的衛星，總連線時間最短的衛星，以及各個衛星總連線時間的平均
    void printGroundStationConnectingInfo(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出設定區域(多個地面站)內對各個衛星一天中對星群中各個衛星的時間總合，總連線時間最長的衛星，總連線時間最短的衛星，以及各個衛星總連線時間的平均，多個地面站中只要任一個可以連上，就算那一秒鐘可以連上
    void printAreaConnectingInfo(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //印出根據parameter.txt設置的區域(多個地面站)，與星群中每一個衛星一天中有那些時間是可以連線的
    void printAreaAllSatConnectionTime(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);

    //模擬計算連結失效率，根據所設置的模擬次數，模擬星群的Link要損壞多少個才會發生連結失效，並將最後的分布統計數據印到所設定的output檔案中
    void simulateLinkbreakingStatistics(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable, std::map<int, std::map<int, bool>> &closeLinksTable);

    //模擬計算衛星隨機壞掉的連結失效率，根據所設置的模擬次數，模擬星群的衛星要損壞多少個才會發生連結失效，並將最後的分布統計數據印到所設定的output檔案中
    void simulateSatFailStatistics(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable, std::map<int, std::map<int, bool>> &closeLinksTable);
}

#endif