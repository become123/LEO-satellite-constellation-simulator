#include "util.h"
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
}

