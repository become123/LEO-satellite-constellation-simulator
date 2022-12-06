#include "util.h"
#include "satellite.h"
#include <vector>
#include <string>
#include <sstream>
#include <bitset>

namespace util
{
    //將string根據splitC做分割
    std::vector<std::string> splitString(char splitC, std::string str){
        std::stringstream sstr(str);
        std::vector<std::string> v;
        while(sstr.good()){
            std::string substr;
            getline(sstr, substr, splitC);
            v.push_back(substr);
        }	
        return v;
    } 

    //將字串vector轉換成double vector
    std::vector<double> strVec2DoubleVec(const std::vector<std::string> &v){
        std::vector<double> res;
        for(auto str:v){
            res.push_back(std::stod(str));
        }
        return res;
    }

    //將字串vector轉換成int vector
    std::vector<int> strVec2IntVec(const std::vector<std::string> &v){
        std::vector<int> res;
        for(auto str:v){
            res.push_back(std::stoi(str));
        }
        return res;
    }

    //回傳vector中所有bitset進行OR operation的結果
    std::bitset<86400> orAllElement(const std::vector<std::bitset<86400>> v){
        std::bitset<86400> res;
        for(auto &element: v){
            res |= element;
        }
        return res;
    }

    //根據input parameter stateOfDay，回傳一個vector，裡面是紀錄每個connection state改變的時間點，及他是Link Breaking(false)還是connecting(true)
    std::vector<std::pair<size_t, bool>> getStateChangeInfo(std::bitset<86400> &stateOfDay){
        std::vector<std::pair<size_t, bool>> stateChangeInfo;
        bool currentState = false;
        for(size_t i = 0; i < 86400; ++i){
            if(stateOfDay[i] != currentState){
                currentState = !currentState;
                stateChangeInfo.push_back(std::make_pair(i, currentState));
            }
        }
        if(currentState){ //到一天中的最後一秒還是可以連
            stateChangeInfo.push_back(std::make_pair(86400, false));
        }
        return stateChangeInfo;
    }  

    //印出時間
    void printTime(size_t t, std::ofstream &output, bool printSecond) { 
        int second, min, hour;
        hour = (int)t/3600;
        min = (int)t/60-(hour*60);
        second = (int)t-(min*60)-(hour*3600);
        if(printSecond){
            output<<t;
        }
        else{
            output<<hour<<":"<<min<<":"<<second;
        }
    }

    //印出虛線
    void printDashLine(std::ofstream &output, int lineWidth){
        for(int dash = 0; dash < lineWidth; ++dash) 
            output<<"-";
        output<<"\n";
    }

    //印出星群表格的第一列(每個衛星的ID)
    void printTableFirstLine(std::ofstream &output, long unsigned int satCount, long unsigned int satCountPerOrbit){
        output<<"     ";
        for(size_t i = 0; i < satCount; ++i){
            output<<std::setw(3)<<satellite::indexToSatId(i, satCountPerOrbit)<<" |";
        }
        output<<"\n";        
    }

    //印出要關掉的Link
    void getClosedLinkFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable){
        std::ofstream output("./" + parameterTable.at("outputFileName"));
        std::map<int, std::map<int, bool>> openLink;
        for(int i = 101; i <= 116; i+=4){
            openLink[i][satellites.at(i).getRightSatId()] = true;
        }
        for(int i = 201; i <= 216; i+=4){
            openLink[i][satellites.at(i).getRightSatId()] = true;
        }
        for(int i = 301; i <= 316; i+=4){
            openLink[i][satellites.at(i).getRightSatId()] = true;
        }
        for(int i = 401; i <= 416; i+=4){
            openLink[i][satellites.at(i).getRightSatId()] = true;
        }
        for(int i = 501; i <= 516; i+=4){
            openLink[i][satellites.at(i).getRightSatId()] = true;
        }
        for(int i = 601; i <= 616; i+=4){
            openLink[i][satellites.at(i).getRightSatId()] = true;
        }        
        for(int i = 701; i <= 716; i+=4){
            openLink[i][satellites.at(i).getRightSatId()] = true;
        }    
        
        for(int i = 101; i <= 116; i++){
            if(openLink[i][satellites.at(i).getRightSatId()]) continue;
            output<<"("<<i<<","<<satellites.at(i).getRightSatId()<<")\n";
        }
        for(int i = 201; i <= 216; i++){
            if(openLink[i][satellites.at(i).getRightSatId()]) continue;
            output<<"("<<i<<","<<satellites.at(i).getRightSatId()<<")\n";
        }
        for(int i = 301; i <= 316; i++){
            if(openLink[i][satellites.at(i).getRightSatId()]) continue;
            output<<"("<<i<<","<<satellites.at(i).getRightSatId()<<")\n";
        }
        for(int i = 401; i <= 416; i++){
            if(openLink[i][satellites.at(i).getRightSatId()]) continue;
            output<<"("<<i<<","<<satellites.at(i).getRightSatId()<<")\n";
        }
        for(int i = 501; i <= 516; i++){
            if(openLink[i][satellites.at(i).getRightSatId()]) continue;
            output<<"("<<i<<","<<satellites.at(i).getRightSatId()<<")\n";
        }
        for(int i = 601; i <= 616; i++){
            if(openLink[i][satellites.at(i).getRightSatId()]) continue;
            output<<"("<<i<<","<<satellites.at(i).getRightSatId()<<")\n";
        }        
        for(int i = 701; i <= 716; i++){
            if(openLink[i][satellites.at(i).getRightSatId()]) continue;
            output<<"("<<i<<","<<satellites.at(i).getRightSatId()<<")\n";
        }                 
        output.close();    
    }    
}

