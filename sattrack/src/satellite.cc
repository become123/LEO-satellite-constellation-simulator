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
    AER satellite::getAER(int second, satellite other){
        DateTime dt = this->getTle().Epoch().AddSeconds(second);
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
        return *rightSat;
    }

    satellite satellite::getLeftSat(){
        return *leftSat;
    }

    satellite satellite::getFrontSat(){
        return *frontSat;
    }

    satellite satellite::getBackSat(){
        return *backSat;
    }

    //印出每一個相鄰衛星的編號
    void satellite::printNeighborId(){
        std::cout<<"neighbors of sat"<<id<<" ---> right: "<<neighbors[0].first<<", left: "<<neighbors[1].first<<", front: "<<neighbors[2].first<<", back: "<<neighbors[3].first;
        std::cout<<",  ISLangle of sat"<<id<<" ---> right: "<<neighbors[0].second<<", left: "<<neighbors[1].second<<", front: "<<neighbors[2].second<<", back: "<<neighbors[3].second<<"\n";
    }

    //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeRightConnectability(int second, const AER &acceptableAER_diff){
        AER rightSatAER = this->getAER(second, this->getRightSat());
        //if可以連到右方衛星
        if(judgeAzimuth(this->getISLrightAngle(), acceptableAER_diff.A, rightSatAER.A) && judgeElevation(acceptableAER_diff.E, rightSatAER.E) && judgeRange(acceptableAER_diff.R, rightSatAER.R)){
            return (int)rightSatAER.R;
        }
        return 0;
    }  
 
    //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeRightConnectability(int second, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &rightSatAER){
        rightSatAER = this->getAER(second, this->getRightSat());
        connectionState[0] = judgeAzimuth(this->getISLrightAngle(), acceptableAER_diff.A, rightSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, rightSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, rightSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeLeftConnectability(int second, const AER &acceptableAER_diff){
        AER leftSatAER = this->getAER(second, this->getLeftSat());
        if(judgeAzimuth(this->getISLleftAngle(), acceptableAER_diff.A, leftSatAER.A) && judgeElevation(acceptableAER_diff.E, leftSatAER.E) && judgeRange(acceptableAER_diff.R, leftSatAER.R)){
            return leftSatAER.R;
        }
        return 0;
    } 

    //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeLeftConnectability(int second, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &leftSatAER){
        leftSatAER = this->getAER(second, this->getLeftSat());
        connectionState[0] = judgeAzimuth(this->getISLleftAngle(), acceptableAER_diff.A, leftSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, leftSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, leftSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳前方鄰近軌道的衛星在特定時刻是否可以建立連線，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeFrontConnectability(int second, const AER &acceptableAER_diff){
        AER frontSatAER = this->getAER(second, this->getFrontSat());
        if(judgeAzimuth(this->getISLfrontAngle(), acceptableAER_diff.A, frontSatAER.A) && judgeElevation(acceptableAER_diff.E, frontSatAER.E) && judgeRange(acceptableAER_diff.R, frontSatAER.R)){
            return frontSatAER.R;
        }
        return 0;
    } 

    //回前方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeFrontConnectability(int second, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &frontSatAER){
        frontSatAER = this->getAER(second, this->getFrontSat());
        connectionState[0] = judgeAzimuth(this->getISLfrontAngle(), acceptableAER_diff.A, frontSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, frontSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, frontSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳後方鄰近軌道的衛星在特定時刻是否可以建立連線，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeBackConnectability(int second, const AER &acceptableAER_diff){
        AER backSatAER = this->getAER(second, this->getBackSat());
        if(judgeAzimuth(this->getISLbackAngle(), acceptableAER_diff.A, backSatAER.A) && judgeElevation(acceptableAER_diff.E, backSatAER.E) && judgeRange(acceptableAER_diff.R, backSatAER.R)){
            return backSatAER.R;
        }
        return 0;
    } 

    //回後方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool satellite::judgeBackConnectability(int second, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &backSatAER){
        backSatAER = this->getAER(second, this->getBackSat());
        connectionState[0] = judgeAzimuth(this->getISLbackAngle(), acceptableAER_diff.A, backSatAER.A);
        connectionState[1] = judgeElevation(acceptableAER_diff.E, backSatAER.E);
        connectionState[2] = judgeRange(acceptableAER_diff.R, backSatAER.R);
        return  connectionState[0] && connectionState[1] && connectionState[2];
    }

    //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeRightISL(int second, const AER &acceptableAER_diff){
        int right = this->judgeRightConnectability(second, acceptableAER_diff);
        int left = this->getRightSat().judgeLeftConnectability(second, acceptableAER_diff);
        if(left && right){
            return left;
        }
        return 0;
    } 

    //回傳特定時刻可否建立左方的ISL(要彼此可以連線到彼此才可以建立)，可連線則回傳距離，不可連線則回傳0
    int satellite::judgeLeftISL(int second, const AER &acceptableAER_diff){
        int right = this->judgeLeftConnectability(second, acceptableAER_diff);
        int left = this->getLeftSat().judgeRightConnectability(second, acceptableAER_diff);
        if(left && right){
            return left;
        }
        return 0;
    } 

    //回傳考慮PAT time一天(86400秒)中衛星右方ISL的可連性
    std::bitset<86400> satellite::getRightISLstates(int PATtime, const AER &acceptableAER_diff){
        std::bitset<86400> ISLstates;
        int duration = 0;
        for(size_t second = 0; second < 86400; ++second){
            if(this->judgeRightISL(second, acceptableAER_diff)){
                if(duration == PATtime){
                    ISLstates[second] = true;
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
        for(size_t second = 0; second < 86400; ++second){
            if(this->judgeLeftISL(second, acceptableAER_diff)){
                if(duration == PATtime){
                    ISLstates[second] = true;
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


}