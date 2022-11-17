#ifndef UTIL
#define UTIL
#include <vector>
#include <string>
#include <sstream>
#include <bitset>

namespace util
{
    //將string根據splitC做分割
    std::vector<std::string> splitString(char splitC, std::string str); 

    std::vector<double> strVec2DoubleVec(const std::vector<std::string> &v);

    std::bitset<86400> orAllElement(const std::vector<std::bitset<86400>> v);
}
#endif