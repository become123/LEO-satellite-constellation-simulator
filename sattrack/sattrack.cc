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

//印出編號observerId衛星觀察編號otherId衛星一天中的AER數值到./sattrack/output.txt
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

//印出特定observerId的衛星一天中對飛行方向右方衛星的連線狀態(1表示可連，0表示不可連)到./output.txt
void printRightAvailableTimeFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    satellite::satellite sat = satellites.at(observerId);
    std::ofstream output("./output.txt");
    for (int second = 0; second < 86400; ++second){
        output<<sat.judgeRightConnectability(second, satellites, parameterTable);
    }
    output.close();
}

//印出特定一顆衛星一天中對飛行方向左方衛星的連線狀態(1表示可連，0表示不可連)到./output.txt
void printLeftAvailableTimeFile(int observerId, std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    satellite::satellite sat = satellites.at(observerId);
    std::ofstream output("./output.txt");
    for (int second = 0; second < 86400; ++second){
        output<<sat.judgeLeftConnectability(second, satellites, parameterTable);
    }
    output.close();
}

//印出每顆衛星在一天中，分別對飛行方向左方右方衛星的連線狀態到./outputFile/資料夾中，檔名會是acceptableAzimuthDif_acceptableElevationDif_acceptableRange.txt
void printAllSatConnectionInfoFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
    std::string outputDir = "./outputFile/" + parameterTable.at("acceptableAzimuthDif") + "_" + parameterTable.at("acceptableElevationDif") + "_" + parameterTable.at("acceptableRange") + ".txt";
    std::ofstream output(outputDir);
    output << std::setprecision(5) << std::fixed;    
    for(auto &sat: satellites){
        int rightAvailableTime = 0;
        int leftAvailableTime = 0;       
        output<<"sat"<<sat.first<<": ";
        for (int second = 0; second < 86400; ++second){
            if(sat.second.judgeRightConnectability(second, satellites, parameterTable)) ++rightAvailableTime;
            if(sat.second.judgeLeftConnectability(second, satellites, parameterTable)) ++leftAvailableTime;
        }
        output<<"rightAvailableTime: "<<rightAvailableTime<<", leftAvailableTime: "<<leftAvailableTime;
        double avgUtilization = (double)(172800+rightAvailableTime+leftAvailableTime)/345600;//86400*2=172800(同軌道前後的衛星永遠可以連線得上), 86400*4=345600
        output<<", average Utilization: "<<avgUtilization<<"\n";                
    }
    output.close();    
}




int main()
{
    std::map<int, satellite::satellite> satellites = getFileData::getTLEdata("TLE_7P_16Sats.txt");
    std::map<std::string, std::string> parameterTable  = getFileData::getParameterdata("parameter.txt");

    printParameter(parameterTable);

    switch (str2int(parameterTable["execute_function"].c_str()))
    {
        case str2int("printAllSatNeighborId"):
            printAllSatNeighborId(satellites);
            break;
        case str2int("printAERfile"):
            printAERfile(std::stoi(parameterTable.at("observerId")), std::stoi(parameterTable.at("otherId")),satellites);
            break;
        case str2int("printRightAvailableTimeFile"):
            printRightAvailableTimeFile(std::stoi(parameterTable.at("observerId")),satellites,parameterTable);
            break;
        case str2int("printLeftAvailableTimeFile"):
            printLeftAvailableTimeFile(std::stoi(parameterTable.at("observerId")),satellites,parameterTable);
            break;
        case str2int("printAllSatConnectionInfoFile"):
            printAllSatConnectionInfoFile(satellites,parameterTable);
            break;          
        default:
            std::cout<<"unknown execute_function!"<<"\n";
            break;
    }
    return 0;
}

/*
    printParameter(parameterTable);
    printAllSatNeighborId(satellites);
    testJudgeAzimuthFunction(200, 359.99);
    printAERfile(101, satellites.at(101).getLeftSatId(),satellites);
    printRightAvailableTimeFile(101, satellites, parameterTable);
    printLeftAvailableTimeFile(101, satellites, parameterTable);
    printAllSatConnectionInfoFile(satellites, parameterTable);
*/