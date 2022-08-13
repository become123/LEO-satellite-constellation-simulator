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

    //將衛星編號轉成二微陣列的index
    size_t satIdtoIndex(int SatId){
        const int satNum = SatId%100;
        const int orbitNum = (SatId-satNum)/100;
        return  (size_t)(orbitNum-1)*16 + (size_t)(satNum-1);
    }

    //回傳某個特定時刻，行星群的連線狀態(116*116的二維vetcor，可以連的話填上距離，不可連的話填上0，自己連自己也是填0)
    std::vector<std::vector<int>> getConstellationState(int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites){
        std::vector<std::vector<int>> constellationState(116, std::vector<int>(116, -1));
        for(size_t i = 0; i < 116; ++i){
            constellationState[i][i] = 0; //衛星自己
        }
        for(auto &sat:satellites){
            size_t satIndex = satIdtoIndex(sat.second.getId());
            size_t rightSatIndex = satIdtoIndex(sat.second.getRightSatId());
            size_t leftSatIndex = satIdtoIndex(sat.second.getLeftSatId());
            size_t frontSatIndex = satIdtoIndex(sat.second.getFrontSatId());
            size_t backSatIndex = satIdtoIndex(sat.second.getBackSatId());
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

    //satellite的建構子，初始化衛星的各個資訊
    satellite::satellite(Tle _tle, SGP4 _sgp4, int _id, int ISLfrontAngle, int ISLrightAngle, int ISLbackAngle, int ISLleftAngle) : tle(_tle), sgp4(_sgp4), id(_id) {
        neighbors = std::vector<std::pair<int,double>>(4);//right left front back
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

    //將四個衛星物件的指標，指到對應的衛星衛星物件上
    void satellite::buildNeighborSats(std::map<int, satellite> &satellites){
        this->rightSat = &satellites.at(this->getRightSatId());
        this->leftSat = &satellites.at(this->getLeftSatId());
        this->frontSat = &satellites.at(this->getFrontSatId());
        this->backSat = &satellites.at(this->getBackSatId());
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
    AER satellite::getAER(int time, satellite other){
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

    satellite satellite::getRightSat(){
        if(!rightSat){
            std::cout<<"rightSat is NULL! please call satellite::buildNeighborSats to setup the pointers."<<"\n";
            exit(-1);
        }
        return *rightSat;
    }

    satellite satellite::getLeftSat(){
        if(!leftSat){
            std::cout<<"leftSat is NULL! please call satellite::buildNeighborSats to setup the pointers."<<"\n";
            exit(-1);
        }        
        return *leftSat;
    }

    satellite satellite::getFrontSat(){
        if(!frontSat){
            std::cout<<"frontSat is NULL! please call satellite::buildNeighborSats to setup the pointers."<<"\n";
            exit(-1);
        }        
        return *frontSat;
    }

    satellite satellite::getBackSat(){
        if(!backSat){
            std::cout<<"backSat is NULL! please call satellite::buildNeighborSats to setup the pointers."<<"\n";
            exit(-1);
        }          
        return *backSat;
    }

    //印出每一個相鄰衛星的編號
    void satellite::printNeighborId(){
        std::cout<<"neighbors of sat"<<id<<" ---> right: "<<neighbors[0].first<<", left: "<<neighbors[1].first<<", front: "<<neighbors[2].first<<", back: "<<neighbors[3].first;
        std::cout<<",  ISLangle of sat"<<id<<" ---> right: "<<neighbors[0].second<<", left: "<<neighbors[1].second<<", front: "<<neighbors[2].second<<", back: "<<neighbors[3].second<<"\n";
    }

    //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeRightConnectability(int time, const AER &acceptableAER_diff){
        AER rightSatAER = this->getAER(time, this->getRightSat());
        //if可以連到右方衛星
        if(judgeAzimuth(this->getISLrightAngle(), acceptableAER_diff.A, rightSatAER.A) && judgeElevation(acceptableAER_diff.E, rightSatAER.E) && judgeRange(acceptableAER_diff.R, rightSatAER.R)){
            return (int)rightSatAER.R;
        }
        return 0;
    }  
 
    //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeRightConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &rightSatAER){
        rightSatAER = this->getAER(time, this->getRightSat());
        connectionState[0] = judgeAzimuth(this->getISLrightAngle(), acceptableAER_diff.A, rightSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, rightSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, rightSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeLeftConnectability(int time, const AER &acceptableAER_diff){
        AER leftSatAER = this->getAER(time, this->getLeftSat());
        if(judgeAzimuth(this->getISLleftAngle(), acceptableAER_diff.A, leftSatAER.A) && judgeElevation(acceptableAER_diff.E, leftSatAER.E) && judgeRange(acceptableAER_diff.R, leftSatAER.R)){
            return leftSatAER.R;
        }
        return 0;
    } 

    //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeLeftConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &leftSatAER){
        leftSatAER = this->getAER(time, this->getLeftSat());
        connectionState[0] = judgeAzimuth(this->getISLleftAngle(), acceptableAER_diff.A, leftSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, leftSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, leftSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳前方鄰近軌道的衛星在特定時刻是否可以建立連線，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeFrontConnectability(int time, const AER &acceptableAER_diff){
        AER frontSatAER = this->getAER(time, this->getFrontSat());
        if(judgeAzimuth(this->getISLfrontAngle(), acceptableAER_diff.A, frontSatAER.A) && judgeElevation(acceptableAER_diff.E, frontSatAER.E) && judgeRange(acceptableAER_diff.R, frontSatAER.R)){
            return frontSatAER.R;
        }
        return 0;
    } 

    //回前方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeFrontConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &frontSatAER){
        frontSatAER = this->getAER(time, this->getFrontSat());
        connectionState[0] = judgeAzimuth(this->getISLfrontAngle(), acceptableAER_diff.A, frontSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, frontSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, frontSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳後方鄰近軌道的衛星在特定時刻是否可以建立連線，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeBackConnectability(int time, const AER &acceptableAER_diff){
        AER backSatAER = this->getAER(time, this->getBackSat());
        if(judgeAzimuth(this->getISLbackAngle(), acceptableAER_diff.A, backSatAER.A) && judgeElevation(acceptableAER_diff.E, backSatAER.E) && judgeRange(acceptableAER_diff.R, backSatAER.R)){
            return backSatAER.R;
        }
        return 0;
    } 

    //回後方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeBackConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &backSatAER){
        backSatAER = this->getAER(time, this->getBackSat());
        connectionState[0] = judgeAzimuth(this->getISLbackAngle(), acceptableAER_diff.A, backSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, backSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, backSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeRightISL(int time, const AER &acceptableAER_diff){
        int right = this->judgeRightConnectability(time, acceptableAER_diff);
        int left = this->getRightSat().judgeLeftConnectability(time, acceptableAER_diff);
        if(left && right){
            return left;
        }
        return 0;
    } 

    //回傳特定時刻可否建立左方的ISL(要彼此可以連線到彼此才可以建立)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeLeftISL(int time, const AER &acceptableAER_diff){
        int right = this->judgeLeftConnectability(time, acceptableAER_diff);
        int left = this->getLeftSat().judgeRightConnectability(time, acceptableAER_diff);
        if(left && right){
            return left;
        }
        return 0;
    } 

    //回傳考慮PAT time一天(86400秒)中衛星右方ISL的可連性
    std::bitset<86400> satellite::getRightISLstates(int PATtime, const AER &acceptableAER_diff){
        std::bitset<86400> ISLstates;
        int duration = 0;
        for(size_t time = 0; time < 86400; ++time){
            if(this->judgeRightISL(time, acceptableAER_diff)){
                if(duration == PATtime){
                    ISLstates[time] = true;
                }
                else{
                    ++duration;
                }
            }
            else{
                duration = 0;
            }
        }
        return ISLstates;
    }

    //回傳考慮PAT time一天(86400秒)中衛星右方ISL的可連性
    std::bitset<86400> satellite::getLeftISLstates(int PATtime, const AER &acceptableAER_diff){
        std::bitset<86400> ISLstates;
        int duration = 0;
        for(size_t time = 0; time < 86400; ++time){
            if(this->judgeLeftISL(time, acceptableAER_diff)){
                if(duration == PATtime){
                    ISLstates[time] = true;
                }
                else{
                    ++duration;
                }
            }
            else{
                duration = 0;
            }
        }
        return ISLstates;
    }

    //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，且有考慮PAT，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeRightISLwithPAT(int time, int PAT_time, const AER &acceptableAER_diff){
        if(time < PAT_time){
            return 0;
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
            return 0;
        }
        for(int setupTime = time-PAT_time; setupTime < time; ++setupTime){
            if(this->judgeLeftISL(setupTime, acceptableAER_diff) == 0){
                return 0;
            }
        }
        return this->judgeLeftISL(time, acceptableAER_diff);
    }

}