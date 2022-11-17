#ifndef UTIL
#define UTIL
#include <vector>
#include <string>
#include <sstream>

namespace util
{
    //將string根據splitC做分割
    std::vector<std::string> splitString(char splitC, std::string str); 
}
#endif