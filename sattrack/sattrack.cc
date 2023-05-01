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
#include "mainFunction.h"
#include "groundStation.h"
#include "util.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <fstream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <bitset>
#include <set>

//for using std::string type in switch statement
constexpr unsigned int str2int(const char* str, int h = 0){
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ (unsigned int)str[h];
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
    std::string TLE_inputFileName = parameterTable.at("TLE_inputFileName");
    long unsigned int totalSatCount = 0, satCountPerOrbit = 0;
    if(TLE_inputFileName == "TLE_7P_16Sats.txt"){
        totalSatCount = 112;
        satCountPerOrbit = 16;
    }
    else if(TLE_inputFileName == "TLE_6P_22Sats.txt"){
        totalSatCount = 132;
        satCountPerOrbit = 22;
    }
    else if(TLE_inputFileName == "TLE_6P_44Sats.txt"){
        totalSatCount = 264;
        satCountPerOrbit = 44;
    }
    else if(TLE_inputFileName == "TLE_8P_33Sats.txt"){
        totalSatCount = 264;
        satCountPerOrbit = 33;
    }
    else if(TLE_inputFileName == "TLE_12P_16Sats.txt"){
        totalSatCount = 264;
        satCountPerOrbit = 16;
    }
    std::map<int, std::map<int, bool>> closeLinksTable = getFileData::getCloseLinkTable(parameterTable.at("closeLinksFileName"));
    std::map<int, satellite::satellite> satellites = getFileData::getSatellitesTable(TLE_inputFileName, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
    std::set<std::set<int>> openLinkSet = satellite::getOpenLinkSet(satellites);

    //讓衛星物件知道自己的鄰居是誰(指標指到鄰居衛星)
    for(auto &sat:satellites){
        sat.second.buildNeighborSats(satellites);
    }
    mainFunction::printParameter(parameterTable);
    std::cout<<"running function "<<parameterTable["execute_function"]<<"...\n";

    switch (str2int(parameterTable["execute_function"].c_str()))
    {
        case str2int("printAllSatNeighborId"):
            mainFunction::printAllSatNeighborId(satellites);
            break;
        case str2int("printAERfile"):
            mainFunction::printAERfile(std::stoi(parameterTable.at("observerId")), std::stoi(parameterTable.at("otherId")),satellites, "./" + parameterTable.at("outputFileName"));
            break;
        case str2int("printRightConnectabilityFile"):
            mainFunction::printRightConnectabilityFile(std::stoi(parameterTable.at("observerId")),satellites,parameterTable);
            break;
        case str2int("printLeftConnectabilityFile"):
            mainFunction::printLeftConnectabilityFile(std::stoi(parameterTable.at("observerId")),satellites,parameterTable);
            break;
        case str2int("printAllIslConnectionInfoFile"):
            mainFunction::printAllIslConnectionInfoFile(satellites,parameterTable);
            break;                                       
        case str2int("printConstellationStateFile"):
            mainFunction::printConstellationStateFile(satCountPerOrbit, totalSatCount, satellites,parameterTable);
            break; 
        case str2int("printConstellationHopCountFile"):
            mainFunction::printConstellationHopCountFile(satCountPerOrbit, totalSatCount, satellites,parameterTable);
            break;                  
        case str2int("printConstellationHopCountFileAndOutputCertainPath"):
            mainFunction::printConstellationHopCountFileAndOutputCertainPath(satCountPerOrbit, totalSatCount, satellites,parameterTable);
            break;   
        case str2int("printConstellationDistanceFile"):
            mainFunction::printConstellationDistanceFile(satCountPerOrbit, totalSatCount, satellites,parameterTable);
            break;                  
        case str2int("printConstellationDistanceAndOutputCertainPath"):
            mainFunction::printConstellationDistanceAndOutputCertainPath(satCountPerOrbit, totalSatCount, satellites,parameterTable);
            break;
        case str2int("printStationAllSatConnectionTime"):
            mainFunction::printStationAllSatConnectionTime(satellites, parameterTable);
            break;                 
        case str2int("printConstellationISLdeviceInfo"):
            mainFunction::printConstellationISLdeviceInfo(satellites, parameterTable);
            break;  
        case str2int("printStationCoverSatsPerSecond"):
            mainFunction::printStationCoverSatsPerSecond(satellites, parameterTable);
            break;
        case str2int("printDifferentLatitudeCoverTimeOfDay"):
            mainFunction::printDifferentLatitudeCoverTimeOfDay(satellites, parameterTable);
            break;    
        case str2int("printDifferentLatitudeConnectedCountOfDay"):
            mainFunction::printDifferentLatitudeConnectedCountOfDay(satellites, parameterTable);
            break; 
        case str2int("printGroundStationConnectingInfo"):
            mainFunction::printGroundStationConnectingInfo(satellites, parameterTable);
            break; 
        case str2int("printAreaConnectingInfo"):
            mainFunction::printAreaConnectingInfo(satellites, parameterTable);
            break;       
        case str2int("printAreaAllSatConnectionTime"):
            mainFunction::printAreaAllSatConnectionTime(satellites, parameterTable);
            break;
        case str2int("printRightSatAERdiff"):
            mainFunction::printRightSatAERdiff(std::stoi(parameterTable.at("observerId")), satellites, "./" + parameterTable.at("outputFileName"));
            break;
        case str2int("printLeftSatAERdiff"):
            mainFunction::printLeftSatAERdiff(std::stoi(parameterTable.at("observerId")), satellites, "./" + parameterTable.at("outputFileName"));
            break; 
        case str2int("simulateLinkbreakingStatistics"):
            mainFunction::simulateLinkbreakingStatistics(satCountPerOrbit, totalSatCount, satellites, parameterTable, closeLinksTable);
            break; 
        case str2int("simulateSatFailStatistics"):
            mainFunction::simulateSatFailStatistics(satCountPerOrbit, totalSatCount, satellites, parameterTable, closeLinksTable);
            break;                                                                                                                             
        default:
            std::cout<<"running test!"<<"\n";
            /*-------------test-------------*/
            util::getClosedLinkFile(satellites, parameterTable);
            /*------------end test---------*/





            break;
        }




    /*-------------------------------------------*/
    End = clock();
    cpu_time_used = ((double) (End - start)) / CLOCKS_PER_SEC;
    std::cout<<"cpu_time_used: "<<cpu_time_used<<"\n";
    return 0;
}


