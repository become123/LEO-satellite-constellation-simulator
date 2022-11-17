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

    std::vector<double> strVec2DoubleVec(const std::vector<std::string> &v){
        std::vector<double> res;
        for(auto str:v){
            res.push_back(std::stod(str));
        }
        return res;
    }

    std::bitset<86400> orAllElement(const std::vector<std::bitset<86400>> v){
        std::bitset<86400> res;
        for(auto &element: v){
            res |= element;
        }
        return res;
    }
}

