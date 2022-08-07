#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include <string>
#include <iostream>
#include "rectifyAzimuth.h"
#include "satellite.h"
#include "AER.h"

namespace satellite
{
    satellite::satellite(Tle _tle, SGP4 _sgp4, int _id) : tle(_tle), sgp4(_sgp4), id(_id) {
        neighbors = std::vector<int>(4);//east west front back
        const int satNum = id%100;
        const int orbitNum = (id-satNum)/100;
        // std::cout<<"orbitNum: "<<orbitNum<<", satNum: "<<satNum<<"\n";
        //east
        neighbors[0] = satNum - 2 < 1 ? 100*(orbitNum+1)+satNum-2+16 : 100*(orbitNum+1)+satNum-2;
        if(orbitNum == 7){
            neighbors[0] = 100+satNum;
        }
        //west
        neighbors[1] = satNum + 2 > 16 ? 100*(orbitNum-1)+satNum+2-16 : 100*(orbitNum-1)+satNum+2;
        if(orbitNum == 1){
            neighbors[1] = 700+satNum;
        }
        //front
        neighbors[2] = satNum == 16 ? 100*orbitNum+1 : 100*orbitNum+satNum+1;
        //back
        neighbors[3] = satNum == 1 ? 100*orbitNum+16 : 100*orbitNum+satNum-1;
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

    int satellite::getEastSat(){
        return neighbors[0];
    }

    int satellite::getWestSat(){
        return neighbors[1];
    }

    int satellite::getFrontSat(){
        return neighbors[2];
    }

    int satellite::getBackSat(){
        return neighbors[3];
    }

    void satellite::printNeighbor(){
        std::cout<<"neighbors of sat"<<id<<"->east: "<<neighbors[0]<<", west: "<<neighbors[1]<<", front: "<<neighbors[2]<<", back: "<<neighbors[3]<<"\n";
    }








}