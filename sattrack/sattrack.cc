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


void printNeighbor(std::map<int, satellite::satellite> &satellites){
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
    satellite::satellite other = satellites.at(otherId);

    std::ofstream output("./output.txt");
    output << std::setprecision(8) << std::fixed;
    for (int i = 0; i < 86400; ++i)
    {
        AER curAER = observer.getAER(i, other);
        output<<"satellite"<<observer.getId()<<" observe satellite"<<other.getId()<<" at date "<<curAER.date <<":    A: "<<curAER.A<<",    E: "<<curAER.E<<",    R: "<<curAER.R<<"\n";
    }
    output.close();

}

void printEastConnectabilityFile(int satId, std::map<int, satellite::satellite> &satellites){
    satellite::satellite sat = satellites.at(satId);
    std::ofstream output("./output.txt");
    for (int i = 0; i < 86400; ++i){
        output<<sat.judgeEastConnectability(i, satellites);
    }
    output.close();
}

int main()
{
    std::map<int, satellite::satellite> satellites = getTLEdata::getTLEdata("TLE_7P_16Sats.txt");
    printEastConnectabilityFile(101, satellites);
    return 0;
}
