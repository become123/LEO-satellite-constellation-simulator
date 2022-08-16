/*
 * Copyright 2013 Daniel Warner <contact@danrw.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include "getFileData.h"
#include "rectifyAzimuth.h"
#include "satellite.h"
#include "AER.h"

#include <iostream>
#include <iomanip>
#include<map>
#include<fstream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <bitset>
#include <set>

//for using std::string type in switch statement
constexpr unsigned int str2int(const char* str, int h = 0){
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ (unsigned int)str[h];
}


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
void printAERfile(int observerId, int otherId, std::map<int, satellite::satellite> &satellites){
    satellite::satellite observer = satellites.at(observerId);
    satellite::satellite other = satellites.at(otherId);
    std::ofstream output("./output.txt");
    output << std::setprecision(8) << std::fixed;
    for (int i = 0; i < 86400; ++i)
    {
        AER curAER = observer.getAER(i, other);
        output<<"satellite"<<observer.getId()<<" observe satellite"<<otherId<<" at date "<<curAER.date <<":    A: "<<curAER.A<<",    E: "<<curAER.E<<",    R: "<<curAER.R<<"\n";
    }
    output.close();

}

//印出編號observerId衛星一天中對飛行方向右方衛星的連線狀態(單向)到sattrack/output.txt中
void printRightConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    satellite::satellite sat = satellites.at(observerId);
    std::ofstream output("./output.txt");
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
    std::ofstream output("./output.txt");
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

//印出每顆衛星在一天中，左右ISL的一天total可建立連線秒數(雙向皆通才可建立連線)到./outputFile/資料夾中，檔名會是acceptableAzimuthDif_acceptableElevationDif_acceptableRange.txt
void printAllIslConnectionInfoFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    std::string outputDir = "./outputFile/" + parameterTable.at("acceptableAzimuthDif") + "_" + parameterTable.at("acceptableElevationDif") + "_" + parameterTable.at("acceptableRange") + ".txt";
    std::ofstream output(outputDir);
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

//印出從方位角誤差80、85、90、...、170、175，所有衛星的左方與右方ISL平均可連線總時間到sattrack/output.txt中
void compareDifferentAcceptableAzimuthDif(std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable){
    std::ofstream output("./output.txt");
    output << std::setprecision(5) << std::fixed; 
    double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
    double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
    int PATtime = std::stoi(parameterTable.at("PAT_time"));
    for(int acceptableAzimuthDif = 80; acceptableAzimuthDif <= 175; acceptableAzimuthDif+=5){
        satellite::resetAllISL(ISLtable);
        AER acceptableAER_diff("acceptableAER_diff", (double)acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
        double rightAvailableTimeOfAllSat = 0;
        double leftAvailableTimeOfAllSat = 0;
        for(auto &sat: satellites){
            /*----------不考慮PAT----------*/
            // int rightAvailableTime = 0;
            // int leftAvailableTime = 0;   
            // for (int time = 0; time < 86400; ++time){
            //     if(sat.second.judgeRightISL(time, acceptableAER_diff)) ++rightAvailableTime;
            //     if(sat.second.judgeLeftISL(time, acceptableAER_diff)) ++leftAvailableTime;
            // }
            // rightAvailableTimeOfAllSat += rightAvailableTime;
            // leftAvailableTimeOfAllSat += leftAvailableTime;

            /*----------考慮PAT----------*/
            std::bitset<86400> rightISLstateOfDay = sat.second.getRightISLstateOfDay(PATtime, acceptableAER_diff);  
            std::bitset<86400> leftISLstateOfDay = sat.second.getLeftISLstateOfDay(PATtime, acceptableAER_diff);
            rightAvailableTimeOfAllSat += rightISLstateOfDay.count(); 
            leftAvailableTimeOfAllSat += leftISLstateOfDay.count();            
        }   
        double rightAvgAvailableTime = rightAvailableTimeOfAllSat / 112;
        double leftAvgAvailableTime = leftAvailableTimeOfAllSat / 112;
        double interLinkAvgAvailableTime = (rightAvgAvailableTime+leftAvgAvailableTime)/2;
        // output<<"acceptableAzimuthDif = "<<std::setw(3)<<acceptableAzimuthDif<<" --> "<<", interplane link avg available time: "<<interLinkAvgAvailableTime<<", right link avg available time: "<<rightAvgAvailableTime<<", left link avg available time: "<<leftAvgAvailableTime<<"\n";    
        output<<std::setw(3)<<acceptableAzimuthDif<<","<<std::setw(14)<<interLinkAvgAvailableTime<<","<<std::setw(14)<<rightAvgAvailableTime<<","<<std::setw(14)<<leftAvgAvailableTime<<"\n";
    }
    output.close();    
}

//印出從方PAT_time從5、10、15、...、到60，所有衛星的左方與右方ISL平均可連線總時間到sattrack/output.txt中
void compareDifferentPAT_time(std::map<int, satellite::satellite> &satellites, std::map<std::set<int>, satellite::ISL> &ISLtable, std::map<std::string, std::string> &parameterTable){
    std::ofstream output("./output.txt");
    output << std::setprecision(5) << std::fixed; 
    double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
    double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
    double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
    AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);  
    // std::map<int, std::bitset<86400>> alreadyCalculate;
    for(int PATtime = 5; PATtime <= 60; PATtime+=5){
        satellite::resetAllISL(ISLtable);
        double rightAvailableTimeOfAllSat = 0;
        double leftAvailableTimeOfAllSat = 0;
        for(auto &sat: satellites){
            std::bitset<86400> rightISLstateOfDay = sat.second.getRightISLstateOfDay(PATtime, acceptableAER_diff);  
            std::bitset<86400> leftISLstateOfDay = sat.second.getLeftISLstateOfDay(PATtime, acceptableAER_diff);                        
            rightAvailableTimeOfAllSat += rightISLstateOfDay.count(); 
            leftAvailableTimeOfAllSat += leftISLstateOfDay.count();
        }   
        double rightAvgAvailableTime = rightAvailableTimeOfAllSat / 112;
        double leftAvgAvailableTime = leftAvailableTimeOfAllSat / 112;
        double interLinkAvgAvailableTime = (rightAvgAvailableTime+leftAvgAvailableTime)/2;
        // output<<"PATtime = "<<std::setw(3)<<PATtime<<" --> "<<", interplane link avg available time: "<<interLinkAvgAvailableTime<<", right link avg available time: "<<rightAvgAvailableTime<<", left link avg available time: "<<leftAvgAvailableTime<<"\n";    
        output<<std::setw(3)<<PATtime<<","<<std::setw(14)<<interLinkAvgAvailableTime<<","<<std::setw(14)<<rightAvgAvailableTime<<","<<std::setw(14)<<leftAvgAvailableTime<<"\n";
    }
    output.close();    
}

//印出某個特定時刻，行星群的連線狀態(112*112的二維陣列，可以連的話填上距離，不可連的話填上0，自己連自己也是填0)到sattrack/output.txt中
void printConstellationStateFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    std::ofstream output("./output.txt");
    double acceptableAzimuthDif = std::stod(parameterTable.at("acceptableAzimuthDif"));
    double acceptableElevationDif = std::stod(parameterTable.at("acceptableElevationDif"));
    double acceptableRange = std::stod(parameterTable.at("acceptableRange"));
    AER acceptableAER_diff("acceptableAER_diff", acceptableAzimuthDif, acceptableElevationDif, acceptableRange);
    int time = std::stoi(parameterTable.at("time"));
    int PAT_time = std::stoi(parameterTable.at("PAT_time"));
    std::vector<std::vector<int>> constellationState = satellite::getConstellationState(time, PAT_time, acceptableAER_diff, satellites);
    //印出112*112二維陣列
    for(auto row: constellationState){
        for(auto i: row){
            output<<std::setw(5)<<i;
        }
        output<<"\n";
    }
    output.close();
}

int main()
{
    // std::cout<<sizeof(satellite::satellite)<<"\n";
    clock_t start, End;
    double cpu_time_used;
    start = clock();
    /*---------------------------------------*/
    
    std::map<std::string, std::string> parameterTable  = getFileData::getParameterdata("parameter.txt");
    int ISLfrontAngle = std::stoi(parameterTable.at("ISLfrontAngle"));
    int ISLrightAngle = std::stoi(parameterTable.at("ISLrightAngle"));
    int ISLbackAngle = std::stoi(parameterTable.at("ISLbackAngle"));
    int ISLleftAngle = std::stoi(parameterTable.at("ISLleftAngle"));
    std::map<int, satellite::satellite> satellites = getFileData::getSatellitesTable("TLE_7P_16Sats.txt", ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
    std::map<std::set<int>, satellite::ISL> ISLtable = satellite::getISLtable(satellites);
    /*----------------testing area----------------*/

    /*-------------------------------------------*/
    //讓衛星物件知道自己的鄰居及ISL是誰(指標指到鄰居衛星及ISL)
    for(auto &sat:satellites){
        sat.second.buildNeighborSatsAndISLs(satellites, ISLtable);
    }
    printParameter(parameterTable);
    std::cout<<"running function "<<parameterTable["execute_function"]<<"...\n";

    switch (str2int(parameterTable["execute_function"].c_str()))
    {
        case str2int("printAllSatNeighborId"):
            printAllSatNeighborId(satellites);
            break;
        case str2int("printAERfile"):
            printAERfile(std::stoi(parameterTable.at("observerId")), std::stoi(parameterTable.at("otherId")),satellites);
            break;
        case str2int("printRightConnectabilityFile"):
            printRightConnectabilityFile(std::stoi(parameterTable.at("observerId")),satellites,parameterTable);
            break;
        case str2int("printLeftConnectabilityFile"):
            printLeftConnectabilityFile(std::stoi(parameterTable.at("observerId")),satellites,parameterTable);
            break;
        case str2int("printAllIslConnectionInfoFile"):
            printAllIslConnectionInfoFile(satellites,parameterTable);
            break;
        case str2int("compareDifferentAcceptableAzimuthDif"):
            compareDifferentAcceptableAzimuthDif(satellites, ISLtable ,parameterTable);
            break;   
        case str2int("compareDifferentPAT_time"):
            compareDifferentPAT_time(satellites, ISLtable ,parameterTable);            
            break;                      
        case str2int("printConstellationStateFile"):
            printConstellationStateFile(satellites,parameterTable);
            break;          
        default:
            std::cout<<"unknown execute_function!"<<"\n";
            break;
    }



    /*-------------------------------------------*/
    End = clock();
    cpu_time_used = ((double) (End - start)) / CLOCKS_PER_SEC;
    std::cout<<"cpu_time_used: "<<cpu_time_used<<"\n";
    return 0;
}



void printRightISLFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    satellite::satellite sat = satellites.at(observerId);
    std::ofstream output("./output.txt");
    AER acceptableAER_diff("acceptableAER_diff", std::stod(parameterTable.at("acceptableAzimuthDif")), std::stod(parameterTable.at("acceptableElevationDif")), std::stod(parameterTable.at("acceptableRange")));
    for (int time = 0; time < 86400; ++time){
        bool result = sat.judgeRightISL(time, acceptableAER_diff);
        if(result)  
            output<<"1\n";
        else    
            output<<"0\n";
    }
    output.close();
}




//測試function satellite::judgeAzimuth，印出acceptableAngle介於0~180，連線裝置設在角度ISLdirAngle，觀測衛星位在角度otherSatAngle時，可否連線
void testJudgeAzimuthFunction(int ISLdirAngle, int otherSatAngle){
    for(int acceptableAngle = 0; acceptableAngle < 180; ++acceptableAngle){
        if(satellite::judgeAzimuth(ISLdirAngle, acceptableAngle, otherSatAngle)){
            std::cout<<"acceptableAngle: "<<acceptableAngle<<"->"<<"can connect"<<"\n";
        }
        else{
            std::cout<<"acceptableAngle: "<<acceptableAngle<<"->"<<"can not connect"<<"\n";
        }
    }    
}
