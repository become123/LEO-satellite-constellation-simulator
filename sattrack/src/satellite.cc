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
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include "rectifyAzimuth.h"
#include "satellite.h"
#include "AER.h"

namespace satellite
{
    double getAngleDiff(double angle1, double angle2){
        double AngleDif1 = angle1-angle2;
        double AngleDif2 = angle2-angle1;
        if(AngleDif1 < 0) AngleDif1+=360;
        if(AngleDif2 < 0) AngleDif2+=360;
        return std::min(AngleDif1, AngleDif2);
    }


    //檢查對其他衛星的方位角是否符合可連線標準
    bool judgeAzimuth(double ISLdirAzimuth, double AcceptableAzimuthDif, double otherSatAzimuth){
        return getAngleDiff(ISLdirAzimuth, otherSatAzimuth) < AcceptableAzimuthDif;
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
            size_t frontSatIndex = satIdToIndex(sat.second.getFrontSatId(), satCountPerOrbit);
            // std::cout<< "satIndex: "<<satIndex<<", rightSatIndex: "<<rightSatIndex<<", leftSatIndex: "<<leftSatIndex<<", frontSatIndex: "<<frontSatIndex<<", backSatIndex: "<<backSatIndex<<"\n";
                
            //設定跨軌道方向的Link(有判斷連線條件)
            constellationState[satIndex][rightSatIndex] = constellationState[rightSatIndex][satIndex] = sat.second.judgeRightISLwithPAT(time, PAT_time, acceptableAER_diff);
            //設定跨軌道方向的Link(無判斷連線條件)
            // if(!sat.second.rightLinkClosed()){
            //     constellationState[satIndex][rightSatIndex] = constellationState[rightSatIndex][satIndex] = sat.second.getAER(time, sat.second.getRightSat()).R;
            // }
            //設定同軌道方向的Link(無判斷連線條件)
            if(!sat.second.frontLinkClosed()){
                constellationState[satIndex][frontSatIndex] = constellationState[frontSatIndex][satIndex] = sat.second.getAER(time, sat.second.getFrontSat()).R;
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

    //判斷星群是否有任何兩個衛星無法經ISL抵達彼此
    bool judgeConstellationBreaking(const std::vector<std::vector<int>> &constellationHopCount){
        for(size_t i = 0; i < constellationHopCount.size(); ++i){
            for(size_t j = 0; j < constellationHopCount.size(); ++j){
                if(constellationHopCount[i][j] == INT_MAX){
                    return true;
                }
            }
        } 
        return false;       
    }

    //判斷星群是否有任何兩個衛星無法經ISL抵達彼此，不包含已經在模擬中壞掉的衛星
    bool judgeConstellationBreaking(const std::vector<std::vector<int>> &constellationHopCount,const std::set<int> &nonBrokenSatSet, size_t satCountPerOrbit){
        for(size_t i = 0; i < constellationHopCount.size(); ++i){
            for(size_t j = 0; j < constellationHopCount.size(); ++j){
                if(constellationHopCount[i][j] == INT_MAX && nonBrokenSatSet.find(indexToSatId(i, satCountPerOrbit)) != nonBrokenSatSet.end() && nonBrokenSatSet.find(indexToSatId(j, satCountPerOrbit)) != nonBrokenSatSet.end()){
                    return true;
                }
            }
        } 
        return false;       
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

    //獲得紀錄還有哪些Link是正常還沒壞掉或被關掉的set
    std::set<std::set<int>> getOpenLinkSet(std::map<int, satellite> &satellites){
        std::set<std::set<int>> openLinkSet;
        for(auto &satPair:satellites){
            int satId = satPair.second.getId();
            int rightId = satPair.second.getRightSatId();
            int leftId = satPair.second.getLeftSatId();
            int frontId = satPair.second.getFrontSatId();
            int backId = satPair.second.getBackSatId();
            openLinkSet.insert(std::set<int>({satId, frontId}));
            openLinkSet.insert(std::set<int>({satId, backId}));
            if(!satPair.second.rightLinkClosed()){
                openLinkSet.insert(std::set<int>({satId, rightId}));
            }
            if(!satPair.second.leftLinkClosed()){
                openLinkSet.insert(std::set<int>({satId, leftId}));
            }         
        }
        return openLinkSet;
    }

    //獲得紀錄還有哪些衛星是正常還沒壞掉的set(初始為所有衛星都是正常的)
    std::set<int> getNonBrokenSatSet(std::map<int, satellite> &satellites){
        std::set<int> nonBrokenSatSet;
        for(auto &satPair:satellites){
            nonBrokenSatSet.insert(satPair.first);        
        }
        return nonBrokenSatSet;
    }    

    //從尚可以使用的Link中，隨機選出一個Link關掉，模擬ISL壞掉的情形
    void randomCloseLink(std::map<int, satellite> &satellites, std::set<std::set<int>> &openLinkSet){
        srand( time(NULL) );
        // std::cout<<(int)openLinkSet.size()<<"\n";
        auto n = rand() % (int)openLinkSet.size();
        // std::cout<<"n = "<<n<<"\n";
        auto it = std::begin(openLinkSet);
        std::advance(it,n);
        int sat1 = *it->begin(), sat2 = *it->rbegin();
        // std::cout<<"close:("<<sat1<<","<<sat2<<")\n";
        satellites.at(sat1).closeLink(sat2);
        openLinkSet.erase(it);
    }

    //從尚可以使用的衛星中，隨機選出一個衛星壞掉(4個ISL都壞掉)，模擬衛星壞掉的情形
    void randomBreakSat(std::map<int, satellite> &satellites, std::set<int> &nonBrokenSatSet){
        srand( time(NULL) );
        // std::cout<<(int)openLinkSet.size()<<"\n";
        auto n = rand() % (int)nonBrokenSatSet.size();
        // std::cout<<"n = "<<n<<"\n";
        auto it = std::begin(nonBrokenSatSet);
        std::advance(it,n);
        int satId = *it;
        satellites.at(satId).closeBackLink();
        satellites.at(satId).closeFrontLink();
        satellites.at(satId).closeLeftLink();
        satellites.at(satId).closeRightLink();
        nonBrokenSatSet.erase(it);
    }    

    //將所有模擬中設定壞掉的Link重新開啟
    void resetConstellationBreakingLinks(std::map<int, satellite> &satellites, std::map<int, std::map<int, bool>> &closeLinksTable, std::set<std::set<int>> &openLinkSet){
        for(auto &satPair:satellites){
            if(!closeLinksTable[satPair.second.getId()][satPair.second.getRightSatId()]){
                satPair.second.openRightLink();
                openLinkSet.insert(std::set<int>({satPair.second.getId(), satPair.second.getRightSatId()}));
            }
            if(!closeLinksTable[satPair.second.getId()][satPair.second.getLeftSatId()]){
                satPair.second.openLeftLink();
                openLinkSet.insert(std::set<int>({satPair.second.getId(), satPair.second.getLeftSatId()}));
            }
            if(!closeLinksTable[satPair.second.getId()][satPair.second.getFrontSatId()]){
                satPair.second.openFrontLink();
                openLinkSet.insert(std::set<int>({satPair.second.getId(), satPair.second.getFrontSatId()}));
            }
            if(!closeLinksTable[satPair.second.getId()][satPair.second.getBackSatId()]){
                satPair.second.openBackLink();
                openLinkSet.insert(std::set<int>({satPair.second.getId(), satPair.second.getBackSatId()}));
            }            
        }
    }

    //將所有模擬中設定壞掉的Link重新開啟
    void resetConstellationBreakingLinks(std::map<int, satellite> &satellites, std::map<int, std::map<int, bool>> &closeLinksTable){
        for(auto &satPair:satellites){
            if(!closeLinksTable[satPair.second.getId()][satPair.second.getRightSatId()]){
                satPair.second.openRightLink();
            }
            if(!closeLinksTable[satPair.second.getId()][satPair.second.getLeftSatId()]){
                satPair.second.openLeftLink();
            }
            if(!closeLinksTable[satPair.second.getId()][satPair.second.getFrontSatId()]){
                satPair.second.openFrontLink();
            }
            if(!closeLinksTable[satPair.second.getId()][satPair.second.getBackSatId()]){
                satPair.second.openBackLink();
            }            
        }
    }

    /*------------------Satellite class 的函式  start------------------*/
    //satellite的建構子，初始化衛星的各個資訊
    satellite::satellite(std::unordered_map<std::string, std::vector<int>> &constellationInfoTable, Tle _tle, SGP4 _sgp4, int _id, std::map<int, std::map<int, bool>> &closeLinksTable, int ISLfrontAngle, int ISLrightAngle, int ISLbackAngle, int ISLleftAngle) : tle(_tle), sgp4(_sgp4), id(_id) {
        neighbors = std::vector<std::pair<int,double>>(4);//right left front back 的 <satId, ISLangle>
        neighbors[0].second = ISLrightAngle;
        neighbors[1].second = ISLleftAngle;
        neighbors[2].second = ISLfrontAngle;
        neighbors[3].second = ISLbackAngle;
        int generalOffset = constellationInfoTable.at("generalOffset")[0];
        int specialOffset = constellationInfoTable.at("specialOffset")[0];
        int satCountPerOrbit = constellationInfoTable.at("satCountPerOrbit")[0];
        int orbitCount = constellationInfoTable.at("orbitCount")[0]; 
        const int satNum = id%100;
        const int orbitNum = (id-satNum)/100;
        // std::cout<<"orbitNum: "<<orbitNum<<", satNum: "<<satNum<<"\n";
        //right
        if(orbitNum == orbitCount){
            neighbors[0].first = 100+satNum+specialOffset;
            if(satNum+specialOffset < 1){
                neighbors[0].first+=satCountPerOrbit;
            }
            if(satNum+specialOffset > satCountPerOrbit){
                neighbors[0].first-=satCountPerOrbit;
            }
        }
        else{
            neighbors[0].first = 100*(orbitNum+1)+satNum+generalOffset;
            if(satNum+generalOffset < 1){
                neighbors[0].first+=satCountPerOrbit;
            }
            if(satNum+generalOffset > satCountPerOrbit){
                neighbors[0].first-=satCountPerOrbit;
            }            
        }
        //left
        if(orbitNum == 1){
            neighbors[1].first = 100*orbitCount+satNum-specialOffset;
            if(satNum-specialOffset < 1){
                neighbors[1].first+=satCountPerOrbit;
            }
            if(satNum-specialOffset > satCountPerOrbit){
                neighbors[1].first-=satCountPerOrbit;
            }
        }
        else{
            neighbors[1].first = 100*(orbitNum-1)+satNum-generalOffset;
            if(satNum-generalOffset < 1){
                neighbors[1].first+=satCountPerOrbit;
            }
            if(satNum-generalOffset > satCountPerOrbit){
                neighbors[1].first-=satCountPerOrbit;
            }            
        }
        //front
        neighbors[2].first = satNum == satCountPerOrbit ? 100*orbitNum+1 : 100*orbitNum+satNum+1;
        //back
        neighbors[3].first = satNum == 1 ? 100*orbitNum+satCountPerOrbit : 100*orbitNum+satNum-1;
        std::string curId = std::to_string(id);
        if(constellationInfoTable.count(curId)){
            for(size_t i = 0; i < 4; ++i){
                neighbors[i].first = constellationInfoTable[curId][i];
                // std::cout<<curId<<","<<neighbors[i].first<<"\n";
            }  
        }        
        if(closeLinksTable[this->getId()][this->getRightSatId()]){
            this->closeRightLink();
        }
        if(closeLinksTable[this->getId()][this->getLeftSatId()]){
            this->closeLeftLink();
        }
        if(closeLinksTable[this->getId()][this->getFrontSatId()]){
            this->closeFrontLink();
        }
        if(closeLinksTable[this->getId()][this->getBackSatId()]){
            this->closeBackLink();
        }
    }

    //將四個衛星物件還有ISL的指標，指到對應的衛星衛星物件上
    void satellite::buildNeighborSats(std::map<int, satellite> &satellites){
        int rightSatId = this->getRightSatId();
        int leftSatId = this->getLeftSatId();
        // std::cout<<"satId: "<<this->getId()<<", rightSatId: "<<rightSatId<<", leftSatId: "<<leftSatId<<"\n";
        this->rightSatPtr = &satellites.at(rightSatId);
        this->leftSatPtr = &satellites.at(leftSatId);
        this->frontSatPtr = &satellites.at(this->getFrontSatId());
        this->backSatPtr = &satellites.at(this->getBackSatId());
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

    //獲得右側衛星AER差異數值(用於判斷可否連線)
    AER satellite::getrightSatAERdiff(int time){
        AER aer = this->getAER(time, this->getRightSat());
        double azimuthDiff = getAngleDiff(this->getISLrightAngle(), aer.A);
        double elevationDiff = abs(aer.E);
        AER AERdiff(aer.date, azimuthDiff, elevationDiff, aer.R);
        return AERdiff;
    }

    //獲得左側衛星AER差異數值(用於判斷可否連線)
    AER satellite::getleftSatAERdiff(int time){
        AER aer = this->getAER(time, this->getLeftSat());
        double azimuthDiff = getAngleDiff(this->getISLleftAngle(), aer.A);
        double elevationDiff = abs(aer.E);
        AER AERdiff(aer.date, azimuthDiff, elevationDiff, aer.R);
        return AERdiff;
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

    void satellite::closeRightLink(){
        this->rightClosed = true;
    }

    void satellite::closeLeftLink(){
        this->leftClosed = true;
    }

    void satellite::closeFrontLink(){
        this->frontClosed = true;
    }

    void satellite::closeBackLink(){
        this->backClosed = true;
    }

    void satellite::closeLink(int otherSatId){
        if(otherSatId == this->getRightSatId()){
            this->closeRightLink();
            this->getRightSat().closeLeftLink();
        }
        else if(otherSatId == this->getLeftSatId()){
            this->closeLeftLink();
            this->getLeftSat().closeRightLink();
        }
        else if(otherSatId == this->getFrontSatId()){
            this->closeFrontLink();
            this->getFrontSat().closeBackLink();
        }
        else if(otherSatId == this->getBackSatId()){
            this->closeBackLink();
            this->getBackSat().closeFrontLink();
        }
        else{
            std::cout<<"error in satellite::closeLink!\n";
            exit(-1);
        }
    }  
    
    void satellite::openRightLink(){
        this->rightClosed = false;
    }

    void satellite::openLeftLink(){
        this->leftClosed = false;
    }

    void satellite::openFrontLink(){
        this->frontClosed = false;
    }

    void satellite::openBackLink(){
        this->backClosed = false;
    }

    bool satellite::rightLinkClosed(){
        return this->rightClosed;
    }
    
    bool satellite::leftLinkClosed(){
        return this->leftClosed;
    }

    bool satellite::frontLinkClosed(){
        return this->frontClosed;
    }

    bool satellite::backLinkClosed(){
        return this->backClosed;
    }    

    satellite& satellite::getRightSat(){
        if(!rightSatPtr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<": rightSatPtr is NULL! please call satellite::buildNeighborSats to setup the pointers."<<"\n";
            exit(-1);
        }
        return *rightSatPtr;
    }

    satellite& satellite::getLeftSat(){
        if(!leftSatPtr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<": leftSatPtr is NULL! please call satellite::buildNeighborSats to setup the pointers."<<"\n";
            exit(-1);
        }        
        return *leftSatPtr;
    }

    satellite& satellite::getFrontSat(){
        if(!frontSatPtr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<": frontSatPtr is NULL! please call satellite::buildNeighborSats to setup the pointers."<<"\n";
            exit(-1);
        }        
        return *frontSatPtr;
    }

    satellite& satellite::getBackSat(){
        if(!backSatPtr){
            std::cout<<"line "<<__LINE__<<" of "<<__FILE__<<": backSatPtr is NULL! please call satellite::buildNeighborSats to setup the pointers."<<"\n";
            exit(-1);
        }          
        return *backSatPtr;
    }

    //印出每一個相鄰衛星的編號
    void satellite::printNeighborId(){
        std::cout<<"neighbors of sat"<<id<<" ---> right: "<<neighbors[0].first<<", left: "<<neighbors[1].first<<", front: "<<neighbors[2].first<<", back: "<<neighbors[3].first;
        std::cout<<",  ISLangle of sat"<<id<<" ---> right: "<<neighbors[0].second<<", left: "<<neighbors[1].second<<", front: "<<neighbors[2].second<<", back: "<<neighbors[3].second<<"\n";
        // std::cout<<"("<<id<<"):("<<neighbors[0].first<<","<<neighbors[1].first<<","<<neighbors[2].first<<","<<neighbors[3].first<<")\n";
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
        // if(right == 0){
        //     std::cout<<"At t="<<time<<" sat"<<this->getId()<<" disconnect to sat"<<this->getRightSatId()<<" due to unacceptable Azimuth\n";
        // }
        // if(left == 0){
        //     std::cout<<"At t="<<time<<" sat"<<this->getRightSatId()<<" disconnect to sat"<<this->getId()<<" due to unacceptable Azimuth\n";
        // }
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

    //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，且有考慮PAT，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeRightISLwithPAT(int time, int PAT_time, const AER &acceptableAER_diff){
        if(this->rightLinkClosed()){
            return 0;
        }
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
        if(this->leftLinkClosed()){
            return 0;
        }
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
    /*------------------Satellite class  end------------------*/
}