#ifndef UTIL
#define UTIL
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <bitset>
#include <map>
#include "satellite.h"

namespace util
{
    //將string根據splitC做分割
    std::vector<std::string> splitString(char splitC, std::string str); 

    //將字串vector轉換成double vector
    std::vector<double> strVec2DoubleVec(const std::vector<std::string> &v);

    //將字串vector轉換成int vector
    std::vector<int> strVec2IntVec(const std::vector<std::string> &v);

    //回傳vector中所有bitset進行OR operation的結果
    std::bitset<86400> orAllElement(const std::vector<std::bitset<86400>> v);

    //根據input parameter stateOfDay，回傳一個vector，裡面是紀錄每個connection state改變的時間點，及他是Link Breaking(false)還是connecting(true)
    std::vector<std::pair<size_t, bool>> getStateChangeInfo(std::bitset<86400> &stateOfDay);

    //印出時間
    void printTime(size_t t, std::ofstream &_output, bool _printSecond);

    //印出虛線
    void printDashLine(std::ofstream &outputint, int lineWidth);

    //印出星群表格的第一列(每個衛星的ID)
    void printTableFirstLine(std::ofstream &output, long unsigned int satCount, long unsigned int satCountPerOrbit);

    //獲得vector所有元素的平均
    double getAverage(std::vector<int> const& v);

    //印出要關掉的Link
    void getClosedLinkFile(std::map<int, satellite::satellite> &satellites, std::map<std::string, std::string> &parameterTable);    
}
#endif