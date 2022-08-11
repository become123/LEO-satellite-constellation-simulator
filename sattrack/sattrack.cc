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
#include <bitset>

//for using std::string type in switch statement
constexpr unsigned int str2int(const char* str, int h = 0){
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ (unsigned int)str[h];
}

typedef void (*ScriptFunction)(); // function pointer type 
std::map<std::string, ScriptFunction> functionTable;


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
    std::ofstream output("./output.txt");
    output << std::setprecision(8) << std::fixed;
    for (int i = 0; i < 86400; ++i)
    {
        AER curAER = observer.getAER(i, otherId, satellites);
        output<<"satellite"<<observer.getId()<<" observe satellite"<<otherId<<" at date "<<curAER.date <<":    A: "<<curAER.A<<",    E: "<<curAER.E<<",    R: "<<curAER.R<<"\n";
    }
    output.close();

}

//印出編號observerId衛星一天中對飛行方向右方衛星的連線狀態到sattrack/output.txt中
void printRightConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    satellite::satellite sat = satellites.at(observerId);
    std::ofstream output("./output.txt");
    AER acceptableAER_diff("acceptableAER_diff", std::stod(parameterTable.at("acceptableAzimuthDif")), std::stod(parameterTable.at("acceptableElevationDif")), std::stod(parameterTable.at("acceptableRange")));
    for (int second = 0; second < 86400; ++second){
        AER rightSatAER;
        std::bitset<3> connectionState;
        bool result = sat.judgeRightConnectability(second, satellites, acceptableAER_diff, connectionState, rightSatAER);
        output<<rightSatAER.date<<":"<<std::setw(10)<<rightSatAER.A<<","<<std::setw(10)<<rightSatAER.E<<","<<std::setw(10)<<rightSatAER.R;
        output<<",   connectability of A: "<<connectionState[0]<<",   connectability of E: "<<connectionState[1]<<",   connectability of R: "<<connectionState[2]<<"  ---->  ";
        if(result)  
            output<<"OK\n";
        else    
            output<<"can't connect\n";
    }
    output.close();
}

//印出編號observerId衛星一天中對飛行方向左方衛星的連線狀態到sattrack/output.txt中
void printLeftConnectabilityFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    satellite::satellite sat = satellites.at(observerId);
    std::ofstream output("./output.txt");
    AER acceptableAER_diff("acceptableAER_diff", std::stod(parameterTable.at("acceptableAzimuthDif")), std::stod(parameterTable.at("acceptableElevationDif")), std::stod(parameterTable.at("acceptableRange")));
    for (int second = 0; second < 86400; ++second){
        AER leftSatAER;
        std::bitset<3> connectionState;
        bool result = sat.judgeLeftConnectability(second, satellites, acceptableAER_diff, connectionState, leftSatAER);
        output<<leftSatAER.date<<":"<<std::setw(10)<<leftSatAER.A<<","<<std::setw(10)<<leftSatAER.E<<","<<std::setw(10)<<leftSatAER.R;
        output<<",   connectability of A: "<<connectionState[0]<<",   connectability of E: "<<connectionState[1]<<",   connectability of R: "<<connectionState[2]<<"  ---->  ";
        if(result)  
            output<<"OK\n";
        else    
            output<<"can't connect\n";
    }
    output.close();
}

//印出每顆衛星在一天中，分別對飛行方向左方右方衛星的連線狀態到sattrack/outputFile/資料夾中，檔名會是acceptableAzimuthDif_acceptableElevationDif_acceptableRange.txt
void printAllSatConnectionInfoFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    std::string outputDir = "./outputFile/" + parameterTable.at("acceptableAzimuthDif") + "_" + parameterTable.at("acceptableElevationDif") + "_" + parameterTable.at("acceptableRange") + ".txt";
    std::ofstream output(outputDir);
    AER acceptableAER_diff("acceptableAER_diff", std::stod(parameterTable.at("acceptableAzimuthDif")), std::stod(parameterTable.at("acceptableElevationDif")), std::stod(parameterTable.at("acceptableRange")));
    output << std::setprecision(5) << std::fixed;    
    for(auto &sat: satellites){
        int rightAvailableTime = 0;
        int leftAvailableTime = 0;       
        output<<"sat"<<sat.first<<": ";
        for (int second = 0; second < 86400; ++second){
            if(sat.second.judgeRightConnectability(second, satellites, acceptableAER_diff)) ++rightAvailableTime;
            if(sat.second.judgeLeftConnectability(second, satellites, acceptableAER_diff)) ++leftAvailableTime;
        }
        output<<"rightAvailableTime: "<<rightAvailableTime<<", leftAvailableTime: "<<leftAvailableTime;
        double avgUtilization = (double)(172800+rightAvailableTime+leftAvailableTime)/345600;//86400*2=172800(同軌道前後的衛星永遠可以連線得上), 86400*4=345600
        output<<", average Utilization: "<<avgUtilization<<"\n";                
    }
    output.close();    
}

//印出從方位角誤差80、85、90、...、170、175，所有衛星的左方與右方ISL平均可連線總時間到sattrack/output.txt中
void printAvgAvailableTimeFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    std::ofstream output("./output.txt");
    output << std::setprecision(5) << std::fixed; 
    AER acceptableAER_diff("acceptableAER_diff", std::stod(parameterTable.at("acceptableAzimuthDif")), std::stod(parameterTable.at("acceptableElevationDif")), std::stod(parameterTable.at("acceptableRange")));  
    for(int acceptableAzimuthDif = 80; acceptableAzimuthDif <= 175; acceptableAzimuthDif+=5){
        std::vector<int> rightAvailableTimeOfAllSat;
        std::vector<int> leftAvailableTimeOfAllSat;
        parameterTable["acceptableAzimuthDif"] = std::to_string(acceptableAzimuthDif);
        for(auto &sat: satellites){
            int rightAvailableTime = 0;
            int leftAvailableTime = 0;       
            for (int second = 0; second < 86400; ++second){
                if(sat.second.judgeRightConnectability(second, satellites, acceptableAER_diff)) ++rightAvailableTime;
                if(sat.second.judgeLeftConnectability(second, satellites, acceptableAER_diff)) ++leftAvailableTime;
            }
            rightAvailableTimeOfAllSat.push_back(rightAvailableTime);
            leftAvailableTimeOfAllSat.push_back(leftAvailableTime);
        }   
        double rightAvgAvailableTime = (double)std::accumulate(rightAvailableTimeOfAllSat.begin(), rightAvailableTimeOfAllSat.end(), 0.0) / rightAvailableTimeOfAllSat.size();
        double leftAvgAvailableTime = (double)std::accumulate(leftAvailableTimeOfAllSat.begin(), leftAvailableTimeOfAllSat.end(), 0.0) / leftAvailableTimeOfAllSat.size();
        double interLinkAvgAvailableTime = (rightAvgAvailableTime+leftAvgAvailableTime)/2;
        // output<<"acceptableAzimuthDif = "<<std::setw(3)<<acceptableAzimuthDif<<" --> "<<", interplane link avg available time: "<<interLinkAvgAvailableTime<<", right link avg available time: "<<rightAvgAvailableTime<<", left link avg available time: "<<leftAvgAvailableTime<<"\n";    
        output<<std::setw(3)<<acceptableAzimuthDif<<","<<std::setw(14)<<interLinkAvgAvailableTime<<","<<std::setw(14)<<rightAvgAvailableTime<<","<<std::setw(14)<<leftAvgAvailableTime<<"\n";
    }
    output.close();    
}




int main()
{
    clock_t start, End;
    double cpu_time_used;
    start = clock();
    /*---------------------------------------*/
    std::map<std::string, std::string> parameterTable  = getFileData::getParameterdata("parameter.txt");
    std::map<int, satellite::satellite> satellites = getFileData::getSatellitesTable("TLE_7P_16Sats.txt", std::stoi(parameterTable.at("ISLfrontAngle")), std::stoi(parameterTable.at("ISLrightAngle")), std::stoi(parameterTable.at("ISLbackAngle")), std::stoi(parameterTable.at("ISLleftAngle")));

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
        case str2int("printAllSatConnectionInfoFile"):
            printAllSatConnectionInfoFile(satellites,parameterTable);
            break;
        case str2int("printAvgAvailableTimeFile"):
            printAvgAvailableTimeFile(satellites,parameterTable);
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
