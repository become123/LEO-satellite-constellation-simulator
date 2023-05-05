#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include "getFileData.h"
#include "satellite.h"
#include "AER.h"
#include "mainFunction.h"
#include "groundStation.h"
#include "util.h"

#include <iostream>
#include <iomanip>
#include <map>
#include<fstream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <bitset>
#include <set>
#include <numeric>


namespace mainFunction
{
    //印出設定的parameter
    void printParameter(std::map<std::string, std::string> &parameterTable){
        std::cout<<"**********************************************\nparameters:\n----------------------------------------------\n";
        for(auto p:parameterTable){
            std::cout<<p.first<<": "<<p.second<<"\n";
        }
        std::cout<<"**********************************************\n";
    }

    //印出每一個衛星的四個鄰居衛星編號
    void printAllSatNeighborId(std::map<int, satellite::satellite> &satellites){
        for(auto &sat:satellites){
            sat.second.printNeighborId();
        }
    }

    //印出編號observerId衛星觀察編號otherId衛星一天中的AER數值到sattrack/output.txt
    void printAERfile(int observerId, int otherId, std::map<int, satellite::satellite> &satellites, std::string OutputFileName){
        satellite::satellite observer = satellites.at(observerId);
        satellite::satellite other = satellites.at(otherId);
        std::ofstream output(OutputFileName);
        output << std::setprecision(8) << std::fixed;
        for (int i = 0; i < 86400; ++i)
        {
            AER curAER = observer.getAER(i, other);
            output<<"satellite"<<observer.getId()<<" observe satellite"<<otherId<<" at date "<<curAER.date <<":    A: "<<curAER.A<<",    E: "<<curAER.E<<",    R: "<<curAER.R<<"\n";
        }
        output.close();
    }

    //印出星網的跨軌道與同軌道的衛星距離資訊到sattrack/output.txt
    void printDistanceInformation(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        satellite::satellite observer = satellites.at(101);
        satellite::satellite right = observer.getRightSat();
        satellite::satellite front = observer.getFrontSat();
        output << std::setprecision(8) << std::fixed;
        std::vector<double> rightDistance;
        std::vector<double> frontDistance;
        for (int i = 0; i < 6050; ++i)//目前的設定,每天會繞地球14.28圈,每一圈86400/14.28 = 6,050.42秒
        {
            AER rightAER = observer.getAER(i, right);
            AER frontAER = observer.getAER(i, front);
            rightDistance.push_back(rightAER.R);
            frontDistance.push_back(frontAER.R);
        }
        output<<"inter-plane distance: \n";
        output<<"max: "<<*std::max_element(rightDistance.begin(), rightDistance.end());
        output<<", min: "<<*std::min_element(rightDistance.begin(), rightDistance.end());
        output<<", average: "<<std::accumulate(rightDistance.begin(), rightDistance.end(), 0.0)/rightDistance.size()<<"\n";
        output<<"intra-plane distance: \n";
        output<<"max: "<<*std::max_element(frontDistance.begin(), frontDistance.end());
        output<<", min: "<<*std::min_element(frontDistance.begin(), frontDistance.end());
        output<<", average: "<<std::accumulate(frontDistance.begin(), frontDistance.end(), 0.0)/frontDistance.size()<<"\n";
        output.close();
    }

    //印出編號observerId衛星觀察右方衛星一天中的AER差異數值(用於判斷可否連線)到sattrack/output.txt
    void printRightSatAERdiff(int observerId, std::map<int, satellite::satellite> &satellites, std::string OutputFileName){
        satellite::satellite observer = satellites.at(observerId);
        std::ofstream output(OutputFileName);
        output << std::setprecision(8) << std::fixed;
        for (int t = 0; t < 86400; ++t){
            AER curAERdiff = observer.getrightSatAERdiff(t);
            output<<"satellite"<<observer.getId()<<" observe right satellite at date "<<curAERdiff.date <<":    A_diff: "<<curAERdiff.A<<",    E_diff: "<<curAERdiff.E<<",    R_diff: "<<curAERdiff.R<<"\n";
            // output<<curAERdiff.A<<",  "<<curAERdiff.E<<", "<<curAERdiff.R<<"\n";
        }
        output.close();
    }

    //印出編號observerId衛星觀察左方衛星一天中的AER差異數值(用於判斷可否連線)到sattrack/output.txt
    void printLeftSatAERdiff(int observerId, std::map<int, satellite::satellite> &satellites, std::string OutputFileName){
        satellite::satellite observer = satellites.at(observerId);
        std::ofstream output(OutputFileName);
        output << std::setprecision(8) << std::fixed;
        for (int t = 0; t < 86400; ++t){
            AER curAERdiff = observer.getleftSatAERdiff(t);
            output<<"satellite"<<observer.getId()<<" observe left satellite at date "<<curAERdiff.date <<":    A_diff: "<<curAERdiff.A<<",    E_diff: "<<curAERdiff.E<<",    R_diff: "<<curAERdiff.R<<"\n";
            // output<<curAERdiff.A<<",  "<<curAERdiff.E<<", "<<curAERdiff.R<<"\n";
        }
        output.close();
    }    

    //印出編號observerId衛星一天中對飛行方向右方衛星的連線狀態(單向)到sattrack/output.txt中
    void printRightConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        satellite::satellite sat = satellites.at(observerId);
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
        for (int time = 0; time < 86400; ++time){
            AER rightSatAER;
            std::bitset<3> connectionState;
            bool result = sat.judgeRightConnectability(time, acceptableAER_diff, connectionState, rightSatAER);
            output<<rightSatAER.date<<":"<<std::setw(10)<<rightSatAER.A<<","<<std::setw(10)<<rightSatAER.E<<","<<std::setw(10)<<rightSatAER.R;
            output<<",   connectability of A: "<<connectionState[0]<<",   connectability of E: "<<connectionState[1]<<",   connectability of R: "<<connectionState[2]<<"  ---->  ";
            if(result)  
                output<<"OK\n";
            else    
                output<<"can't connect\n";
        }
        output.close();
    }

    //印出編號observerId衛星一天中對飛行方向左方衛星的連線狀態(單向)到sattrack/output.txt中
    void printLeftConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        satellite::satellite sat = satellites.at(observerId);
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
        for (int time = 0; time < 86400; ++time){
            AER leftSatAER;
            std::bitset<3> connectionState;
            bool result = sat.judgeLeftConnectability(time, acceptableAER_diff, connectionState, leftSatAER);
            output<<leftSatAER.date<<":"<<std::setw(10)<<leftSatAER.A<<","<<std::setw(10)<<leftSatAER.E<<","<<std::setw(10)<<leftSatAER.R;
            output<<",   connectability of A: "<<connectionState[0]<<",   connectability of E: "<<connectionState[1]<<",   connectability of R: "<<connectionState[2]<<"  ---->  ";
            if(result)  
                output<<"OK\n";
            else    
                output<<"can't connect\n";
        }
        output.close();
    }

    //印出每顆衛星在一天中，左右ISL的一天total可建立連線秒數(雙向皆通才可建立連線)到sattrack/output.txt中
    void printAllIslConnectionInfoFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
        output << std::setprecision(5) << std::fixed;    
        for(auto &sat: satellites){
            int rightAvailableTime = 0;
            int leftAvailableTime = 0;       
            output<<"sat"<<sat.first<<": ";
            for (int time = 0; time < 86400; ++time){
                if(sat.second.judgeRightISL(time, acceptableAER_diff)) ++rightAvailableTime;
                if(sat.second.judgeLeftISL(time, acceptableAER_diff)) ++leftAvailableTime;
            }
            output<<"rightAvailableTime: "<<rightAvailableTime<<", leftAvailableTime: "<<leftAvailableTime;
            double avgUtilization = (double)(172800+rightAvailableTime+leftAvailableTime)/345600;//86400*2=172800(同軌道前後的衛星永遠可以連線得上), 86400*4=345600
            output<<", average Utilization: "<<avgUtilization<<"\n";                
        }
        output.close();    
    }

    //印出某個特定時刻，行星群的連線狀態(totalSatCount*totalSatCount)到sattrack/output.txt中
    void printConstellationStateFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
        int time = std::stoi(parameterTable.at("time"));
        int PAT_time = std::stoi(parameterTable.at("PAT_time"));
        std::vector<std::vector<int>> constellationState = satellite::getConstellationState(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
         //印出totalSatCount*totalSatCount二維陣列
        output<<"     ";
        for(size_t i = 0; i < constellationState.size(); ++i){
            output<<std::setw(5)<<satellite::indexToSatId(i, satCountPerOrbit);
        }
        output<<"\n";
        for(size_t i = 0; i < constellationState.size(); ++i){
            output<<std::setw(5)<<satellite::indexToSatId(i, satCountPerOrbit);
            for(size_t j = 0; j < constellationState.size(); ++j){
                output<<std::setw(5)<<constellationState[i][j];
            }
            output<<"\n";
        }
        output.close();
    }

    //印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過幾個ISL才會抵達另一個衛星)到sattrack/output.txt中，並且在文件最後印出hopCount的統計數據
    void printConstellationHopCountFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
        int time = std::stoi(parameterTable.at("time"));
        int PAT_time = std::stoi(parameterTable.at("PAT_time"));
        std::vector<std::vector<int>> constellationHopCount = satellite::getConstellationHopCount(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
        util::printTableFirstLine(output, totalSatCount, satCountPerOrbit);
        int lineWidth = 5*totalSatCount;
        util::printDashLine(output, lineWidth+5);
        std::vector<int> hopCounts;
        for(size_t i = 0; i < constellationHopCount.size(); ++i){
            output<<std::setw(4)<<satellite::indexToSatId(i, satCountPerOrbit)<<"|";
            for(size_t j = 0; j < constellationHopCount.size(); ++j){
                output<<std::setw(3)<<constellationHopCount[i][j]<<" |";
                hopCounts.push_back(constellationHopCount[i][j]);
            }
            output<<"\n";
            util::printDashLine(output, lineWidth+5);;
        }

        output<<"\n\nmean:"<<util::getAverage(hopCounts)<<"\n";
        output<<"max:"<<*max_element(hopCounts.begin(), hopCounts.end())<<"\n";
        output<<"min:"<<*min_element(hopCounts.begin(), hopCounts.end())<<"\n";
        output<<"-----------------------------------------------------------------------\n";

        std::map<int, int> table;
        for(int hopCount:hopCounts){
            table[(size_t)hopCount]++;
        }
        for(auto p:table){
            // output<<"hopCount = "<<p.first<<": "<<p.second<<"\n";
            output<<p.first<<","<<p.second<<"\n";
        }
        output.close(); 
    }

    //印出某個特定時刻，行星群的hop count狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過多少距離才會抵達另一個衛星)到sattrack/output.txt中，並且在terminal中印出由observerId的衛星到otherId衛星的路徑
    void printConstellationHopCountFileAndOutputCertainPath(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
        int time = std::stoi(parameterTable.at("time"));
        int PAT_time = std::stoi(parameterTable.at("PAT_time"));
        std::vector<std::vector<int>> medium(totalSatCount, std::vector<int>(totalSatCount, -1));
        std::vector<std::vector<int>> constellationHopCount = satellite::getConstellationHopCountRecordMedium(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites, medium);
        output<<"     ";
        for(size_t i = 0; i < constellationHopCount.size(); ++i){ //印出表格的第一列
            output<<std::setw(3)<<satellite::indexToSatId(i, satCountPerOrbit)<<" |";
        }
        output<<"\n-----";
        int lineWidth = 5*totalSatCount;
        for(int dash = 0; dash < lineWidth; ++dash) output<<"-"; //印出分隔線
        output<<"\n";
        for(size_t i = 0; i < constellationHopCount.size(); ++i){ //印出整個hop count array
            output<<std::setw(4)<<satellite::indexToSatId(i, satCountPerOrbit)<<"|";
            for(size_t j = 0; j < constellationHopCount.size(); ++j){
                output<<std::setw(3)<<constellationHopCount[i][j]<<" |";
            }
            output<<"\n-----";
            for(int dash = 0; dash < lineWidth; ++dash) output<<"-";
            output<<"\n";
        }

        size_t sourceId = (size_t)std::stoi(parameterTable.at("observerId"));
        size_t destId = (size_t)std::stoi(parameterTable.at("otherId"));
        std::vector<int> path = satellite::getPath(satCountPerOrbit, sourceId, destId, medium, constellationHopCount);
        std::cout<<"\n\n\n---------------------------------------------\n";
        //在terminal中印出observerId衛星到otherId衛星所經過的衛星
        std::cout<<"path from sat"<<sourceId<<" to "<<"sat"<<destId<<":\n";
        for(auto v: path)
            std::cout<<v<<" ";
        std::cout<<"\n---------------------------------------------\n";
        output.close();        
    }

    //印出某個特定時刻，行星群的shortest path狀態(totalSatCount*totalSatCount的對稱二維vetcor，內容意義為衛星最少要經過多少距離才會抵達另一個衛星)到sattrack/output.txt中
    void printConstellationDistanceFile(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
        int time = std::stoi(parameterTable.at("time"));
        int PAT_time = std::stoi(parameterTable.at("PAT_time"));
        std::vector<std::vector<int>> constellationShortestDistance = satellite::getConstellationShortestPath(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
        output<<"        ";
        for(size_t i = 0; i < constellationShortestDistance.size(); ++i){
            output<<std::setw(6)<<satellite::indexToSatId(i, satCountPerOrbit)<<" |";
        }
        output<<"\n--------";
        int lineWidth = 8*totalSatCount;
        for(int dash = 0; dash < lineWidth; ++dash) output<<"-";
        output<<"\n";
        for(size_t i = 0; i < constellationShortestDistance.size(); ++i){
            output<<std::setw(7)<<satellite::indexToSatId(i, satCountPerOrbit)<<"|";
            for(size_t j = 0; j < constellationShortestDistance.size(); ++j){
                output<<std::setw(6)<<constellationShortestDistance[i][j]<<" |";
            }
            output<<"\n--------";
            for(int dash = 0; dash < lineWidth; ++dash) output<<"-";
            output<<"\n";
        }
        output.close(); 
    }

    //印出某個特定時刻，行星群的shortest path狀態，並且在terminal中印出由observerId的衛星到otherId衛星的路徑
    void printConstellationDistanceAndOutputCertainPath(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
        double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
        double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
        AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
        int time = std::stoi(parameterTable.at("time"));
        int PAT_time = std::stoi(parameterTable.at("PAT_time"));
        std::vector<std::vector<int>> medium(totalSatCount, std::vector<int>(totalSatCount, -1));
        std::vector<std::vector<int>> constellationShortestDistance = satellite::getConstellationShortestPathRecordMedium(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites, medium);
        for(size_t i = 0; i < constellationShortestDistance.size(); ++i){ //印出整個hop count array
            for(size_t j = i; j < constellationShortestDistance.size(); ++j){
                size_t sourceId = (size_t)satellite::indexToSatId(i, satCountPerOrbit);
                size_t destId = (size_t)satellite::indexToSatId(j, satCountPerOrbit);
                output<<"sat"<<sourceId<<" to sat"<<destId<<": ";
                output<<std::setw(6)<<constellationShortestDistance[i][j]<<"  ,path: ";
                std::vector<int> path = satellite::getPath(satCountPerOrbit, sourceId, destId, medium, constellationShortestDistance);
                for(auto v: path)
                    output<<v<<" ";
                output<<"\n";
            }
        }

        size_t sourceId = (size_t)std::stoi(parameterTable.at("observerId"));
        size_t destId = (size_t)std::stoi(parameterTable.at("otherId"));
        std::vector<int> path = satellite::getPath(satCountPerOrbit, sourceId, destId, medium, constellationShortestDistance);
        std::cout<<"\n\n\n---------------------------------------------\n";
        //在terminal中印出observerId衛星到otherId衛星所經過的衛星
        std::cout<<"path from sat"<<sourceId<<" to "<<"sat"<<destId<<":\n";
        for(auto v: path)
            std::cout<<v<<" ";
        std::cout<<"\n---------------------------------------------\n";
        output.close();        
    }    

    //印出根據parameter.txt設置位置的地面站，與星群中每一個衛星一天中有那些時間是可以連線的
    void printStationAllSatConnectionTime(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        bool printSecond = parameterTable.at("printSecond") == "Y";       
        std::ofstream output("./" + parameterTable.at("outputFileName"));

        groundStation::groundStation station(std::stod(parameterTable.at("stationLatitude"))
                                            ,std::stod(parameterTable.at("stationLongitude"))
                                            ,std::stod(parameterTable.at("stationAltitude")));
        int groundStationAcceptableElevation = std::stoi(parameterTable.at("groundStationAcceptableElevation"));
        int groundStationAcceptableDistance = std::stoi(parameterTable.at("groundStationAcceptableDistance"));
        bool round = parameterTable.at("round") == "Y";
        for(auto &satPair: satellites){
            std::vector<std::pair<size_t, bool>> stateChangeInfoOfDay = station.getStateChangeInfoOfDay(satPair.second, groundStationAcceptableElevation, groundStationAcceptableDistance, round);
            output<<"sat"<<satPair.first<<" connecting time: ";
            // output<<"sat"<<satPair.first<<" connecting time: \n-----------------------------\n";
            for(size_t i = 0; i < stateChangeInfoOfDay.size(); ++i){
                if(stateChangeInfoOfDay[i].second){
                    util::printTime(stateChangeInfoOfDay[i].first,output, printSecond);
                    output<<"~";
                    // output<<stateChangeInfoOfDay[i].first<<"\n";
                }
                else{
                    util::printTime(stateChangeInfoOfDay[i].first,output, printSecond);
                    output<<"-"<<(int)((float)stateChangeInfoOfDay[i].first/6050.42)+1; //目前的設定,每天會繞地球14.28圈,每一圈86400/14.28 = 6,050.42秒
                    output<<", ";
                    // output<<stateChangeInfoOfDay[i].first<<"\n";
                }
            }  
            output<<"\n";              
        }

        output.close();
    }

    //印出自parameter.txt中的startTime到endTime每秒的衛星間方位角關係及角度差較小的ISL裝置設置角度
    void printConstellationISLdeviceInfo(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        auto printSatAngle = [](std::map<int, satellite::satellite> &_satellites, int &_satId, size_t &_t, std::ofstream &_output) { 
            double lookRightA = _satellites.at(_satId).getAER(_t, _satellites.at(_satId).getRightSat()).A;
            double lookLeftA = _satellites.at(_satId).getAER(_t, _satellites.at(_satId).getLeftSat()).A;
            _output<<std::fixed<< std::setprecision(2);
            _output<<"("<<std::setw(6)<<lookLeftA;    
            _output<<"["<<_satId<<"]"; 
            _output<<std::setw(6)<<lookRightA<<")-";          
        };       

        auto printSatState = [](int _ISLrightAngle, int _ISLleftAngle, std::map<int, satellite::satellite> &_satellites, int &_satId, size_t &_t, std::ofstream &_output) { 
            double lookRightA = _satellites.at(_satId).getAER(_t, _satellites.at(_satId).getRightSat()).A;
            double lookLeftA = _satellites.at(_satId).getAER(_t, _satellites.at(_satId).getLeftSat()).A;
            _output<<"(  "<< std::setw(4) ;   
            satellite::getAngleDiff(lookLeftA, _ISLrightAngle) < satellite::getAngleDiff(lookLeftA, _ISLleftAngle) ? _output << _ISLrightAngle : _output << _ISLleftAngle;
            _output<<"["<<_satId<<"]"<< std::setw(4) ;       
            satellite::getAngleDiff(lookRightA, _ISLrightAngle) < satellite::getAngleDiff(lookRightA, _ISLleftAngle) ? _output << _ISLrightAngle : _output << _ISLleftAngle;
            _output<<"  )-";   
        };
        
        int ISLrightAngle = std::stoi(parameterTable.at("ISLrightAngle"));
        int ISLleftAngle = std::stoi(parameterTable.at("ISLleftAngle")); 
        size_t startTime = (size_t)std::stoi(parameterTable.at("startTime"));
        size_t endTime = (size_t)std::stoi(parameterTable.at("endTime"));

        if(parameterTable.at("TLE_inputFileName") == "TLE_7P_16Sats.txt"){
            for(size_t t = startTime; t <= endTime; ++t){
                output<<"t = "<<t<<":\n";
                //print angle
                int satId = 101;
                printSatAngle(satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 101){
                    printSatAngle(satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                }
                output<<" | ";
                satId  = 102;
                printSatAngle(satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 102){
                    printSatAngle(satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                }
                output<<" | ";
                satId  = 103;
                printSatAngle(satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 103){
                    printSatAngle(satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                }
                output<<" | ";
                satId  = 104;
                printSatAngle(satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 104){
                    printSatAngle(satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                }    
                output<<"\n";
                //print smaller diff device
                satId = 101;
                printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 101){
                    printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                }
                output<<" | ";
                satId  = 102;
                printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 102){
                    printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                }
                output<<" | ";
                satId  = 103;
                printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 103){
                    printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                }
                output<<" | ";
                satId  = 104;
                printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 104){
                    printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                }    
                output<<"\n\n";
            }
        }
        else if(parameterTable.at("TLE_inputFileName") == "TLE_6P_22Sats.txt"){
            for(size_t t = startTime; t <= endTime; ++t){
                output<<"t = "<<t<<":\n";
                //print angle
                int satId = 101;
                printSatAngle(satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 101){
                    printSatAngle(satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                } 
                output<<"\n";
                //print smaller diff device
                satId = 101;
                printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                satId = satellites.at(satId).getRightSatId();
                while(satId != 101){
                    printSatState(ISLrightAngle, ISLleftAngle, satellites, satId, t, output);
                    satId = satellites.at(satId).getRightSatId();
                }
                output<<"\n\n"; 
            }   
        }

        output.close();
    }

    //印出根據parameter.txt設置位置的地面站，一天中的每一秒有哪些衛星是可以連線的
    void printStationCoverSatsPerSecond(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));

        groundStation::groundStation station(std::stod(parameterTable.at("stationLatitude"))
                                            ,std::stod(parameterTable.at("stationLongitude"))
                                            ,std::stod(parameterTable.at("stationAltitude")));
        int groundStationAcceptableElevation = std::stoi(parameterTable.at("groundStationAcceptableElevation"));
        int groundStationAcceptableDistance = std::stoi(parameterTable.at("groundStationAcceptableDistance"));
        bool round = parameterTable.at("round") == "Y";
        for(size_t t = 0; t < 86400; ++t){
            std::vector<int> availableSatsList = station.getSecondCoverSatsList(satellites, t, groundStationAcceptableElevation, groundStationAcceptableDistance, round);
            output<<"t = "<<t<<":  ";
            if(availableSatsList.empty()){
                output<<"All can not connect!";
            }
            for(auto id: availableSatsList){
                output<<id<<",  ";
            }
            output<<"\n";
        }

        output.close();       
    }

    //印出不同緯度的地面站86400秒中，有幾秒是有被衛星覆蓋的
    void printDifferentLatitudeCoverTimeOfDay(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        output<<"latitude : coverTimeofDay\n";
        double stationLongitude = std::stod(parameterTable.at("stationLongitude"));
        double stationAltitude = std::stod(parameterTable.at("stationAltitude"));
        int groundStationAcceptableElevation = std::stoi(parameterTable.at("groundStationAcceptableElevation"));
        int groundStationAcceptableDistance = std::stoi(parameterTable.at("groundStationAcceptableDistance"));
        bool round = parameterTable.at("round") == "Y";
        int minLatitude = std::stoi(parameterTable.at("minLatitude"));
        int maxLatitude = std::stoi(parameterTable.at("maxLatitude"));
        for(double latitude = minLatitude; latitude <= maxLatitude; ++latitude){
            groundStation::groundStation station(latitude, stationLongitude, stationAltitude);
            std::bitset<86400> availabilityOfDay = station.getCoverTimeOfDay(satellites, groundStationAcceptableElevation, groundStationAcceptableDistance, round);
            output<<std::setw(3)<<(int)latitude<<"      :  "<<availabilityOfDay.count()<<"\n";
        }
        output.close();  
    }

    //印出不同緯度的地面站86400秒中，平均/最小/最大的可連線衛星數量是多少
    void printDifferentLatitudeConnectedCountOfDay(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        output<<"latitude : coversatCount\n";
        double stationLongitude = std::stod(parameterTable.at("stationLongitude"));
        double stationAltitude = std::stod(parameterTable.at("stationAltitude"));
        int groundStationAcceptableElevation = std::stoi(parameterTable.at("groundStationAcceptableElevation"));
        int groundStationAcceptableDistance = std::stoi(parameterTable.at("groundStationAcceptableDistance"));
        bool round = parameterTable.at("round") == "Y";
        int minLatitude = std::stoi(parameterTable.at("minLatitude"));
        int maxLatitude = std::stoi(parameterTable.at("maxLatitude"));        
        for(double latitude = minLatitude; latitude <= maxLatitude; ++latitude){
            groundStation::groundStation station(latitude, stationLongitude, stationAltitude);
            std::vector<int> coverSatCountOfDay = station.getCoverSatCountOfDay(satellites, groundStationAcceptableElevation, groundStationAcceptableDistance, round);
            output<<std::setw(3)<<(int)latitude<<"      :  "<<"min="<<*std::min_element(coverSatCountOfDay.begin(), coverSatCountOfDay.end())<<",max="<<*std::max_element(coverSatCountOfDay.begin(), coverSatCountOfDay.end())<<",avg:"<<std::setprecision(6)<<util::getAverage(coverSatCountOfDay)<<"\n";
        }
        output.close();        
    }    

    //印出地面站對各個衛星一天中對星群中各個衛星的時間總合，總連線時間最長的衛星，總連線時間最短的衛星，以及各個衛星總連線時間的平均
    void printGroundStationConnectingInfo(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){    
        std::ofstream output("./" + parameterTable.at("outputFileName"));

        groundStation::groundStation station(std::stod(parameterTable.at("stationLatitude"))
                                            ,std::stod(parameterTable.at("stationLongitude"))
                                            ,std::stod(parameterTable.at("stationAltitude")));
        int groundStationAcceptableElevation = std::stoi(parameterTable.at("groundStationAcceptableElevation"));
        int groundStationAcceptableDistance = std::stoi(parameterTable.at("groundStationAcceptableDistance"));
        bool round = parameterTable.at("round") == "Y";
        std::vector<int> connectingTimesOfAllSats;
        std::map<int, std::vector<int>> satConnectingRecord;
        for(auto &satPair: satellites){
            std::bitset<86400> connectingStatusOfDay = station.getConnectionOfDay(satPair.second, groundStationAcceptableElevation, groundStationAcceptableDistance, round);   
            int totalConnectingTime = connectingStatusOfDay.count();
            output<<"sat"<<satPair.first<<" total connecting time: "<<totalConnectingTime<<"\n";
            connectingTimesOfAllSats.push_back(totalConnectingTime);
            satConnectingRecord[totalConnectingTime].push_back(satPair.first);
        }
        std::pair<int, std::vector<int>> shortest = *satConnectingRecord.begin();
        std::pair<int, std::vector<int>> longest = *satConnectingRecord.rbegin();
        output<<"longest connecting time sat: ";
        for(auto satId:longest.second){
            output<<","<<satId;
        }
        output<<": "<<longest.first<<"\n";
        output<<"shortest connecting time sat: ";
        for(auto satId:shortest.second){
            output<<","<<satId;
        }
        output<<": "<<shortest.first<<"\n";
        output<<"mean connecting time:"<<(float)std::accumulate(connectingTimesOfAllSats.begin(), connectingTimesOfAllSats.end(),0)/connectingTimesOfAllSats.size()<<"\n";

        output.close();        
    }    

    //印出設定區域(多個地面站)內對各個衛星一天中對星群中各個衛星的時間總合，總連線時間最長的衛星，總連線時間最短的衛星，以及各個衛星總連線時間的平均，多個地面站中只要任一個可以連上，就算那一秒鐘可以連上
    void printAreaConnectingInfo(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){    
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        std::vector<groundStation::groundStation> stations = getFileData::getInputStations(parameterTable);

        int groundStationAcceptableElevation = std::stoi(parameterTable.at("groundStationAcceptableElevation"));
        int groundStationAcceptableDistance = std::stoi(parameterTable.at("groundStationAcceptableDistance"));
        bool round = parameterTable.at("round") == "Y";
        std::vector<int> connectingTimesOfAllSats; // for calculate avg connecting time
        std::map<int, std::vector<int>> satConnectingRecord;
        for(auto &satPair: satellites){

            std::vector<std::bitset<86400>> connectingStatusOfDays; 
            for(auto &station:stations){
                connectingStatusOfDays.push_back(station.getConnectionOfDay(satPair.second, groundStationAcceptableElevation, groundStationAcceptableDistance, round));
            }
            std::bitset<86400> connectingStatusOfDay = util::orAllElement(connectingStatusOfDays);
            
            int totalConnectingTime = connectingStatusOfDay.count();
            output<<"sat"<<satPair.first<<" total connecting time: "<<totalConnectingTime<<"\n";
            connectingTimesOfAllSats.push_back(totalConnectingTime);
            satConnectingRecord[totalConnectingTime].push_back(satPair.first);
        }
        std::pair<int, std::vector<int>> shortest = *satConnectingRecord.begin();
        std::pair<int, std::vector<int>> longest = *satConnectingRecord.rbegin();
        output<<"longest connecting time sat: ";
        for(auto satId:longest.second){
            output<<","<<satId;
        }
        output<<": "<<longest.first<<"\n";
        output<<"shortest connecting time sat: ";
        for(auto satId:shortest.second){
            output<<","<<satId;
        }
        output<<": "<<shortest.first<<"\n";
        output<<"mean connecting time:"<<(float)std::accumulate(connectingTimesOfAllSats.begin(), connectingTimesOfAllSats.end(),0)/connectingTimesOfAllSats.size()<<"\n";

        output.close();        
    }

    //印出根據parameter.txt設置的區域(多個地面站)，與星群中每一個衛星一天中有那些時間是可以連線的
    void printAreaAllSatConnectionTime(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        std::vector<groundStation::groundStation> stations = getFileData::getInputStations(parameterTable);


        int groundStationAcceptableElevation = std::stoi(parameterTable.at("groundStationAcceptableElevation"));
        int groundStationAcceptableDistance = std::stoi(parameterTable.at("groundStationAcceptableDistance"));
        bool round = parameterTable.at("round") == "Y";
        bool printSecond = parameterTable.at("printSecond") == "Y";
        for(auto &satPair: satellites){
            std::vector<std::bitset<86400>> connectingStatusOfDays; 
            for(auto &station:stations){
                connectingStatusOfDays.push_back(station.getConnectionOfDay(satPair.second, groundStationAcceptableElevation, groundStationAcceptableDistance, round));
            }
            std::bitset<86400> connectingStatusOfDay = util::orAllElement(connectingStatusOfDays);
            std::vector<std::pair<size_t, bool>> stateChangeInfoOfDay = util::getStateChangeInfo(connectingStatusOfDay);
            
            output<<"sat"<<satPair.first<<" connecting time: ";
            // output<<"sat"<<satPair.first<<" connecting time: \n-----------------------------\n";
            for(size_t i = 0; i < stateChangeInfoOfDay.size(); ++i){
                if(stateChangeInfoOfDay[i].second){
                    util::printTime(stateChangeInfoOfDay[i].first,output, printSecond);
                    output<<"~";
                    // output<<stateChangeInfoOfDay[i].first<<"\n";
                }
                else{
                    util::printTime(stateChangeInfoOfDay[i].first,output, printSecond);
                    output<<"-"<<(int)((float)stateChangeInfoOfDay[i].first/6050.42)+1; //目前的設定,每天會繞地球14.28圈,每一圈86400/14.28 = 6,050.42秒
                    output<<", ";
                    // output<<stateChangeInfoOfDay[i].first<<"\n";
                }
            }  
            output<<"\n";              
        }

        output.close();
    }

    //模擬計算連結失效率，根據所設置的模擬次數，模擬星群的Link要損壞多少個才會發生連結失效，並將最後的分布統計數據印到所設定的output檔案中
    void simulateLinkbreakingStatistics(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable, std::map<int, std::map<int, bool>> &closeLinksTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        output<<"breaking link total count : occur time\n";
        std::set<std::set<int>> openLinkSet = satellite::getOpenLinkSet(satellites);
        int closeLinkSimulateTime = std::stoi(parameterTable.at("closeLinkSimulateTime"));
        std::vector<int> linkBreakingStatisticTable(241,0);
        while(closeLinkSimulateTime--){
            double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
            double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
            double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
            AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
            int time = std::stoi(parameterTable.at("time"));
            int PAT_time = std::stoi(parameterTable.at("PAT_time"));
            satellite::resetConstellationBreakingLinks(satellites, closeLinksTable, openLinkSet);
            std::vector<std::vector<int>> constellationHopCount = satellite::getConstellationHopCount(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
            
            size_t breakingCnt = 0;
            while(!satellite::judgeConstellationBreaking(constellationHopCount)){
                satellite::randomCloseLink(satellites, openLinkSet);
                breakingCnt++;
                constellationHopCount = satellite::getConstellationHopCount(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
            }
            // std::cout<<"breakingCnt: "<<breakingCnt<<"\n";
            linkBreakingStatisticTable[breakingCnt]++;
        }
        for(size_t i = 0; i < linkBreakingStatisticTable.size(); ++i){
            output<<i<<":"<<linkBreakingStatisticTable[i]<<"\n";
        }
    }

    //模擬計算衛星隨機壞掉的連結失效率，根據所設置的模擬次數，模擬星群的衛星要損壞多少個才會發生連結失效，並將最後的分布統計數據印到所設定的output檔案中
    void simulateSatFailStatistics(long unsigned int satCountPerOrbit, long unsigned int totalSatCount, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable, std::map<int, std::map<int, bool>> &closeLinksTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        output<<"sat fail total count : occur time\n";
        std::set<int> nonBrokenSatSet;
        int satFailSimulateTime = std::stoi(parameterTable.at("satFailSimulateTime"));
        std::vector<int> satFailStatisticTable(satellites.size(),0);
        while(satFailSimulateTime--){
            double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
            double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
            double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
            AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
            int time = std::stoi(parameterTable.at("time"));
            int PAT_time = std::stoi(parameterTable.at("PAT_time"));
            satellite::resetConstellationBreakingLinks(satellites, closeLinksTable);
            nonBrokenSatSet = satellite::getNonBrokenSatSet(satellites);// reset成每一個衛星都沒有壞掉
            std::vector<std::vector<int>> constellationHopCount = satellite::getConstellationHopCount(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
            
            size_t breakingCnt = 0;
            while(!satellite::judgeConstellationBreaking(constellationHopCount,nonBrokenSatSet,satCountPerOrbit)){
                satellite::randomBreakSat(satellites, nonBrokenSatSet);
                breakingCnt++;
                constellationHopCount = satellite::getConstellationHopCount(satCountPerOrbit, totalSatCount, time, PAT_time, acceptableAER_diff, satellites);
            }
            // std::cout<<"breakingCnt: "<<breakingCnt<<"\n";
            satFailStatisticTable[breakingCnt]++;
        }
        for(size_t i = 0; i < satFailStatisticTable.size(); ++i){
            output<<i<<":"<<satFailStatisticTable[i]<<"\n";
        }
    }    
}


