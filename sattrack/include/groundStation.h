#ifndef GOUND_STATION_H
#define GOUND_STATION_H
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include "satellite.h"

#include <vector>
#include <utility>
#include <bitset>



namespace groundStation
{
    //根據input parameter stateOfDay，回傳一個vector，裡面是紀錄每個connection state改變的時間點，及他是Link Breaking(false)還是connecting(true)
    std::vector<std::pair<size_t, bool>> getStateChangeInfo(std::bitset<86400> &stateOfDay);
    
    class groundStation{
    public:
        groundStation(const double latitude, const double longitude, const double altitude);
        Observer getObserver();

        //判斷地面站在某個特定時間能不能連上特定一顆衛星
        bool judgeConnection(satellite::satellite &sat,int time, int groundStationAcceptableElevation, int groundStationAcceptableDistance);

        //回傳一整天86400秒中，地面站對特定一顆衛星的連線狀態
        std::bitset<86400> getConnectionOfDay(satellite::satellite &sat, int groundStationAcceptableElevation, int groundStationAcceptableDistance);

        //回傳一整天86400秒中，地面站每秒是否有任何一顆衛星是可以連上的
        std::bitset<86400> getCoverTimeOfDay(std::map<int, satellite::satellite> &satellites, int groundStationAcceptableElevation, int groundStationAcceptableDistance);
        
        //回傳特定時間地面站可以連線到的所有衛星ID的list
        std::vector<int> getSecondCoverSatsList(std::map<int, satellite::satellite> &satellites,int time, int groundStationAcceptableElevation, int groundStationAcceptableDistance);

        //回傳一個vector，裡面是紀錄每個connection state改變的時間點，及他是Link Breaking(false)還是connecting(true)
        std::vector<std::pair<size_t, bool>> getStateChangeInfoOfDay(satellite::satellite &sat, int groundStationAcceptableElevation, int groundStationAcceptableDistance);


    private:
        Observer obs;
    };
    

}
#endif