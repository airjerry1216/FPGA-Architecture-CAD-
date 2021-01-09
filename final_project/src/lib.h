#ifndef LIB_H_
#define LIB_H_
#include <string>
#include <vector>

class LUT {
public:
    std::string name;
    std::pair<int, int> coord;
    LUT();
};
class FF {
public:
    std::string name;
    std::pair<int, int> coord;
    FF();
};
class CLB {
public:
    std::pair<int, int> constraint;
    std::pair<int, int> coord;
    CLB();
};
class FPGA {
public:
    std::pair<int, int> CLBDim;
    std::vector<CLB> CLBs;
    std::pair<int, int> num_IOPad;
    int num_PI;
    int num_PO;
    std::pair<int, int> num_Inst;
    std::vector<std::pair<double, double> > PI;
    std::vector<std::pair<double, double> > PO;
    std::vector<LUT> LUTs;
    std::vector<FF> FFs;
    int num_nets;
    FPGA();
};
class Net {
public:
    std::string name;
    std::vector<std::string> pins;
    Net();
};
#endif