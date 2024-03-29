#ifndef SATELLITE_H
#define SATELLITE_H
#include <vector>
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <map>
#include <utility>
#include <set>
#include <unordered_map>
#include <SGP4.h>
#include <bitset>
#include "AER.h"
#include "GEO.h"



namespace satellite
{
    class satellite;
    double getAngleDiff(double angle1, double angle2);
    bool judgeAzimuth(double ISLdirAzimuth, double acceptableAzimuthDif, double otherSatAzimuth);
    bool judgeElevation(double acceptableElevationDif, double otherSatElevation);
    bool judgeRange(double acceptableRange, double otherSatRange);

    //將衛星編號轉成二維陣列的index
    size_t satIdToIndex(int SatId, size_t satCountPerOrbit);

    //將二維陣列的index轉成衛星編號
    int indexToSatId(size_t IndexNumber, size_t satCountPerOrbit);

    //回傳某個特定時刻，行星群的連線狀態(112*112的二維vetcor，可以連的話填上距離，不可連的話填上0，自己連自己也是填0)
    std::vector<std::vector<int>> getConstellationState(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites);
    
    //回傳某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)
    std::vector<std::vector<int>> getConstellationHopCount(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites);
    
    //判斷星群是否有任何兩個衛星無法經ISL抵達彼此
    bool judgeConstellationBreaking(const std::vector<std::vector<int>> &constellationHopCount);

    //判斷星群是否有任何兩個衛星無法經ISL抵達彼此，不包含已經在模擬中壞掉的衛星
    bool judgeConstellationBreaking(const std::vector<std::vector<int>> &constellationHopCount,const std::set<int> &nonBrokenSatSet, size_t satCountPerOrbit);

    //回傳某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)，同時記錄中間點(shortest path經過的點)，以用來計算shortest path
    std::vector<std::vector<int>> getConstellationHopCountRecordMedium(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites, std::vector<std::vector<int>> &medium);
    
    //回傳某個特定時刻，行星群的shortest path狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過多少距離才會抵達另一個衛星)
    std::vector<std::vector<int>> getConstellationShortestPath(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites);

    //回傳某個特定時刻，行星群的shortest path狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過多少距離才會抵達另一個衛星)，同時記錄中間點(shortest path經過的點)，以用來計算shortest path
    std::vector<std::vector<int>> getConstellationShortestPathRecordMedium(size_t satCountPerOrbit, size_t totalSatCount, int time, int PAT_time, const AER &acceptableAER_diff, std::map<int, satellite> &satellites, std::vector<std::vector<int>> &medium);  
    
    std::vector<int> getPath(size_t satCountPerOrbit, size_t sourceSatId, size_t destSatId, const std::vector<std::vector<int>> &medium, std::vector<std::vector<int>> shortestPath);
    
    //getPath function helper
    void find_path(size_t satCountPerOrbit, size_t source, size_t dest, std::vector<int> &path, const std::vector<std::vector<int>> &medium);
    
    //獲得紀錄還有哪些Link是正常還沒壞掉或被關掉的set
    std::set<std::set<int>> getOpenLinkSet(std::map<int, satellite> &satellites);

    //獲得紀錄還有哪些衛星是正常還沒壞掉的set(初始為所有衛星都是正常的)
    std::set<int> getNonBrokenSatSet(std::map<int, satellite> &satellites);

    //從尚可以使用的Link中，隨機選出一個Link關掉，模擬ISL壞掉的情形
    void randomCloseLink(std::map<int, satellite> &satellites, std::set<std::set<int>> &openLinkSet);

    //從尚可以使用的衛星中，隨機選出一個衛星壞掉(4個ISL都壞掉)，模擬衛星壞掉的情形
    void randomBreakSat(std::map<int, satellite> &satellites, std::set<int> &nonBrokenSatSet);

    //將所有模擬中設定壞掉的Link重新開啟
    void resetConstellationBreakingLinks(std::map<int, satellite> &satellites, std::map<int, std::map<int, bool>> &closeLinksTable, std::set<std::set<int>> &openLinkSet);          
    void resetConstellationBreakingLinks(std::map<int, satellite> &satellites, std::map<int, std::map<int, bool>> &closeLinksTable);


    class satellite{
    public: 
        satellite(std::unordered_map<std::string, std::vector<int>> &constellationInfoTable, Tle _tle, SGP4 _sgp4, int _id,std::map<int, std::map<int, bool>> &closeLinksTable, int ISLfrontAngle, int ISLrightAngle, int ISLbackAngle, int ISLleftAngle);
        void buildNeighborSats(std::map<int, satellite> &satellites);
        Tle getTle();
        SGP4 getSgp4();
        int getId();
        AER getAER(int time, satellite &other);
        AER getrightSatAERdiff(int time);
        AER getleftSatAERdiff(int time);
        GEOcoordinate getGEOcoordinate(int time);
        int getRightSatId();
        int getLeftSatId();
        int getFrontSatId();
        int getBackSatId();
        double getISLrightAngle();
        double getISLleftAngle();
        double getISLfrontAngle();
        double getISLbackAngle();
        void closeRightLink();
        void closeLeftLink();
        void closeFrontLink();
        void closeBackLink();
        void closeLink(int otherSatId);
        void openRightLink();
        void openLeftLink();
        void openFrontLink();
        void openBackLink();     
        bool rightLinkClosed();
        bool leftLinkClosed();
        bool frontLinkClosed();
        bool backLinkClosed();
        satellite& getRightSat();
        satellite& getLeftSat();
        satellite& getFrontSat();
        satellite& getBackSat();

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

        //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，且有考慮PAT，可連線則回傳距離，不可連線則回傳0
        int judgeRightISLwithPAT(int time, int PAT_time, const AER &acceptableAER_diff);

        //回傳特定時刻可否建立左方的ISL(要彼此可以連線到彼此才可以建立)，且有考慮PAT，可連線則回傳距離，不可連線則回傳0
        int judgeLeftISLwithPAT(int time, int PAT_time, const AER &acceptableAER_diff);

        //根據方位角設置衛星的ISL setting state
        void setState(size_t _t, const int &_ISLrightAngle, const int &_ISLleftAngle);

        //回傳特定時刻可否建立右方的ISL(要彼此可以連線到彼此才可以建立)，且左側右側ISL可以連P+1或P-1軌道(沒有固定)，尚未考慮PAT
        bool adjustableISLdeviceJudgeRight(int time, const AER &acceptableAER_diff);

        //回傳特定時刻可否建立左方的ISL(要彼此可以連線到彼此才可以建立)，且左側右側ISL可以連P+1或P-1軌道(沒有固定)，尚未考慮PAT
        bool adjustableISLdeviceJudgeLeft(int time, const AER &acceptableAER_diff);
        

    private:
        Tle tle;
        SGP4 sgp4;
        int id;
        satellite *rightSatPtr = nullptr , *leftSatPtr = nullptr , *frontSatPtr = nullptr , *backSatPtr = nullptr ;
        std::vector<std::pair<int, double>> neighbors;//依序是 right left  front back 的<衛星編號，ISL角度>
        bool rightClosed = false, leftClosed = false, backClosed = false, frontClosed = false;


        


    };









}
#endif