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
    class satellite;
    bool judgeAzimuth(double ISLdirAzimuth, double acceptableAzimuthDif, double otherSatAzimuth);
    bool judgeElevation(double acceptableElevationDif, double otherSatElevation);
    bool judgeRange(double acceptableRange, double otherSatRange);
    //回傳某個特定時刻，行星群的連線狀態(112*112的二維vetcor，可以連的話填上距離，不可連的話填上0，自己連自己也是填0)
    std::vector<std::vector<int>> getConstellationState(int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites);

    
    class satellite{
    public: 
        satellite(Tle _tle, SGP4 _sgp4, int _id, int ISLfrontAngle, int ISLrightAngle, int ISLbackAngle, int ISLleftAngle);
        void buildNeighborSats(std::map<int, satellite> &satellites);
        Tle getTle();
        SGP4 getSgp4();
        int getId();
        AER getAER(int time, satellite other);
        int getRightSatId();
        int getLeftSatId();
        int getFrontSatId();
        int getBackSatId();
        double getISLrightAngle();
        double getISLleftAngle();
        double getISLfrontAngle();
        double getISLbackAngle();
        satellite& getRightSat();
        satellite& getLeftSat();
        satellite& getFrontSat();
        satellite& getBackSat();
        bool rightAlreadyCalculate();
        bool leftAlreadyCalculate();
        void setRightStateOfDate(std::bitset<86400> stateOfDay);
        void setLeftStateOfDate(std::bitset<86400> stateOfDay);


        //印出每一個相鄰衛星的編號
        void printNeighborId();

        //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，可連線則回傳距離，不可連線則回傳0
        int judgeRightConnectability(int time, const AER &acceptableAER_diff);

        //回傳右方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
        bool judgeRightConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &rightSatAER);
        
        //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，可連線則回傳距離，不可連線則回傳0
        int judgeLeftConnectability(int time, const AER &acceptableAER_diff);
        
        //回傳左方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
        bool judgeLeftConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &leftSatAER);
        
        //回傳前方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，可連線則回傳距離，不可連線則回傳0
        int judgeFrontConnectability(int time, const AER &acceptableAER_diff);
        
        //回前方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
        bool judgeFrontConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &frontSatAER);
        
        //回傳後方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，可連線則回傳距離，不可連線則回傳0
        int judgeBackConnectability(int time, const AER &acceptableAER_diff);
        
        //回後方鄰近軌道的衛星在特定時刻是否可以建立連線(單向)，同時獲得AER及對AER的三個判斷結果(std::bitset<3> connectionState由右而左三個bit分別代表A(connectionState[2])、E(connectionState[1)、R(connectionState[0])是否符合連線標準)
        bool judgeBackConnectability(int time, const AER &acceptableAER_diff, std::bitset<3> &connectionState, AER &backSatAER);

        //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，可連線則回傳距離，不可連線則回傳0
        int judgeRightISL(int time, const AER &acceptableAER_diff);

        //回傳特定時刻可否建立左方的ISL(要彼此可以連線到彼此才可以建立)，可連線則回傳距離，不可連線則回傳0
        int judgeLeftISL(int time, const AER &acceptableAER_diff);

        //回傳考慮PAT time一天(86400秒)中衛星右方ISL的可連性
        std::bitset<86400> getRightISLstateOfDay(int PATtime, const AER &acceptableAER_diff);

        //回傳考慮PAT time一天(86400秒)中衛星右方ISL的可連性
        std::bitset<86400> getLeftISLstateOfDay(int PATtime, const AER &acceptableAER_diff);

        //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，且有考慮PAT，可連線則回傳距離，不可連線則回傳0
        int judgeRightISLwithPAT(int time, int PAT_time, const AER &acceptableAER_diff);

        //回傳特定時刻可否建立左方的ISL(要彼此可以連線到彼此才可以建立)，且有考慮PAT，可連線則回傳距離，不可連線則回傳0
        int judgeLeftISLwithPAT(int time, int PAT_time, const AER &acceptableAER_diff);

        
    private:
        Tle tle;
        SGP4 sgp4;
        int id;
        satellite *rightSat = 0, *leftSat = 0, *frontSat = 0, *backSat = 0;
        std::vector<std::pair<int, double>> neighbors;//依序是 right left  front back 的<衛星編號，ISL角度>
        std::pair<bool,  std::bitset<86400>> rightStateOfDay,leftStateOfDay;//pair中的first是是否計算過，second是一天中86400秒的連線狀態
    };









}
#endif