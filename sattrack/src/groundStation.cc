
#include "groundStation.h"
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>

#include <iostream>
#include <vector>
#include <utility>
#include <bitset>
#include <algorithm>

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
        double doubleElevation =  Util::RadiansToDegrees(topo.elevation);
        int elevation = (int)(doubleElevation+0.5-(doubleElevation<0));
        // std::cout<<"elevation: "<< elevation <<", "<<topo.range<<"\n";
        // if(elevation > groundStationAcceptableElevation && topo.range < groundStationAcceptableDistance){
        //     std::cout<<"sat"<<sat.getId()<<" at time "<<time<<": elevation: "<< elevation <<", "<<topo.range<<"\n";
        //     return true;
        // }
        return elevation >= groundStationAcceptableElevation && topo.range <= groundStationAcceptableDistance;
    }

    //回傳一整天86400秒中，地面站對特定一顆衛星的連線狀態
    std::bitset<86400> groundStation::getConnectionOfDay(satellite::satellite &sat, int groundStationAcceptableElevation, int groundStationAcceptableDistance){
        std::bitset<86400> res;
        for(size_t t = 0; t < 86400; ++t){
            res[t] = this->judgeConnection(sat, t, groundStationAcceptableElevation, groundStationAcceptableDistance);
        }
        return res;
    }

    //回傳特定時間地面站可以連線到的所有衛星ID的list
    std::vector<int> groundStation::getSecondCoverSatsList(std::map<int, satellite::satellite> &satellites,int time, int groundStationAcceptableElevation, int groundStationAcceptableDistance){
        std::vector<int> availableSatsList;
        for(auto &satPair: satellites){
            if(this->judgeConnection(satPair.second, time, groundStationAcceptableElevation, groundStationAcceptableDistance)){
                availableSatsList.push_back(satPair.second.getId());
            }
        }
        return availableSatsList;
    }

    //回傳一整天86400秒中，地面站每秒是否有任何一顆衛星是可以連上的
    std::bitset<86400> groundStation::getCoverTimeOfDay(std::map<int, satellite::satellite> &satellites, int groundStationAcceptableElevation, int groundStationAcceptableDistance){
        std::bitset<86400> availabilityOfDay;
        std::vector<int> availableSatsList = this->getSecondCoverSatsList(satellites, 0, groundStationAcceptableElevation, groundStationAcceptableDistance);
        for(size_t t = 0; t < 86400; ++t){
            std::vector<int>::iterator iter;
            for (iter = availableSatsList.begin(); iter != availableSatsList.end(); ){ //刪除List中斷線的衛星
                if(!this->judgeConnection(satellites.at(*iter), t, groundStationAcceptableElevation, groundStationAcceptableDistance))
                    iter = availableSatsList.erase(iter);
                else
                    ++iter;
            }
            if(!availableSatsList.empty()){
                availabilityOfDay[t] = true;
            }
            else{
                availableSatsList = this->getSecondCoverSatsList(satellites, t, groundStationAcceptableElevation, groundStationAcceptableDistance);
                availabilityOfDay[t] = !availableSatsList.empty();
            }
        }
        return availabilityOfDay;

        // slow version
        // std::bitset<86400> availabilityOfDay;
        // std::vector<int> availableSatsList = this->getSecondCoverSatsList(satellites, 0, groundStationAcceptableElevation, groundStationAcceptableDistance);
        // for(size_t t = 0; t < 86400; ++t){
        //     availableSatsList = this->getSecondCoverSatsList(satellites, t, groundStationAcceptableElevation, groundStationAcceptableDistance);
        //     availabilityOfDay[t] = !availableSatsList.empty();
        // }
        // return availabilityOfDay;
    }

    //回傳一個vector，裡面是紀錄每個connection state改變的時間點，及他是Link Breaking(false)還是connecting(true)
    std::vector<std::pair<size_t, bool>> groundStation::getStateChangeInfoOfDay(satellite::satellite &sat, int groundStationAcceptableElevation, int groundStationAcceptableDistance){
        std::bitset<86400> stateOfDay = this->getConnectionOfDay(sat, groundStationAcceptableElevation, groundStationAcceptableDistance);
        std::vector<std::pair<size_t, bool>> stateChangeInfoOfDay = getStateChangeInfo(stateOfDay);
        return stateChangeInfoOfDay;
    }
}
