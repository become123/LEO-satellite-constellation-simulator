#ifndef AER_H
#define AER_H
#include <string>

struct AER
{
    AER(std::string _date, double _A, double _E, double _R):date(_date), A(_A), E(_E), R(_R) {}
    std::string date;
    double A;
    double E;
    double R;
};


#endif