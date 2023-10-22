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
#include <unordered_map>
#include <vector>
#include <utility>
#include <set>
#include <algorithm>

namespace getFileData
{
    //獲得satellite table
    std::map<int, satellite::satellite> getSatellitesTable(std::map<int, std::map<int, bool>> &closeLinksTable, std::map<std::string, std::string> &parameterTable){
        std::string constellationInfoFileName = parameterTable.at("constellationInfoFileName");
        // std::cout<<constellationInfoFileName<<"\n";
        std::map<int, satellite::satellite> satellites;
        std::ifstream constellationInfoIfs(constellationInfoFileName);
        if (!constellationInfoIfs.is_open()) {
            std::cout << "Failed to open constellationInfoIfs.\n";
            exit(EXIT_FAILURE);
        }
        std::unordered_map<std::string, std::vector<int>> constellationInfoTable;
        std::string inputLine;
        std::cout<<"reading constellationInfo file\n";
        while ( std::getline (constellationInfoIfs,inputLine) ){
            if(inputLine == "------------------------(satId):(right,left,front,back)---------------------"){
                break;
            }            
            if(inputLine[0] != '>'){
                continue;
            }
            std::vector<std::string> data;
            //找出括號中的資料
            std::string leftBracket = "(";
            std::string rightBracket = ")";
            size_t leftPos = 0;
            size_t rightPos = 0;
            std::string token;
            leftPos = inputLine.find(leftBracket);
            rightPos = inputLine.find(rightBracket);
            while (leftPos != std::string::npos && rightPos != std::string::npos) {
                token = inputLine.substr(leftPos, rightPos-leftPos);
                token.erase(0, 1);
                // std::cout<<token<<"\n";
                data.push_back(token);
                inputLine.erase(0, rightPos + 1);
                leftPos = inputLine.find(leftBracket);
                rightPos = inputLine.find(rightBracket);
            }
            constellationInfoTable[data[0]] = {std::stoi(data[1])};
        }
        while ( std::getline (constellationInfoIfs,inputLine) ){
            std::vector<std::string> data;
            //找出括號中的資料
            std::string leftBracket = "(";
            std::string rightBracket = ")";
            size_t leftPos = 0;
            size_t rightPos = 0;
            std::string token;
            leftPos = inputLine.find(leftBracket);
            rightPos = inputLine.find(rightBracket);
            while (leftPos != std::string::npos && rightPos != std::string::npos) {
                token = inputLine.substr(leftPos, rightPos-leftPos);
                token.erase(0, 1);
                // std::cout<<token<<"\n";
                data.push_back(token);
                inputLine.erase(0, rightPos + 1);
                leftPos = inputLine.find(leftBracket);
                rightPos = inputLine.find(rightBracket);
            }
            std::vector<int> neighborIds = util::strVec2IntVec(util::splitString(',', data[1]));
            constellationInfoTable[data[0]] = neighborIds;
        }
        constellationInfoIfs.close();
        // for(auto p:constellationInfoTable){
        //     std::cout<<p.first<<":";
        //     for(auto i:p.second){
        //         std::cout<<i<<",";
        //     }
        //     std::cout<<"\n";
        // }
        int ISLfrontAngle = std::stoi(parameterTable.at("ISLfrontAngle"));
        int ISLrightAngle = std::stoi(parameterTable.at("ISLrightAngle"));
        int ISLbackAngle = std::stoi(parameterTable.at("ISLbackAngle"));
        int ISLleftAngle = std::stoi(parameterTable.at("ISLleftAngle"));        
        std::string TLE_inputFileName = parameterTable.at("TLE_inputFileName");
        std::cout<<"reading TLE file\n";
        std::ifstream ifs(TLE_inputFileName);
        if (!ifs.is_open()) {
            std::cout << "Failed to open file.\n";
            exit(EXIT_FAILURE);
        }        
        int orbitCount = constellationInfoTable.at("orbitCount")[0];
        int satCountPerOrbit = constellationInfoTable.at("satCountPerOrbit")[0];
        for(int orbidId = 1; orbidId <= orbitCount; ++orbidId){
            for(int satId = 1; satId <= satCountPerOrbit; ++satId){
                int curId = orbidId*100+satId;
                std::string temp;
                while(temp != "---------------------------------------------------------------------"){
                    std::getline (ifs,temp);
                }
                std::string line1, line2;
                std::getline (ifs,line1);
                std::getline (ifs,line2);
                const Tle satelliteTLE = Tle(std::to_string(orbidId*100+satId), line1, line2);
                SGP4 newSatelliteSGP4data(satelliteTLE);
                satellite::satellite s(constellationInfoTable, satelliteTLE,newSatelliteSGP4data, curId, closeLinksTable, ISLfrontAngle, ISLrightAngle, ISLbackAngle, ISLleftAngle);
                satellites.insert(std::make_pair(curId,s));                
            }
        }         
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

