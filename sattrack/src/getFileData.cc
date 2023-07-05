#include "getFileData.h"
#include "satellite.h"
#include "groundStation.h"
#include "util.h"
#include <SGP4.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include<vector>
#include <utility>
#include <set>
#include <algorithm>

namespace getFileData
{
    //獲得satellite table
    std::map<int, satellite::satellite> getSatellitesTable(std::string fileName,std::map<int, std::map<int, bool>> &closeLinksTable, int ISLfrontAngle, int ISLrightAngle, int ISLbackAngle, int ISLleftAngle){
        std::map<int, satellite::satellite> satellites;
        if(fileName == "TLE_7P_16Sats.txt"){
            std::cout<<"reading TLE_7P_16Sats.txt\n";
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
                satellite::satellite s("7P_16Sats", satelliteTLE,newSatelliteSGP4data, id, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            }            
        }
        else if(fileName == "TLE_6P_22Sats.txt"){
            std::cout<<"reading TLE_6P_22Sats.txt\n";
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
                satellite::satellite s("6P_22Sats", satelliteTLE,newSatelliteSGP4data, id, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            } 
        }
        else if(fileName == "TLE_6P_44Sats.txt"){
            std::cout<<"reading TLE_6P_44Sats.txt\n";
            std::ifstream ifs(fileName);
            std::vector<std::string> satelliteNumbers;
            std::vector<std::pair<std::string,std::string>> TLEs;
            /*----------------get input data from file--------------*/
            if (!ifs.is_open()) {
                std::cout << "Failed to open file.\n";
                exit(EXIT_FAILURE);
            }
            for(int i = 0; i < 264; ++i){
                std::string temp;
                std::getline (ifs,temp);
                while(temp.find("Satellite") == std::string::npos){
                    std::getline (ifs,temp);
                }
                satelliteNumbers.push_back(temp.substr(40,3));
                std::getline (ifs,temp);//"---------------------------------------------------------------------"
                std::string line1, line2;
                std::getline (ifs,line1);
                std::getline (ifs,line2);
                // std::cout<<"line1:"<<line1<<",  ";
                // std::cout<<"line2:"<<line2<<"\n";
                TLEs.push_back(std::pair<std::string, std::string>(line1, line2));
            }
            ifs.close();
            /*-----------------------------------------------------*/
            for(size_t i = 0; i < 264; ++i){ //build table
                // std::cout<<"line1: "<<TLEs[i].first<<",line2: "<<TLEs[i].second<<"\n";
                //example:
                //     Tle satellite = Tle("satellite name",
                // "1 99999U          21305.00000000 -.00000094  00000-0 -36767-4 0 00007",
                // "2 99999 029.0167 000.0781 0005106 264.8962 095.3405 14.28719153000015");
                // SGP4 observerSgp4(satellite);
                const Tle satelliteTLE = Tle(satelliteNumbers[i], TLEs[i].first, TLEs[i].second);
                SGP4 newSatelliteSGP4data(satelliteTLE);
                int id = stoi(satelliteNumbers[i]);
                satellite::satellite s("6P_44Sats", satelliteTLE,newSatelliteSGP4data, id, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            } 
        }
        else if(fileName == "TLE_8P_33Sats.txt"){
            std::cout<<"reading TLE_8P_33Sats.txt\n";
            std::ifstream ifs(fileName);
            std::vector<std::string> satelliteNumbers;
            std::vector<std::pair<std::string,std::string>> TLEs;
            /*----------------get input data from file--------------*/
            if (!ifs.is_open()) {
                std::cout << "Failed to open file.\n";
                exit(EXIT_FAILURE);
            }
            for(int i = 0; i < 264; ++i){
                std::string temp;
                std::getline (ifs,temp);
                while(temp.find("Satellite") == std::string::npos){
                    std::getline (ifs,temp);
                }
                satelliteNumbers.push_back(temp.substr(40,3));
                std::getline (ifs,temp);//"---------------------------------------------------------------------"
                std::string line1, line2;
                std::getline (ifs,line1);
                std::getline (ifs,line2);
                // std::cout<<"line1:"<<line1<<",  ";
                // std::cout<<"line2:"<<line2<<"\n";
                TLEs.push_back(std::pair<std::string, std::string>(line1, line2));
            }
            ifs.close();
            /*-----------------------------------------------------*/
            for(size_t i = 0; i < 264; ++i){ //build table
                // std::cout<<"line1: "<<TLEs[i].first<<",line2: "<<TLEs[i].second<<"\n";
                //example:
                //     Tle satellite = Tle("satellite name",
                // "1 99999U          21305.00000000 -.00000094  00000-0 -36767-4 0 00007",
                // "2 99999 029.0167 000.0781 0005106 264.8962 095.3405 14.28719153000015");
                // SGP4 observerSgp4(satellite);
                const Tle satelliteTLE = Tle(satelliteNumbers[i], TLEs[i].first, TLEs[i].second);
                SGP4 newSatelliteSGP4data(satelliteTLE);
                int id = stoi(satelliteNumbers[i]);
                satellite::satellite s("8P_33Sats", satelliteTLE,newSatelliteSGP4data, id, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            } 
        }
        else if(fileName == "TLE_12P_22Sats.txt"){
            std::cout<<"reading TLE_12P_22Sats.txt\n";
            std::ifstream ifs(fileName);
            std::vector<std::string> satelliteNumbers;
            std::vector<std::pair<std::string,std::string>> TLEs;
            /*----------------get input data from file--------------*/
            if (!ifs.is_open()) { 
                std::cout << "Failed to open file.\n";
                exit(EXIT_FAILURE);
            }
            std::set<int> Set;
            for(int i = 0; i < 264; ++i){
                std::string temp;
                std::getline (ifs,temp);
                while(temp.find("Satellite") == std::string::npos){
                    std::getline (ifs,temp);
                }
                Set.insert(std::stoi(temp.substr(40,4)));
                satelliteNumbers.push_back(temp.substr(40,4));
                std::getline (ifs,temp);//"---------------------------------------------------------------------"
                std::string line1, line2;
                std::getline (ifs,line1);
                std::getline (ifs,line2);
                // std::cout<<"line1:"<<line1<<",  ";
                // std::cout<<"line2:"<<line2<<"\n";
                TLEs.push_back(std::pair<std::string, std::string>(line1, line2));
            }
            ifs.close();
            /*-----------------------------------------------------*/
            for(size_t i = 0; i < 264; ++i){ //build table
                // std::cout<<"line1: "<<TLEs[i].first<<",line2: "<<TLEs[i].second<<"\n";
                //example:
                //     Tle satellite = Tle("satellite name",
                // "1 99999U          21305.00000000 -.00000094  00000-0 -36767-4 0 00007",
                // "2 99999 029.0167 000.0781 0005106 264.8962 095.3405 14.28719153000015");
                // SGP4 observerSgp4(satellite);
                const Tle satelliteTLE = Tle(satelliteNumbers[i], TLEs[i].first, TLEs[i].second);
                SGP4 newSatelliteSGP4data(satelliteTLE);
                int id = stoi(satelliteNumbers[i]);
                satellite::satellite s("12P_22Sats", satelliteTLE,newSatelliteSGP4data, id, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            } 
        }                        
        else if(fileName == "TLE_6P_22Sats_38deg.txt"){
            std::cout<<"reading TLE_6P_22Sats_38deg.txt\n";
            std::ifstream ifs(fileName);
            std::vector<std::string> satelliteNumbers;
            std::vector<std::pair<std::string,std::string>> TLEs;
            /*----------------get input data from file--------------*/
            if (!ifs.is_open()) {
                std::cout << "Failed to open file.\n";
                exit(EXIT_FAILURE);
            }
            std::string description; //TLE檔案中的第一行會把所有衛星編號列出來
            std::getline (ifs,description);
            satelliteNumbers = util::splitString(',', description);
            size_t satCnt = satelliteNumbers.size();
            for(size_t i = 0; i < satCnt; ++i){
                satelliteNumbers[i] = satelliteNumbers[i].substr(21,3);
                // std::cout<<"i = "<<i<<":"<<satelliteNumbers[i]<<"\n";
            }
            // std::cout<<"\n";
            for(size_t i = 0; i < satCnt; ++i){
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
            for(size_t i = 0; i < satCnt; ++i){ //build table
                // std::cout<<"line1: "<<TLEs[i].first<<",line2: "<<TLEs[i].second<<"\n";
                //example:
                //     Tle satellite = Tle("satellite name",
                // "1 99999U          21305.00000000 -.00000094  00000-0 -36767-4 0 00007",
                // "2 99999 029.0167 000.0781 0005106 264.8962 095.3405 14.28719153000015");
                // SGP4 observerSgp4(satellite);
                const Tle satelliteTLE = Tle(satelliteNumbers[i], TLEs[i].first, TLEs[i].second);
                SGP4 newSatelliteSGP4data(satelliteTLE);
                int id = stoi(satelliteNumbers[i]);
                satellite::satellite s("6P_22Sats", satelliteTLE,newSatelliteSGP4data, id, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            }                 
        }
        else if(fileName == "TLE_6P_44Sats_38deg.txt"){
            std::cout<<"reading TLE_6P_44Sats_38deg.txt\n";
            std::ifstream ifs(fileName);
            std::vector<std::string> satelliteNumbers;
            std::vector<std::pair<std::string,std::string>> TLEs;
            /*----------------get input data from file--------------*/
            if (!ifs.is_open()) {
                std::cout << "Failed to open file.\n";
                exit(EXIT_FAILURE);
            }
            std::string description; //TLE檔案中的第一行會把所有衛星編號列出來
            std::getline (ifs,description);
            satelliteNumbers = util::splitString(',', description);
            size_t satCnt = satelliteNumbers.size();
            for(size_t i = 0; i < satCnt; ++i){
                satelliteNumbers[i] = satelliteNumbers[i].substr(21,4);
                // std::cout<<"i = "<<i<<":"<<satelliteNumbers[i]<<"\n";
            }
            // std::cout<<"\n";
            for(size_t i = 0; i < satCnt; ++i){
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
            for(size_t i = 0; i < satCnt; ++i){ //build table
                // std::cout<<"line1: "<<TLEs[i].first<<",line2: "<<TLEs[i].second<<"\n";
                //example:
                //     Tle satellite = Tle("satellite name",
                // "1 99999U          21305.00000000 -.00000094  00000-0 -36767-4 0 00007",
                // "2 99999 029.0167 000.0781 0005106 264.8962 095.3405 14.28719153000015");
                // SGP4 observerSgp4(satellite);
                const Tle satelliteTLE = Tle(satelliteNumbers[i], TLEs[i].first, TLEs[i].second);
                SGP4 newSatelliteSGP4data(satelliteTLE);
                int id = stoi(satelliteNumbers[i]);
                satellite::satellite s("6P_44Sats", satelliteTLE,newSatelliteSGP4data, id, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            }                       
        }        
        else if(fileName == "TLE_8P_33Sats_38deg.txt"){
            std::cout<<"reading TLE_8P_33Sats_38deg.txt\n";
            std::ifstream ifs(fileName);
            std::vector<std::string> satelliteNumbers;
            std::vector<std::pair<std::string,std::string>> TLEs;
            /*----------------get input data from file--------------*/
            if (!ifs.is_open()) {
                std::cout << "Failed to open file.\n";
                exit(EXIT_FAILURE);
            }
            std::string description; //TLE檔案中的第一行會把所有衛星編號列出來
            std::getline (ifs,description);
            satelliteNumbers = util::splitString(',', description);
            size_t satCnt = satelliteNumbers.size();
            for(size_t i = 0; i < satCnt; ++i){
                satelliteNumbers[i] = satelliteNumbers[i].substr(21,4);
                // std::cout<<"i = "<<i<<":"<<satelliteNumbers[i]<<"\n";
            }
            // std::cout<<"\n";
            for(size_t i = 0; i < satCnt; ++i){
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
            for(size_t i = 0; i < satCnt; ++i){ //build table
                // std::cout<<"line1: "<<TLEs[i].first<<",line2: "<<TLEs[i].second<<"\n";
                //example:
                //     Tle satellite = Tle("satellite name",
                // "1 99999U          21305.00000000 -.00000094  00000-0 -36767-4 0 00007",
                // "2 99999 029.0167 000.0781 0005106 264.8962 095.3405 14.28719153000015");
                // SGP4 observerSgp4(satellite);
                const Tle satelliteTLE = Tle(satelliteNumbers[i], TLEs[i].first, TLEs[i].second);
                SGP4 newSatelliteSGP4data(satelliteTLE);
                int id = stoi(satelliteNumbers[i]);
                satellite::satellite s("8P_33Sats", satelliteTLE,newSatelliteSGP4data, id, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            }                
        }         
        else if(fileName == "TLE_12P_22Sats_38deg.txt"){
            std::cout<<"reading TLE_12P_22Sats_38deg.txt\n";
            std::ifstream ifs(fileName);
            std::vector<std::string> satelliteNumbers;
            std::vector<std::pair<std::string,std::string>> TLEs;
            /*----------------get input data from file--------------*/
            if (!ifs.is_open()) {
                std::cout << "Failed to open file.\n";
                exit(EXIT_FAILURE);
            }
            std::string description; //TLE檔案中的第一行會把所有衛星編號列出來
            std::getline (ifs,description);
            satelliteNumbers = util::splitString(',', description);
            size_t satCnt = satelliteNumbers.size();
            for(size_t i = 0; i < satCnt; ++i){
                satelliteNumbers[i] = satelliteNumbers[i].substr(21,4);
                // std::cout<<"i = "<<i<<":"<<satelliteNumbers[i]<<"\n";
            }
            // std::cout<<"\n";
            for(size_t i = 0; i < satCnt; ++i){
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
            for(size_t i = 0; i < satCnt; ++i){ //build table
                // std::cout<<"line1: "<<TLEs[i].first<<",line2: "<<TLEs[i].second<<"\n";
                //example:
                //     Tle satellite = Tle("satellite name",
                // "1 99999U          21305.00000000 -.00000094  00000-0 -36767-4 0 00007",
                // "2 99999 029.0167 000.0781 0005106 264.8962 095.3405 14.28719153000015");
                // SGP4 observerSgp4(satellite);
                const Tle satelliteTLE = Tle(satelliteNumbers[i], TLEs[i].first, TLEs[i].second);
                SGP4 newSatelliteSGP4data(satelliteTLE);
                int id = stoi(satelliteNumbers[i]);
                satellite::satellite s("12P_22Sats", satelliteTLE,newSatelliteSGP4data, id, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(id,s));
            }            
        }   
        else{
            std::cout<<"can not read "<<fileName<<" !\n";
            exit(-1);
        }
        // std::cout<<"finish!\n";
        return satellites;
    }

    //獲得parameterTable，其中記錄模擬所設置的各種parameter
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

    //獲得parameter.txt中設置的經緯度們的地面站物件(不只一個)
    std::vector<groundStation::groundStation> getInputStations(const std::map<std::string, std::string> &parameterTable){
        std::vector<std::string> stationLatStr = util::splitString(',', parameterTable.at("areaStationLatitudes"));
        std::vector<std::string> stationLongStr = util::splitString(',', parameterTable.at("areaStationLongitudes"));
        std::vector<std::string> stationAltStr = util::splitString(',', parameterTable.at("areaStationAltitudes"));
        if(!((stationLatStr.size() == stationLongStr.size()) && (stationLongStr.size() == stationAltStr.size()))){
            std::cout<<"stations location setting error!"<<"\n";
            exit(-1);
        }
        std::vector<double> stationLatitudes = util::strVec2DoubleVec(stationLatStr);
        std::vector<double> stationLongitudes = util::strVec2DoubleVec(stationLongStr);
        std::vector<double> stationAltitudes = util::strVec2DoubleVec(stationAltStr);
        std::vector<groundStation::groundStation> stations;
        for(size_t i = 0; i < stationLatitudes.size(); ++i){
            stations.push_back(groundStation::groundStation(stationLatitudes[i], stationLongitudes[i], stationAltitudes[i]));
        }       
        return stations; 
    }

    //獲得要關掉的Link的table(table[satId1][satId2]代表satId1與satId2之間的Link被關掉)
    std::map<int, std::map<int, bool>> getCloseLinkTable(std::string fileName){
        std::map<int, std::map<int, bool>> closeLinkTable;
        std::ifstream ifs(fileName);
        if (!ifs.is_open()) {
            std::cout << "Failed to open file.\n";
            exit(EXIT_FAILURE);
        }  
        std::string s;
        while ( std::getline (ifs,s) ){
            if(s[0] != '('){
                continue;
            }
            std::string leftBracket = "(";
            std::string rightBracket = ")";
            size_t leftPos = 0;
            size_t rightPos = 0;
            std::string token;
            leftPos = s.find(leftBracket);
            rightPos = s.find(rightBracket);
            token = s.substr(leftPos, rightPos-leftPos);
            token.erase(0, 1);
            // std::cout<<token<<"\n";
            std::vector<int> link = util::strVec2IntVec(util::splitString(',', token));
            closeLinkTable[link[0]][link[1]] = true;
            closeLinkTable[link[1]][link[0]] = true;
        }
        ifs.close();  
        return closeLinkTable;        
    }
}

