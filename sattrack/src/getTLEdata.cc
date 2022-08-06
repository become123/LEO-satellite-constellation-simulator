#include "getTLEdata.h"
#include "satellite.h"
#include <SGP4.h>
#include <iostream>
#include <iomanip>
#include<fstream>
#include<string>
#include<unordered_map>
#include<vector>
#include<utility>

namespace getTLEdata
{
    std::unordered_map<int, satellite::satellite> getTLEdata(std::string fileName){
        std::ifstream ifs(fileName);
        std::unordered_map<int, satellite::satellite> satellites;
        std::vector<std::string> satelliteNumbers;
        std::vector<std::pair<std::string,std::string>> TLEs;
        /*----------------get input data from file--------------*/
        if (!ifs.is_open()) {
            std::cout << "Failed to open file.\n";
            exit(EXIT_FAILURE);
        }
        for(int i = 0; i < 112; ++i){
            std::string temp;
            std::getline (ifs,temp);
            satelliteNumbers.push_back(temp.substr(15,3));
        }
        for(int i = 0; i < 112; ++i){
            std::string temp;
            while(temp != "---------------------------------------------------------------------"){
                std::getline (ifs,temp);
            }
            std::string line1, line2;
            std::getline (ifs,line1);
            std::getline (ifs,line2);
            TLEs.push_back(std::pair<std::string, std::string>(line1, line2));
        }
        ifs.close();
        /*-----------------------------------------------------*/
        for(size_t i = 0; i < 112; ++i){ //build table
            // std::cout<<"line1: "<<TLEs[i].first<<",line2: "<<TLEs[i].second<<"\n";
            //example:
            //     Tle satellite = Tle("satellite name",
            // "1 99999U          21305.00000000 -.00000094  00000-0 -36767-4 0 00007",
            // "2 99999 029.0167 000.0781 0005106 264.8962 095.3405 14.28719153000015");
            // SGP4 observerSgp4(satellite);
            const Tle satelliteTLE = Tle(satelliteNumbers[i], TLEs[i].first, TLEs[i].second);
            SGP4 newSatelliteSGP4data(satelliteTLE);
            satellite::satellite s(satelliteTLE,newSatelliteSGP4data);
            satellites.insert(std::make_pair(stoi(satelliteNumbers[i]),s));
        }
        return satellites;
    }
}

