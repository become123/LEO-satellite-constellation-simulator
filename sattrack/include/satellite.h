#ifndef SATELLITE_H
#define SATELLITE_H
#include <vector>
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <map>
#include <utility>
#include <SGP4.h>
#include <bitset>
#include "AER.h"


namespace satellite
{
    bool judgeAzimuth(double ISLdirAzimuth, double acceptableAzimuthDif, double otherSatAzimuth);
    bool judgeElevation(double acceptableElevationDif, double otherSatElevation);
    bool judgeRange(double acceptableRange, double otherSatRange);
    
class satellite{
public: 
    satellite(Tle _tle, SGP4 _sgp4, int _id, int ISLfrontAngle, int ISLrightAngle, int ISLbackAngle, int ISLleftAngle);
    Tle getTle();
    SGP4 getSgp4();
    int getId();
    AER getAER(int second, int otherId, std::map<int, satellite> &satellites);
    int getRightSatId();
    int getLeftSatId();
    int getFrontSatId();
    int getBackSatId();
    double getISLrightAngle();
    double getISLleftAngle();
    double getISLfrontAngle();
    double getISLbackAngle();

    //印出每一個相鄰衛星的編號
    void printNeighborId();

    //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線
    bool judgeRightConnectability(int second, std::map<int, satellite> &satellites, const AER &acceptableAER_diff);

    //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool judgeRightConnectability(int second, std::map<int, satellite> &satellites, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &rightSatAER);
    
    //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線
    bool judgeLeftConnectability(int second, std::map<int, satellite> &satellites, const AER &acceptableAER_diff);
    
    //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool judgeLeftConnectability(int second, std::map<int, satellite> &satellites, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &leftSatAER);
    
    //回傳前方鄰近軌道的衛星在特定時刻是否可以建立連線
    bool judgeFrontConnectability(int second, std::map<int, satellite> &satellites, const AER &acceptableAER_diff);
    
    //回前方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool judgeFrontConnectability(int second, std::map<int, satellite> &satellites, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &frontSatAER);
    
    //回傳後方鄰近軌道的衛星在特定時刻是否可以建立連線
    bool judgeBackConnectability(int second, std::map<int, satellite> &satellites, const AER &acceptableAER_diff);
    
    //回後方鄰近軌道的衛星在特定時刻是否可以建立連線，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
    bool judgeBackConnectability(int second, std::map<int, satellite> &satellites, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &backSatAER);
    
    private:
    Tle tle;
    SGP4 sgp4;
    int id;
    std::vector<std::pair<int, double>> neighbors;//依序是 right left  front back 的衛星編號

};









}
#endif