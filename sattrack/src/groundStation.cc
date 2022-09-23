
#include "groundStation.h"
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>

#include <iostream>
#include <vector>
#include <utility>
#include <bitset>

namespace groundStation
{
    //根據input parameter stateOfDay，回傳一個vector，裡面是紀錄每個connection state改變的時間點，及他是Link Breaking(false)還是connecting(true)
    std::vector<std::pair<size_t, bool>> getStateChangeInfo(std::bitset<86400> &stateOfDay){
        std::vector<std::pair<size_t, bool>> stateChangeInfo;
        bool currentState = false;
        for(size_t i = 0; i < 86400; ++i){
            if(stateOfDay[i] != currentState){
                currentState = !currentState;
                stateChangeInfo.push_back(std::make_pair(i, currentState));
            }
        }
        return stateChangeInfo;
    }


    groundStation::groundStation(const double latitude,const double longitude,const double altitude): obs(latitude, longitude, altitude){}
    Observer groundStation::getObserver(){
        return this->obs;
    }
    
    //判斷地面站在某個特定時間能否連上特定一顆衛星
    bool groundStation::judgeConnection(satellite::satellite &sat,int time, int groundStationAcceptableElevation, int groundStationAcceptableDistance){
        DateTime t = sat.getTle().Epoch().AddSeconds(time);
        Eci eci = sat.getSgp4().FindPosition(t);
        CoordTopocentric topo = this->getObserver().GetLookAngle(eci);
        double elevation =  Util::RadiansToDegrees(topo.elevation);
        // std::cout<<"elevation: "<< elevation <<", "<<topo.range<<"\n";
        // if(elevation > groundStationAcceptableElevation && topo.range < groundStationAcceptableDistance){
        //     std::cout<<"sat"<<sat.getId()<<" at time "<<time<<": elevation: "<< elevation <<", "<<topo.range<<"\n";
        //     return true;
        // }
        return elevation > groundStationAcceptableElevation && topo.range < groundStationAcceptableDistance;
    }

    //回傳一整天86400秒中，地面站對特定一顆衛星的連線狀態
    std::bitset<86400> groundStation::getConnectionOfDay(satellite::satellite &sat, int groundStationAcceptableElevation, int groundStationAcceptableDistance){
        std::bitset<86400> res;
        for(size_t t = 0; t < 86400; ++t){
            res[t] = this->judgeConnection(sat, t, groundStationAcceptableElevation, groundStationAcceptableDistance);
        }
        return res;
    }

    //回傳一個vector，裡面是紀錄每個connection state改變的時間點，及他是Link Breaking(false)還是connecting(true)
    std::vector<std::pair<size_t, bool>> groundStation::getStateChangeInfoOfDay(satellite::satellite &sat, int groundStationAcceptableElevation, int groundStationAcceptableDistance){
        std::bitset<86400> stateOfDay = this->getConnectionOfDay(sat, groundStationAcceptableElevation, groundStationAcceptableDistance);
        std::vector<std::pair<size_t, bool>> stateChangeInfoOfDay = getStateChangeInfo(stateOfDay);
        return stateChangeInfoOfDay;
    }
}
