#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include <string>
#include <iostream>
#include <algorithm> //min()
#include <utility>
#include <map>
#include <bitset>
#include <set>
#include <climits>
#include "rectifyAzimuth.h"
#include "satellite.h"
#include "AER.h"

namespace satellite
{
    //檢查對其他衛星的方位角是否符合可連線標準
    bool judgeAzimuth(double ISLdirAzimuth, double AcceptableAzimuthDif, double otherSatAzimuth){
        double AngleDif1 = ISLdirAzimuth-otherSatAzimuth;
        double AngleDif2 = otherSatAzimuth-ISLdirAzimuth;
        if(AngleDif1 < 0) AngleDif1+=360;
        if(AngleDif2 < 0) AngleDif2+=360;
        return std::min(AngleDif1, AngleDif2) < AcceptableAzimuthDif;
    }

    //檢查對其他衛星的仰角是否符合可連線標準
    bool judgeElevation(double AcceptableElevationDif, double otherSatElevation){
        if(otherSatElevation > 0){
            return otherSatElevation < AcceptableElevationDif;
        }
        else{
            return otherSatElevation > -AcceptableElevationDif;
        }
        return true;
    }

    //檢查對其他衛星的距離是否符合可連線標準
    bool judgeRange(double AcceptableRange, double otherSatRange){
        return otherSatRange < AcceptableRange;
    }

    //將衛星編號轉成二維陣列的index
    size_t satIdToIndex(int SatId, size_t satCountPerOrbit){
        const int satNum = SatId%100; 
        const int orbitNum = (SatId-satNum)/100;
        return  (size_t)(orbitNum-1)*satCountPerOrbit + (size_t)(satNum-1);
    }
    //將二維陣列的index轉成衛星編號
    int indexToSatId(size_t IndexNumber, size_t satCountPerOrbit){
        size_t satNum = IndexNumber%satCountPerOrbit+1;
        size_t orbitNum = IndexNumber/satCountPerOrbit+1;
        return  orbitNum*100+satNum;
    }

    //回傳某個特定時刻，行星群的連線狀態(totalSatCount*totalSatCount的對稱二維vetcor，可以連的話填上距離，不可連的話填上0，自己連自己也是填0)
    std::vector<std::vector<int>> getConstellationState(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites){
        std::vector<std::vector<int>> constellationState(totalSatCount, std::vector<int>(totalSatCount, -1));
        for(size_t i = 0; i < totalSatCount; ++i){
            constellationState[i][i] = 0; //衛星自己
        }
        for(auto &sat:satellites){
            size_t satIndex = satIdToIndex(sat.second.getId(), satCountPerOrbit);
            size_t rightSatIndex = satIdToIndex(sat.second.getRightSatId(), satCountPerOrbit);
            size_t leftSatIndex = satIdToIndex(sat.second.getLeftSatId(), satCountPerOrbit);
            size_t frontSatIndex = satIdToIndex(sat.second.getFrontSatId(), satCountPerOrbit);
            size_t backSatIndex = satIdToIndex(sat.second.getBackSatId(), satCountPerOrbit);
            // std::cout<< "satIndex: "<<satIndex<<", rightSatIndex: "<<rightSatIndex<<", leftSatIndex: "<<leftSatIndex<<", frontSatIndex: "<<frontSatIndex<<", backSatIndex: "<<backSatIndex<<"\n";
            if(constellationState[satIndex][rightSatIndex] < 0){
                constellationState[satIndex][rightSatIndex] = constellationState[rightSatIndex][satIndex] = sat.second.judgeRightISLwithPAT(time, PAT_time, acceptableAER_diff);
            }
            if(constellationState[satIndex][leftSatIndex] < 0){
                constellationState[satIndex][leftSatIndex] = constellationState[leftSatIndex][satIndex] = sat.second.judgeLeftISLwithPAT(time, PAT_time, acceptableAER_diff);
            }
            if(constellationState[satIndex][frontSatIndex] < 0){
                constellationState[satIndex][frontSatIndex] = constellationState[frontSatIndex][satIndex] = sat.second.getAER(time, sat.second.getFrontSat()).R;
            }
            if(constellationState[satIndex][backSatIndex] < 0){
                constellationState[satIndex][backSatIndex] = constellationState[backSatIndex][satIndex] = sat.second.getAER(time, sat.second.getBackSat()).R;
            }
        }
        for(auto &row: constellationState){
            for(auto &i: row){
                if(i < 0) 
                    i = 0;
            }
        }
        return constellationState;
    }

    //回傳某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)
    std::vector<std::vector<int>> getConstellationHopCount(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites){
        std::vector<std::vector<int>> constellationHopCount = getConstellationState(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
        for(size_t rowIdx = 0; rowIdx < totalSatCount; ++rowIdx){
            for(size_t colIdx = 0; colIdx < totalSatCount; ++colIdx){
                if(constellationHopCount[rowIdx][colIdx] == 0){
                    if(rowIdx != colIdx){
                        constellationHopCount[rowIdx][colIdx] = INT_MAX;
                    }
                }
                else{
                    constellationHopCount[rowIdx][colIdx] = 1;
                }
            }
        }
        //Floyd Warshall Algorithm
        for (size_t k = 0; k < totalSatCount; k++) {
            // Pick all vertices as source one by one
            for (size_t i = 0; i < totalSatCount; i++) {
                // Pick all vertices as destination for the
                // above picked source
                for (size_t j = 0; j < totalSatCount; j++) {
                    // If vertex k is on the shortest path from
                    // i to j, then update the value of
                    // dist[i][j]
                    if ((constellationHopCount[k][j] != INT_MAX && constellationHopCount[i][k] != INT_MAX) && constellationHopCount[i][j] > (constellationHopCount[i][k] + constellationHopCount[k][j]))
                        constellationHopCount[i][j] = constellationHopCount[i][k] + constellationHopCount[k][j];
                }
            }
        }
        return constellationHopCount;
    }

    //回傳某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)，同時記錄中間點(shortest path經過的點)，以用來計算shortest path
    std::vector<std::vector<int>> getConstellationHopCountRecordMedium(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites, std::vector<std::vector<int>> &medium){
        std::vector<std::vector<int>> constellationHopCount = getConstellationState(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
        for(size_t rowIdx = 0; rowIdx < totalSatCount; ++rowIdx){
            for(size_t colIdx = 0; colIdx < totalSatCount; ++colIdx){
                if(constellationHopCount[rowIdx][colIdx] == 0){
                    if(rowIdx != colIdx){
                        constellationHopCount[rowIdx][colIdx] = INT_MAX;
                    }
                }
                else{
                    constellationHopCount[rowIdx][colIdx] = 1;
                }
            }
        }
        //Floyd Warshall Algorithm
        for (size_t k = 0; k < totalSatCount; k++) {
            // Pick all vertices as source one by one
            for (size_t i = 0; i < totalSatCount; i++) {
                // Pick all vertices as destination for the
                // above picked source
                for (size_t j = 0; j < totalSatCount; j++) {
                    // If vertex k is on the shortest path from
                    // i to j, then update the value of
                    // dist[i][j]
                    if ((constellationHopCount[k][j] != INT_MAX && constellationHopCount[i][k] != INT_MAX) && constellationHopCount[i][j] > (constellationHopCount[i][k] + constellationHopCount[k][j])){
                        constellationHopCount[i][j] = constellationHopCount[i][k] + constellationHopCount[k][j];
                        medium[i][j] = k;
                    }
                }
            }
        }
        return constellationHopCount;
    }

    //回傳某個特定時刻，行星群的shortest path狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過多少距離才會抵達另一個衛星)
    std::vector<std::vector<int>> getConstellationShortestPath(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites){
        std::vector<std::vector<int>> constellationShortestPath = getConstellationState(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
        for(size_t rowIdx = 0; rowIdx < totalSatCount; ++rowIdx){
            for(size_t colIdx = 0; colIdx < totalSatCount; ++colIdx){
                if(constellationShortestPath[rowIdx][colIdx] == 0){
                    if(rowIdx != colIdx){
                        constellationShortestPath[rowIdx][colIdx] = INT_MAX;
                    }
                }
            }
        }
        //Floyd Warshall Algorithm
        for (size_t k = 0; k < totalSatCount; k++) {
            // Pick all vertices as source one by one
            for (size_t i = 0; i < totalSatCount; i++) {
                // Pick all vertices as destination for the
                // above picked source
                for (size_t j = 0; j < totalSatCount; j++) {
                    // If vertex k is on the shortest path from
                    // i to j, then update the value of
                    // dist[i][j]
                    if ((constellationShortestPath[k][j] != INT_MAX && constellationShortestPath[i][k] != INT_MAX) && constellationShortestPath[i][j] > (constellationShortestPath[i][k] + constellationShortestPath[k][j]))
                        constellationShortestPath[i][j] = constellationShortestPath[i][k] + constellationShortestPath[k][j];
                }
            }
        }
        return constellationShortestPath;
    }

    //回傳某個特定時刻，行星群的shortest path狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過多少距離才會抵達另一個衛星)，同時記錄中間點(shortest path經過的點)，以用來計算shortest path
    std::vector<std::vector<int>> getConstellationShortestPathRecordMedium(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites, std::vector<std::vector<int>> &medium){
        std::vector<std::vector<int>> constellationShortestPath = getConstellationState(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
        for(size_t rowIdx = 0; rowIdx < totalSatCount; ++rowIdx){
            for(size_t colIdx = 0; colIdx < totalSatCount; ++colIdx){
                if(constellationShortestPath[rowIdx][colIdx] == 0){
                    if(rowIdx != colIdx){
                        constellationShortestPath[rowIdx][colIdx] = INT_MAX;
                    }
                }
            }
        }
        //Floyd Warshall Algorithm
        for (size_t k = 0; k < totalSatCount; k++) {
            // Pick all vertices as source one by one
            for (size_t i = 0; i < totalSatCount; i++) {
                // Pick all vertices as destination for the
                // above picked source
                for (size_t j = 0; j < totalSatCount; j++) {
                    // If vertex k is on the shortest path from
                    // i to j, then update the value of
                    // dist[i][j]
                    if ((constellationShortestPath[k][j] != INT_MAX && constellationShortestPath[i][k] != INT_MAX) && constellationShortestPath[i][j] > (constellationShortestPath[i][k] + constellationShortestPath[k][j])){
                        constellationShortestPath[i][j] = constellationShortestPath[i][k] + constellationShortestPath[k][j];
                        medium[i][j] = k;
                    }
                }
            }
        }
        return constellationShortestPath;
    }    

    std::vector<int> getPath(size_t satCountPerOrbit, size_t sourceSatId, size_t destSatId, const std::vector<std::vector<int>> &medium, std::vector<std::vector<int>> shortestPath){
        // std::cout<<"start getPath function\n";
        std::vector<int> path;
        size_t source = satIdToIndex(sourceSatId, satCountPerOrbit);
        size_t dest = satIdToIndex(destSatId, satCountPerOrbit);
        if(shortestPath[source][dest] == INT_MAX){
            path.push_back(-1); //-1代表沒路 XD
            return path;
        }
        path.push_back(sourceSatId);
        // std::cout<<"start find path function\n";
        find_path(satCountPerOrbit, source, dest, path, medium);
        path.push_back(destSatId);
        return path;
    }

    //getPath function helper
    void find_path(size_t satCountPerOrbit, size_t source, size_t dest, std::vector<int> &path, const std::vector<std::vector<int>> &medium){
        // std::cout<<"medium: "<<medium[source][dest]<<"\n";
        if (medium[source][dest] == -1) return; // 沒有中繼點就結束

        find_path(satCountPerOrbit, source, (size_t)medium[source][dest], path, medium);     // 前半段最短路徑
        path.push_back(indexToSatId((size_t)medium[source][dest], satCountPerOrbit));         // 中繼點
        find_path(satCountPerOrbit, (size_t)medium[source][dest], dest, path, medium);     // 後半段最短路徑        
    }

    //建出ISLtable讓每個衛星可以指到屬於自己的2個ISL上
    std::map<std::set<int>, ISL> getISLtable(std::map<int, satellite> &satellites){
        std::map<std::set<int>, ISL> ISLtable;
        for(auto &sat:satellites){
            int satId = sat.second.getId();
            int rightId = sat.second.getRightSatId();
            int leftId = sat.second.getLeftSatId();
            ISL ISLright(satId, rightId);
            ISL ISLleft(satId, leftId);
            ISLtable.emplace(ISLright.getSatIdPair(),ISLright);
            ISLtable.emplace(ISLleft.getSatIdPair(),ISLleft);
        }
        // std::cout<<ISLtable.size()<<"\n";
        return ISLtable;
    }

    //計算出所有ISL的stateOfDay
    void setupAllISLstateOfDay(int PATtime, const AER &acceptableAER_diff, std::map<int, satellite> &satellites){
        for(auto &sat: satellites){
            sat.second.getRightISLstateOfDay(PATtime, acceptableAER_diff);
            sat.second.getLeftISLstateOfDay(PATtime, acceptableAER_diff);           
        }          
    }

    //計算出所有ISL的stateOfDay(且左側右側ISL可以連P+1或P-1軌道(沒有固定)，尚未考慮PAT)
    void adjustableISLdeviceSetupAllISLstateOfDay(int PATtime, const AER &acceptableAER_diff, std::map<int, satellite> &satellites, std::map<std::set<int>, ISL> &ISLtable){
        // satellite sat = satellites.at(705);
        for(size_t time = 0; time < 86400; ++time){
            for(auto &sat: satellites){
                sat.second.getRightISL().setSecondState(time, sat.second.adjustableISLdeviceJudgeRight(time, acceptableAER_diff));
            }  
            for(auto &sat: satellites){
                sat.second.setCertainTimeISLdeviceState(time, sat.second.getCurrentISLdeviceState());
            }              
        }
        for(auto &pair: ISLtable){
            pair.second.setStateOfDay();
        }                
    }

    //reset所有ISL的stateOfDay(標記成尚未計算過)
    void resetAllISL(std::map<std::set<int>, ISL> &ISLtable){
        for(auto &pair: ISLtable){
            pair.second.resetStateOfDay();
        }
    }

    //將所有衛星換回state 0
    void resetAllSat(std::map<int, satellite> &satellites){
        for(auto &satPair: satellites){
            satPair.second.resetState();
        }
    }

    /*------------------ISL class 的函式------------------*/
    ISL::ISL(int sat1, int sat2):calculated(false){
        satelliteIdPair = std::set<int>({sat1, sat2});
    }

    std::set<int> ISL::getSatIdPair(){
        return satelliteIdPair;
    }

    bool ISL::alreadyCalculate(){
        return calculated;
    }

    void ISL::printISL2SatId(){
        std::cout<<"sat1: "<<*satelliteIdPair.begin()<<", sat2: "<<*satelliteIdPair.rbegin()<<"\n";
    }

    std::bitset<86400>  ISL::getStateOfDay(){
        return this->stateOfDay;
    }
    
    void ISL::setSecondState(size_t time, bool state){
        this->stateOfDay[time] = state;
    }

    bool ISL::getSecondState(size_t time){
        return this->stateOfDay[time];
    }

    void ISL::setStateOfDay(std::bitset<86400> _stateOfDay){
        stateOfDay = _stateOfDay;
        calculated = true;
    }

    void ISL::setStateOfDay(){
        calculated = true;
    }

    //reset標記成尚未計算過stateOfDay
    void ISL::resetStateOfDay(){
        this->calculated = false;
    }

    //回傳一個vector，裡面是紀錄每個connection state改變的時間點，及他是Link Breaking(false)還是connecting(true)
    std::vector<std::pair<int, bool>> ISL::getStateChangeInfo(){
        std::vector<std::pair<int, bool>> stateChangeInfo;
        bool currentState = stateOfDay[0];
        for(size_t i = 1; i < 86400; ++i){
            if(stateOfDay[i] != currentState){
                currentState = !currentState;
                stateChangeInfo.push_back(std::make_pair(i, currentState));
            }
        }
        return stateChangeInfo;
    }
    /*------------------ISL class  end------------------*/

    /*------------------Satellite class 的函式  start------------------*/
    //satellite的建構子，初始化衛星的各個資訊
    satellite::satellite(std::string constellationType, Tle _tle, SGP4 _sgp4, int _id, int ISLfrontAngle, int ISLrightAngle, int ISLbackAngle, int ISLleftAngle) : tle(_tle), sgp4(_sgp4), id(_id) {
        if(constellationType == "7P_16Sats"){
            neighbors = std::vector<std::pair<int,double>>(4);//right left front back 的 <satId, ISLangle>
            neighbors[0].second = ISLrightAngle;
            neighbors[1].second = ISLleftAngle;
            neighbors[2].second = ISLfrontAngle;
            neighbors[3].second = ISLbackAngle;
            const int satNum = id%100;
            const int orbitNum = (id-satNum)/100;
            // std::cout<<"orbitNum: "<<orbitNum<<", satNum: "<<satNum<<"\n";
            //right
            neighbors[0].first = satNum - 2 < 1 ? 100*(orbitNum+1)+satNum-2+16 : 100*(orbitNum+1)+satNum-2;
            if(orbitNum == 7){
                neighbors[0].first = 100+satNum;
            }
            //left
            neighbors[1].first = satNum + 2 > 16 ? 100*(orbitNum-1)+satNum+2-16 : 100*(orbitNum-1)+satNum+2;
            if(orbitNum == 1){
                neighbors[1].first = 700+satNum;
            }
            //front
            neighbors[2].first = satNum == 16 ? 100*orbitNum+1 : 100*orbitNum+satNum+1;
            //back
            neighbors[3].first = satNum == 1 ? 100*orbitNum+16 : 100*orbitNum+satNum-1;
        }
        else if(constellationType == "6P_22Sats"){
            neighbors = std::vector<std::pair<int,double>>(4);//right left front back 的 <satId, ISLangle>
            neighbors[0].second = ISLrightAngle;
            neighbors[1].second = ISLleftAngle;
            neighbors[2].second = ISLfrontAngle;
            neighbors[3].second = ISLbackAngle;
            const int satNum = id%100;
            const int orbitNum = (id-satNum)/100;
            // std::cout<<"orbitNum: "<<orbitNum<<", satNum: "<<satNum<<"\n";
            //right
            neighbors[0].first = satNum - 3 < 1 ? 100*(orbitNum+1)+satNum-3+22 : 100*(orbitNum+1)+satNum-3;
            if(orbitNum == 6){
                neighbors[0].first = satNum - 2 < 1 ? 100+satNum-2+22 : 100+satNum-2;
            }
            //left
            neighbors[1].first = satNum + 3 > 22 ? 100*(orbitNum-1)+satNum+3-22 : 100*(orbitNum-1)+satNum+3;
            if(orbitNum == 1){
                neighbors[1].first = satNum + 2 > 22 ? 600+satNum+2-22 : 600+satNum+2;
            }
            //front
            neighbors[2].first = satNum == 22 ? 100*orbitNum+1 : 100*orbitNum+satNum+1;
            //back
            neighbors[3].first = satNum == 1 ? 100*orbitNum+22 : 100*orbitNum+satNum-1;        
        }
        else{
            std::cout<<"unknown constellationType!\n";
            exit(-1);
        }
    }

    //將四個衛星物件還有ISL的指標，指到對應的衛星衛星物件上
    void satellite::buildNeighborSatsAndISLs(std::map<int, satellite> &satellites, std::map<std::set<int>, ISL> &ISLtable){
        int selfSatId = this->getId();
        int rightSatId = this->getRightSatId();
        int leftSatId = this->getLeftSatId();
        this->rightSatPtr = &satellites.at(rightSatId);
        this->leftSatPtr = &satellites.at(leftSatId);
        this->frontSatPtr = &satellites.at(this->getFrontSatId());
        this->backSatPtr = &satellites.at(this->getBackSatId());
        this->leftISLptr = &ISLtable.at(std::set<int>({selfSatId,leftSatId}));
        this->rightISLptr = &ISLtable.at(std::set<int>({selfSatId,rightSatId}));
    }

    Tle satellite::getTle(){
        return tle;
    }

    SGP4 satellite::getSgp4(){
        return sgp4;
    }

    int satellite::getId(){
        return id;
    }

    //獲得某個時間點觀測另一個衛星的AER
    AER satellite::getAER(int time, satellite &other){
        DateTime dt = this->getTle().Epoch().AddSeconds(time);
        Eci observerEci = this->getSgp4().FindPosition(dt);
        Eci otherEci = other.getSgp4().FindPosition(dt);
        Observer obs(observerEci.ToGeodetic());
        CoordTopocentric topo = obs.GetLookAngle(otherEci);
        Vector position = observerEci.Position();
        Vector velocity = observerEci.Velocity();        
        Eigen::Vector3d observerECIvector(position.x, position.y, position.z); //觀測者的位置向量
        Eigen::Vector3d orbitDirectionVector(velocity.x, velocity.y, velocity.z); //觀測者的軌道方向向量
        Eigen::Vector3d northDirectionVector = rectifyAzimuth::getNorthDir(observerECIvector); //觀測者位置的地理北方向量
        double modification = rectifyAzimuth::angleDiff(northDirectionVector, orbitDirectionVector, observerECIvector); //地球坐標系與衛星坐標系的方位角原點角度差
        double rectifiedA = Util::RadiansToDegrees(topo.azimuth) - modification < 0 ? Util::RadiansToDegrees(topo.azimuth) - modification + 360 : Util::RadiansToDegrees(topo.azimuth) - modification;
        double E = Util::RadiansToDegrees(topo.elevation);
        double R = topo.range;
        AER ret(dt.ToString(), rectifiedA, E, R);
        return ret;
    }

    int satellite::getRightSatId(){
        return neighbors[0].first;
    }

    int satellite::getLeftSatId(){
        return neighbors[1].first;
    }

    int satellite::getFrontSatId(){
        return neighbors[2].first;
    }

    int satellite::getBackSatId(){
        return neighbors[3].first;
    }

    double satellite::getISLrightAngle(){
        return neighbors[0].second;
    }

    double satellite::getISLleftAngle(){
        return neighbors[1].second;
    }

    double satellite::getISLfrontAngle(){
        return neighbors[2].second;
    }

    double satellite::getISLbackAngle(){
        return neighbors[3].second;
    }

    satellite& satellite::getRightSat(){
        if(!rightSatPtr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<": rightSatPtr is NULL! please call satellite::buildNeighborSatsAndISLs to setup the pointers."<<"\n";
            exit(-1);
        }
        return *rightSatPtr;
    }

    satellite& satellite::getLeftSat(){
        if(!leftSatPtr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<": leftSatPtr is NULL! please call satellite::buildNeighborSatsAndISLs to setup the pointers."<<"\n";
            exit(-1);
        }        
        return *leftSatPtr;
    }

    satellite& satellite::getFrontSat(){
        if(!frontSatPtr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<": frontSatPtr is NULL! please call satellite::buildNeighborSatsAndISLs to setup the pointers."<<"\n";
            exit(-1);
        }        
        return *frontSatPtr;
    }

    satellite& satellite::getBackSat(){
        if(!backSatPtr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<": backSatPtr is NULL! please call satellite::buildNeighborSatsAndISLs to setup the pointers."<<"\n";
            exit(-1);
        }          
        return *backSatPtr;
    }

    ISL& satellite::getRightISL(){
        if(this->rightISLptr == nullptr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<" :rightISLptr is NULL! pls call function satellite::buildNeighborSatsAndISLs\n";
            exit(-1);
        }        
        return *rightISLptr;
    }

    ISL& satellite::getLeftISL(){
        if(this->leftISLptr == nullptr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<" :leftISLptr is NULL! pls call function satellite::buildNeighborSatsAndISLs\n";
            exit(-1);
        }        
        return *leftISLptr;
    }

    bool satellite::rightAlreadyCalculate(){      
        return this->getRightISL().alreadyCalculate();
    }

    bool satellite::leftAlreadyCalculate(){     
        return this->getLeftISL().alreadyCalculate();
    }

    int satellite::getCurrentISLdeviceState(){
        return this->ISLdeviceState;
    }

    void satellite::setCertainTimeISLdeviceState(size_t t, bool state){
        this->ISLsettingStateOfDay[t] = state;
    }    

    //回傳衛星所記錄的特定秒數的device state
    int satellite::getCertainTimeISLdeviceState(size_t t){
        return this->ISLsettingStateOfDay[t];
    }

    void satellite::changeState(){
        this->ISLdeviceState = !ISLdeviceState;
        std::swap(this->neighbors[0].second, this->neighbors[1].second);
    }

    void satellite::resetState(){
        if(this->ISLdeviceState == 1)
            this->changeState();
    }  

    std::bitset<86400> satellite::getISLsettingStateOfDay(){
        return this->ISLsettingStateOfDay;
    }

    //設定右方ISL一天中86400秒的連線狀態
    void satellite::setRightStateOfDate(std::bitset<86400> stateOfDay){ 
        this->getRightISL().setStateOfDay(stateOfDay);
    }

    //設定左方ISL一天中86400秒的連線狀態
    void satellite::setLeftStateOfDate(std::bitset<86400> stateOfDay){       
        this->getLeftISL().setStateOfDay(stateOfDay);
    }

    //印出每一個相鄰衛星的編號
    void satellite::printNeighborId(){
        std::cout<<"neighbors of sat"<<id<<" ---> right: "<<neighbors[0].first<<", left: "<<neighbors[1].first<<", front: "<<neighbors[2].first<<", back: "<<neighbors[3].first;
        std::cout<<",  ISLangle of sat"<<id<<" ---> right: "<<neighbors[0].second<<", left: "<<neighbors[1].second<<", front: "<<neighbors[2].second<<", back: "<<neighbors[3].second<<"\n";
    }

    //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeRightConnectability(int time, const AER &acceptableAER_diff){
        AER rightSatAER = this->getAER(time, this->getRightSat());
        //if可以連到右方衛星
        if(judgeAzimuth(this->getISLrightAngle(), acceptableAER_diff.A, rightSatAER.A) && judgeElevation(acceptableAER_diff.E, rightSatAER.E) && judgeRange(acceptableAER_diff.R, rightSatAER.R)){
            return (int)rightSatAER.R;
        }
        return 0;
    }  
 
    //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeRightConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &rightSatAER){
        rightSatAER = this->getAER(time, this->getRightSat());
        connectionState[0] = judgeAzimuth(this->getISLrightAngle(), acceptableAER_diff.A, rightSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, rightSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, rightSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeLeftConnectability(int time, const AER &acceptableAER_diff){
        AER leftSatAER = this->getAER(time, this->getLeftSat());
        if(judgeAzimuth(this->getISLleftAngle(), acceptableAER_diff.A, leftSatAER.A) && judgeElevation(acceptableAER_diff.E, leftSatAER.E) && judgeRange(acceptableAER_diff.R, leftSatAER.R)){
            return leftSatAER.R;
        }
        return 0;
    } 

    //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeLeftConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &leftSatAER){
        leftSatAER = this->getAER(time, this->getLeftSat());
        connectionState[0] = judgeAzimuth(this->getISLleftAngle(), acceptableAER_diff.A, leftSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, leftSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, leftSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳前方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeFrontConnectability(int time, const AER &acceptableAER_diff){
        AER frontSatAER = this->getAER(time, this->getFrontSat());
        if(judgeAzimuth(this->getISLfrontAngle(), acceptableAER_diff.A, frontSatAER.A) && judgeElevation(acceptableAER_diff.E, frontSatAER.E) && judgeRange(acceptableAER_diff.R, frontSatAER.R)){
            return frontSatAER.R;
        }
        return 0;
    } 

    //回前方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeFrontConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &frontSatAER){
        frontSatAER = this->getAER(time, this->getFrontSat());
        connectionState[0] = judgeAzimuth(this->getISLfrontAngle(), acceptableAER_diff.A, frontSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, frontSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, frontSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳後方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeBackConnectability(int time, const AER &acceptableAER_diff){
        AER backSatAER = this->getAER(time, this->getBackSat());
        if(judgeAzimuth(this->getISLbackAngle(), acceptableAER_diff.A, backSatAER.A) && judgeElevation(acceptableAER_diff.E, backSatAER.E) && judgeRange(acceptableAER_diff.R, backSatAER.R)){
            return backSatAER.R;
        }
        return 0;
    } 

    //回後方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeBackConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &backSatAER){
        backSatAER = this->getAER(time, this->getBackSat());
        connectionState[0] = judgeAzimuth(this->getISLbackAngle(), acceptableAER_diff.A, backSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, backSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, backSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeRightISL(int time, const AER &acceptableAER_diff){
        // if(this->getCurrentISLdeviceState() != this->getRightSat().getCurrentISLdeviceState()){
        //     return 0;
        // }
        int right = this->judgeRightConnectability(time, acceptableAER_diff);
        int left = this->getRightSat().judgeLeftConnectability(time, acceptableAER_diff);
        if(left && right){
            return left;
        }
        return 0;
    } 

    //回傳特定時刻可否建立左方的ISL(要彼此可以連線到彼此才可以建立)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeLeftISL(int time, const AER &acceptableAER_diff){
        // if(this->getCurrentISLdeviceState() != this->getLeftSat().getCurrentISLdeviceState()){
        //     return 0;
        // }        
        int right = this->judgeLeftConnectability(time, acceptableAER_diff);
        int left = this->getLeftSat().judgeRightConnectability(time, acceptableAER_diff);
        if(left && right){
            return left;
        }
        return 0;
    } 

    //回傳考慮PAT time一天(86400秒)中衛星右方ISL的可連性
    std::bitset<86400> satellite::getRightISLstateOfDay(int PATtime, const AER &acceptableAER_diff){
        if(this->rightAlreadyCalculate()){ //若以前計算過，就直接回傳之前算過存下來的
            return this->getRightISL().getStateOfDay();
        }
        std::bitset<86400> stateOfDay;
        int duration = PATtime;
        for(size_t time = 0; time < 86400; ++time){
            if(this->judgeRightISL(time, acceptableAER_diff)){
                if(duration == PATtime){
                    stateOfDay[time] = true;
                }
                else{
                    ++duration;
                }
            }
            else{
                duration = 0;
            }
        }
        this->getRightSat().setLeftStateOfDate(stateOfDay); //右方的衛星下次若要計算左方衛星整天的連線狀態，就不需計算，直接用現在算過存下來的
        return stateOfDay;
    }

    //回傳考慮PAT time一天(86400秒)中衛星右方ISL的可連性
    std::bitset<86400> satellite::getLeftISLstateOfDay(int PATtime, const AER &acceptableAER_diff){
        if(this->leftAlreadyCalculate()){ //若以前計算過，就直接回傳之前算過存下來的    
            return this->getLeftISL().getStateOfDay();
        }
        std::bitset<86400> stateOfDay;
        int duration = PATtime;
        for(size_t time = 0; time < 86400; ++time){
            if(this->judgeLeftISL(time, acceptableAER_diff)){
                if(duration == PATtime){
                    stateOfDay[time] = true;
                }
                else{
                    ++duration;
                }
            }
            else{
                duration = 0;
            }
        }
        this->getLeftSat().setRightStateOfDate(stateOfDay); //左方的衛星下次若要計算左方衛星整天的連線狀態，就不需計算，直接用現在算過存下來的
        return stateOfDay;
    }

    //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，且有考慮PAT，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeRightISLwithPAT(int time, int PAT_time, const AER &acceptableAER_diff){
        if(time < PAT_time){
            for(int setupTime = 0; setupTime < time; ++setupTime){
                if(this->judgeRightISL(setupTime, acceptableAER_diff) == 0){
                    return 0;
                }
            }
            return this->judgeRightISL(time, acceptableAER_diff);
        }
        for(int setupTime = time-PAT_time; setupTime < time; ++setupTime){
            if(this->judgeRightISL(setupTime, acceptableAER_diff) == 0){
                return 0;
            }
        }
        return this->judgeRightISL(time, acceptableAER_diff);
    }

    //回傳特定時刻可否建立左方的ISL(要彼此可以連線到彼此才可以建立)，且有考慮PAT，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeLeftISLwithPAT(int time, int PAT_time, const AER &acceptableAER_diff){
        if(time < PAT_time){
            for(int setupTime = 0; setupTime < time; ++setupTime){
                if(this->judgeLeftISL(setupTime, acceptableAER_diff) == 0){
                    return 0;
                }
            }
            return this->judgeLeftISL(time, acceptableAER_diff);
        }
        for(int setupTime = time-PAT_time; setupTime < time; ++setupTime){
            if(this->judgeLeftISL(setupTime, acceptableAER_diff) == 0){
                return 0;
            }
        }
        return this->judgeLeftISL(time, acceptableAER_diff);
    }

    //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，且左側右側ISL可以連P+1或P-1軌道(沒有固定)，尚未考慮PAT
    bool satellite::adjustableISLdeviceJudgeRight(int time, const AER &acceptableAER_diff){
        if(this->judgeRightISL(time, acceptableAER_diff))
            return true;         
        int selfState = this->getCurrentISLdeviceState();
        int rightSatState = this->getRightSat().getCurrentISLdeviceState();
        bool selfcanConnectLeft = this->judgeLeftISL(time, acceptableAER_diff);
        bool rightSatCanConnectRight = this->getRightSat().judgeRightISL(time, acceptableAER_diff);       
        //ISLdevice setting state same        
        if(selfState == rightSatState){
            if(!selfcanConnectLeft && !rightSatCanConnectRight){ //檢查是否可以拿另外一側的裝置來使用
                this->changeState();
                this->getRightSat().changeState();
                if(this->judgeRightISL(time, acceptableAER_diff))
                    return true;
                else{ //change back
                    this->changeState();
                    this->getRightSat().changeState();                        
                }                    
            }
            return false;            
        }
        //ISLdevice setting state different
        if(!selfcanConnectLeft && !rightSatCanConnectRight){
            selfState == 0 ? this->getRightSat().changeState() : this->changeState();//change to both state0
            if(this->judgeRightISL(time, acceptableAER_diff))
                return true;
            selfState == 0 ? this->getRightSat().changeState() : this->changeState();//change back 

            selfState == 1 ? this->getRightSat().changeState() : this->changeState();//change to both state1
            if(this->judgeRightISL(time, acceptableAER_diff))
                return true;
            selfState == 1 ? this->getRightSat().changeState() : this->changeState();//change back
        }
        else if(selfcanConnectLeft && !rightSatCanConnectRight){
            this->getRightSat().changeState();
            if(this->judgeRightISL(time, acceptableAER_diff))
                return true;
            this->getRightSat().changeState();  
            return false;          
        }
        else if(!selfcanConnectLeft && rightSatCanConnectRight){
            this->changeState();
            if(this->judgeRightISL(time, acceptableAER_diff))
                return true;
            this->changeState();  
            return false;  
        }
        //selfcanConnectLeft && rightSatCanConnectRight
        return false;
    }

    //回傳特定時刻可否建立左方的ISL(要彼此可以連線到彼此才可以建立)，且左側右側ISL可以連P+1或P-1軌道(沒有固定)，尚未考慮PAT
    bool satellite::adjustableISLdeviceJudgeLeft(int time, const AER &acceptableAER_diff){
        if(this->judgeLeftISL(time, acceptableAER_diff))
            return true;        
        int selfState = this->getCurrentISLdeviceState();
        int leftSatState = this->getLeftSat().getCurrentISLdeviceState();
        bool selfcanConnectRight = this->judgeRightISL(time, acceptableAER_diff);
        bool leftSatCanConnectLeft = this->getLeftSat().judgeLeftISL(time, acceptableAER_diff); 
        if(selfState == leftSatState){           
            if(!selfcanConnectRight && !leftSatCanConnectLeft){ //檢查是否可以拿另外一側的裝置來使用
                this->changeState();
                this->getLeftSat().changeState();
                if(this->judgeLeftISL(time, acceptableAER_diff))
                    return true;
                else{
                    this->changeState();
                    this->getLeftSat().changeState();                        
                }                    
            }
            return false;
        }
        //state different
        if(!selfcanConnectRight && !leftSatCanConnectLeft){
            selfState == 0 ?  this->getRightSat().changeState() : this->changeState();//change to both state0
            if(this->judgeLeftISL(time, acceptableAER_diff))
                return true;
            selfState == 0 ?  this->getRightSat().changeState() : this->changeState();//change back 

            selfState == 1 ?  this->getRightSat().changeState() : this->changeState();//change to both state1
            if(this->judgeRightISL(time, acceptableAER_diff))
                return true;
            selfState == 1 ?  this->getRightSat().changeState() : this->changeState();//change back     
            return false;
        }
        else if(selfcanConnectRight && !leftSatCanConnectLeft){
            this->getLeftSat().changeState();
            if(this->judgeLeftISL(time, acceptableAER_diff))
                return true;
            this->getLeftSat().changeState();  
            return false;          
        }
        else if(!selfcanConnectRight && leftSatCanConnectLeft){
            this->changeState();
            if(this->judgeLeftISL(time, acceptableAER_diff))
                return true;
            this->changeState();  
            return false;  
        }
        return false;//selfcanConnectRight && leftSatCanConnectLeft              
    }

    /*------------------Satellite class  end------------------*/
}