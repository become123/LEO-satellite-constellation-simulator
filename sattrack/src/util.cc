#include <vector>
#include <string>
#include <sstream>

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
}

