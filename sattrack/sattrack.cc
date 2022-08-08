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

//印出設定的parameter
void printParameter(std::map<std::string, int> &parameterTable){
    for(auto p:parameterTable){
        std::cout<<p.first<<": "<<p.second<<"\n";
    }
}

//印出每一個衛星的四個鄰居衛星編號
void printAllSatNeighbor(std::map<int, satellite::satellite> &satellites){
    for(auto &sat:satellites){
        sat.second.printNeighbor();
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

//印出編號observerId衛星觀察編號otherId衛星一天中的AER數值到./output.txt
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

//印出特定一顆衛星一天中對東方衛星的連線狀態(1表示可連，2表示不可連)到./output.txt
void printEastAvailableTimeFile(int satId, std::map<int, satellite::satellite> &satellites, std::map<std::string, int> &parameterTable){
    satellite::satellite sat = satellites.at(satId);
    std::ofstream output("./output.txt");
    for (int second = 0; second < 86400; ++second){
        output<<sat.judgeEastConnectability(second, satellites, parameterTable);
    }
    output.close();
}

//印出特定一顆衛星一天中對西方衛星的連線狀態(1表示可連，2表示不可連)到./output.txt
void printWestAvailableTimeFile(int satId, std::map<int, satellite::satellite> &satellites, std::map<std::string, int> &parameterTable){
    satellite::satellite sat = satellites.at(satId);
    std::ofstream output("./output.txt");
    for (int second = 0; second < 86400; ++second){
        output<<sat.judgeWestConnectability(second, satellites, parameterTable);
    }
    output.close();
}

//印出每顆衛星在一天中，分別對東西方衛星的連線狀態到./outputFile/資料夾中
void printAllSatConnectionInfoFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, int> &parameterTable){
    std::string outputDir = "./outputFile/" + std::to_string(parameterTable.at("acceptableAzimuthDif")) + "_" + std::to_string(parameterTable.at("acceptableElevationDif")) + "_" + std::to_string(parameterTable.at("acceptableRange")) + ".txt";
    std::ofstream output(outputDir);
    output << std::setprecision(5) << std::fixed;    
    for(auto &sat: satellites){
        int eastAvailableTime = 0;
        int westAvailableTime = 0;       
        output<<"sat"<<sat.first<<": ";
        for (int second = 0; second < 86400; ++second){
            if(sat.second.judgeEastConnectability(second, satellites, parameterTable)) ++eastAvailableTime;
            if(sat.second.judgeWestConnectability(second, satellites, parameterTable)) ++westAvailableTime;
        }
        output<<"eastAvailableTime: "<<eastAvailableTime<<", westAvailableTime: "<<westAvailableTime;
        double avgUtilization = (double)(172800+eastAvailableTime+westAvailableTime)/345600;//86400*2=172800(同軌道前後的衛星永遠可以連線得上), 86400*4=345600
        output<<", average Utilization: "<<avgUtilization<<"\n";                
    }
    output.close();    
}

int main()
{
    std::map<int, satellite::satellite> satellites = getFileData::getTLEdata("TLE_7P_16Sats.txt");
    std::map<std::string, int> parameterTable  = getFileData::getParameterdata("parameter.txt");
    printParameter(parameterTable);
    // printAllSatNeighbor(satellites);
    // testJudgeAzimuthFunction(200, 359.99);
    // printAERfile(101, satellites.at(101).getWestSatId(),satellites);
    // printEastAvailableTimeFile(101, satellites, parameterTable);
    // printWestAvailableTimeFile(101, satellites, parameterTable);
    printAllSatConnectionInfoFile(satellites, parameterTable);
    return 0;
}
