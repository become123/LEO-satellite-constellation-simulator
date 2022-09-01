#include "getFileData.h"
#include "satellite.h"
#include <SGP4.h>
#include <iostream>
#include <iomanip>
#include<fstream>
#include<string>
#include<map>
#include<vector>
#include<utility>

namespace getFileData
{
    std::map<int, satellite::satellite> getSatellitesTable(std::string fileName, int ISLfrontAngle, int ISLrightAngle, int ISLbackAngle, int ISLleftAngle){
        std::map<int, satellite::satellite> satellites;
        if(fileName == "TLE_7P_16Sats.txt"){
            std::ifstream ifs(fileName);
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
                int id = stoi(satelliteNumbers[i]);
                satellite::satellite s("7P_16Sats", satelliteTLE,newSatelliteSGP4data, id, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            }            
        }
        else if(fileName == "TLE_6P_22Sats.txt"){
            std::ifstream ifs(fileName);
            std::vector<std::string> satelliteNumbers;
            std::vector<std::pair<std::string,std::string>> TLEs;
            /*----------------get input data from file--------------*/
            if (!ifs.is_open()) {
                std::cout << "Failed to open file.\n";
                exit(EXIT_FAILURE);
            }
            for(int i = 0; i < 132; ++i){
                std::string temp;
                std::getline (ifs,temp);
                satelliteNumbers.push_back(temp.substr(15,3));
            }
            for(int i = 0; i < 132; ++i){
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
            for(size_t i = 0; i < 132; ++i){ //build table
                // std::cout<<"line1: "<<TLEs[i].first<<",line2: "<<TLEs[i].second<<"\n";
                //example:
                //     Tle satellite = Tle("satellite name",
                // "1 99999U          21305.00000000 -.00000094  00000-0 -36767-4 0 00007",
                // "2 99999 029.0167 000.0781 0005106 264.8962 095.3405 14.28719153000015");
                // SGP4 observerSgp4(satellite);
                const Tle satelliteTLE = Tle(satelliteNumbers[i], TLEs[i].first, TLEs[i].second);
                SGP4 newSatelliteSGP4data(satelliteTLE);
                int id = stoi(satelliteNumbers[i]);
                satellite::satellite s("6P_22Sats", satelliteTLE,newSatelliteSGP4data, id, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            } 
        }
        else{
            std::cout<<"can not read "<<fileName<<" !\n";
            exit(-1);
        }
        return satellites;
    }

    std::map<std::string, std::string> getParameterdata(std::string fileName){
        std::map<std::string, std::string> parameterTable;
        std::ifstream ifs(fileName);
        if (!ifs.is_open()) {
            std::cout << "Failed to open file.\n";
            exit(EXIT_FAILURE);
        }  
        std::string s;
        while ( std::getline (ifs,s) ){
            if(s[0] != '>'){
                continue;
            }
            std::vector<std::string> data;
            //找出括號中的資料
            std::string leftBracket = "(";
            std::string rightBracket = ")";
            size_t leftPos = 0;
            size_t rightPos = 0;
            std::string token;
            leftPos = s.find(leftBracket);
            rightPos = s.find(rightBracket);
            while (leftPos != std::string::npos && rightPos != std::string::npos) {
                token = s.substr(leftPos, rightPos-leftPos);
                token.erase(0, 1);
                // std::cout<<token<<"\n";
                data.push_back(token);
                s.erase(0, rightPos + 1);
                leftPos = s.find(leftBracket);
                rightPos = s.find(rightBracket);
            }
            parameterTable[data[0]] = data[1];
        }
        ifs.close();  
        return parameterTable;            
    }
}

