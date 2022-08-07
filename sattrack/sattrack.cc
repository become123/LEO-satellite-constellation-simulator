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
#include "getTLEdata.h"
#include "rectifyAzimuth.h"
#include "satellite.h"
#include "AER.h"
#include "simParameters.h"

#include <iostream>
#include <iomanip>
#include<map>
#include<fstream>


void printAllSatNeighbor(std::map<int, satellite::satellite> &satellites){
    for(auto &sat:satellites){
        sat.second.printNeighbor();
    }
}

void testJudgeAzimuthFunction(){
    for(int i = 0; i < 180; ++i){
        std::cout<<i<<": "<<satellite::judgeAzimuth(200, i, 0.0001)<<"\n";
    }    
}

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

//印出特定一顆衛星一天中對東方衛星的連線狀態(1表示可連，2表示不可連)
void printEastAvailableTimeFile(int satId, std::map<int, satellite::satellite> &satellites){
    satellite::satellite sat = satellites.at(satId);
    std::ofstream output("./output.txt");
    for (int second = 0; second < 86400; ++second){
        output<<sat.judgeEastConnectability(second, satellites);
    }
    output.close();
}

//印出特定一顆衛星一天中對西方衛星的連線狀態(1表示可連，2表示不可連)
void printWestAvailableTimeFile(int satId, std::map<int, satellite::satellite> &satellites){
    satellite::satellite sat = satellites.at(satId);
    std::ofstream output("./output.txt");
    for (int second = 0; second < 86400; ++second){
        output<<sat.judgeWestConnectability(second, satellites);
    }
    output.close();
}

//印出一顆衛星在一天中，分別對東西方衛星可以連線的秒數總數
void printAllSatConnectionInfoFile(std::map<int, satellite::satellite> &satellites){
    std::string outputDir = "./outputFile/" + std::to_string(acceptableAzimuthDif) + "_" + std::to_string(acceptableElevationDif) + "_" + std::to_string(acceptableRange) + ".txt";
    std::ofstream output(outputDir);
    output << std::setprecision(5) << std::fixed;    
    for(auto &sat: satellites){
        int eastAvailableTime = 0;
        int westAvailableTime = 0;       
        output<<"sat"<<sat.first<<": ";
        for (int second = 0; second < 86400; ++second){
            if(sat.second.judgeEastConnectability(second, satellites)) ++eastAvailableTime;
            if(sat.second.judgeWestConnectability(second, satellites)) ++westAvailableTime;
        }
        output<<"eastAvailableTime: "<<eastAvailableTime<<", westAvailableTime: "<<westAvailableTime;
        double avgUtilization = (double)(172800+eastAvailableTime+westAvailableTime)/345600;//86400*2=172800(同軌道前後的衛星永遠可以連線得上), 86400*4=345600
        output<<", average Utilization: "<<avgUtilization<<"\n";                
    }
    output.close();    
}

int main()
{
    std::map<int, satellite::satellite> satellites = getTLEdata::getTLEdata("TLE_7P_16Sats.txt");
    printAllSatConnectionInfoFile(satellites);
    // printAERfile(101, satellites.at(101).getWestSatId(),satellites);
    return 0;
}
