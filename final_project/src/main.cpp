#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <climits>
#include <ctime>
#include "lib.h"
using namespace std;
//../bin/project ../benchmarks/alu4_4.info ../benchmarks/alu4_4.nets ../outputs/alu4_4.placement
//python3 ./verifier/verify.py ./benchmarks/alu4_4.info ./benchmarks/alu4_4.nets ./outputs/alu4_4.placement
//int betterSwap = 0, worseSwap = 0, betterMove = 0, worseMove = 0, failMove = 0;
double computeHPWL(FPGA &fpga, vector<Net> &nets)
{
    double HPWL = 0;
    for (int i = 0; i < fpga.num_nets; ++i) {
        double minX = numeric_limits<double>::max(), maxX = 0, minY = numeric_limits<double>::max(), maxY = 0;
        for (int j = 0; j < nets[i].pins.size(); ++j) {
            double x = 0, y = 0;
            if (nets[i].pins[j][0] == 'L') {
                x = fpga.LUTs[stoi(nets[i].pins[j].substr(1)) - 1].coord.first;
                y = fpga.LUTs[stoi(nets[i].pins[j].substr(1)) - 1].coord.second;
            }
            else if (nets[i].pins[j][0] == 'F') {
                x = fpga.FFs[stoi(nets[i].pins[j].substr(1)) - 1].coord.first;
                y = fpga.FFs[stoi(nets[i].pins[j].substr(1)) - 1].coord.second;
            }
            else if (nets[i].pins[j][0] == 'I') {
                x = fpga.PI[stoi(nets[i].pins[j].substr(1)) - 1].first;
                y = fpga.PI[stoi(nets[i].pins[j].substr(1)) - 1].second;
            }
            else if (nets[i].pins[j][0] == 'O') {
                x = fpga.PO[stoi(nets[i].pins[j].substr(1)) - 1].first;
                y = fpga.PO[stoi(nets[i].pins[j].substr(1)) - 1].second;
            }
            minX = min(minX, x);
            maxX = max(maxX, x);
            minY = min(minY, y);
            maxY = max(maxY, y);
        }
        HPWL += (maxX - minX) + (maxY - minY);
    }
    return HPWL;
}
void randomPlacer(FPGA &fpga)
{
    for (int i = 0; i < fpga.num_Inst.first; ++i) {
        while (1) {
            int location = rand() % fpga.CLBs.size();
            if (fpga.CLBs[location].constraint.first < 2) {
                fpga.LUTs[i].coord.first = fpga.CLBs[location].coord.first;
                fpga.LUTs[i].coord.second = fpga.CLBs[location].coord.second;
                ++fpga.CLBs[location].constraint.first;
                break;
            }  
        }
    }
    for (int i = 0; i < fpga.num_Inst.second; ++i) {
        while (1) {
            int location = rand() % fpga.CLBs.size();
            if (fpga.CLBs[location].constraint.second < 2) {
                fpga.FFs[i].coord.first = fpga.CLBs[location].coord.first;
                fpga.FFs[i].coord.second = fpga.CLBs[location].coord.second;
                ++fpga.CLBs[location].constraint.second;
                break;
            }    
        }
    }
    return;
}
void randomSwapLUT(FPGA &fpga, vector<Net> &nets, double &bestHPWL)
{
    int LUT1 = rand() % fpga.num_Inst.first;
    int LUT2 = rand() % fpga.num_Inst.first;
    swap(fpga.LUTs[LUT1].coord.first, fpga.LUTs[LUT2].coord.first);
    swap(fpga.LUTs[LUT1].coord.second, fpga.LUTs[LUT2].coord.second);
    double tmpHPWL = computeHPWL(fpga, nets);
    if (tmpHPWL > bestHPWL) {
        swap(fpga.LUTs[LUT1].coord.first, fpga.LUTs[LUT2].coord.first);
        swap(fpga.LUTs[LUT1].coord.second, fpga.LUTs[LUT2].coord.second);
        //++worseSwap;
    }
    else {
        bestHPWL = tmpHPWL;
        //++betterSwap;
    }
    return;
}
void randomSwapFF(FPGA &fpga, vector<Net> &nets, double &bestHPWL)
{
    int FF1 = rand() % fpga.num_Inst.second;
    int FF2 = rand() % fpga.num_Inst.second;
    swap(fpga.FFs[FF1].coord.first, fpga.FFs[FF2].coord.first);
    swap(fpga.FFs[FF1].coord.second, fpga.FFs[FF2].coord.second);
    double tmpHPWL = computeHPWL(fpga, nets);
    if (tmpHPWL > bestHPWL) {
        swap(fpga.FFs[FF1].coord.first, fpga.FFs[FF2].coord.first);
        swap(fpga.FFs[FF1].coord.second, fpga.FFs[FF2].coord.second);
        //++worseSwap;
    }
    else {
        bestHPWL = tmpHPWL;
        //++betterSwap;
    }
    return;
}
void randomMoveLUT(FPGA &fpga, vector<Net> &nets, double &bestHPWL, bool stage)
{
    int LUT1 = rand() % fpga.num_Inst.first, CLB1 = 0;
    if (!stage)
        CLB1 = rand() % fpga.CLBs.size();
    else {
        double CRSelect =  (double) rand() / (RAND_MAX + 1.0);
        if (CRSelect >= 0.5)
            CLB1 = (fpga.LUTs[LUT1].coord.second - 1) * fpga.CLBDim.first + (rand() % fpga.CLBDim.first);
        else
            CLB1 = (rand() % fpga.CLBDim.second) * fpga.CLBDim.first + (fpga.LUTs[LUT1].coord.first - 1);
    }
    if (fpga.CLBs[CLB1].constraint.first < 2) {
        int LUT1CoordFirst = fpga.LUTs[LUT1].coord.first;
        int LUT1CoordSecond = fpga.LUTs[LUT1].coord.second;
        int originalCLB = (LUT1CoordSecond - 1) * fpga.CLBDim.first + (LUT1CoordFirst - 1);
        fpga.LUTs[LUT1].coord.first = fpga.CLBs[CLB1].coord.first;
        fpga.LUTs[LUT1].coord.second = fpga.CLBs[CLB1].coord.second;
        ++fpga.CLBs[CLB1].constraint.first;
        --fpga.CLBs[originalCLB].constraint.first;
        double tmpHPWL = computeHPWL(fpga, nets);
        if (tmpHPWL > bestHPWL) {
            fpga.LUTs[LUT1].coord.first = LUT1CoordFirst;
            fpga.LUTs[LUT1].coord.second = LUT1CoordSecond;
            --fpga.CLBs[CLB1].constraint.first;
            ++fpga.CLBs[originalCLB].constraint.first;
            //++worseMove;
        }
        else {
            bestHPWL = tmpHPWL;
            //++betterMove;
        }
    }
    /*else
        ++failMove;*/
    return;
}
void randomMoveFF(FPGA &fpga, vector<Net> &nets, double &bestHPWL, bool stage)
{
    int FF1 = rand() % fpga.num_Inst.second, CLB1 = 0;
    if (!stage)
        CLB1 = rand() % fpga.CLBs.size();
    else {
        double CRSelect =  (double) rand() / (RAND_MAX + 1.0);
        if (CRSelect >= 0.5)
            CLB1 = (fpga.FFs[FF1].coord.second - 1) * fpga.CLBDim.first + (rand() % fpga.CLBDim.first);
        else
            CLB1 = (rand() % fpga.CLBDim.second) * fpga.CLBDim.first + (fpga.FFs[FF1].coord.first - 1);
    }
    if (fpga.CLBs[CLB1].constraint.second < 2) {
        int FF1CoordFirst = fpga.FFs[FF1].coord.first;
        int FF1CoordSecond = fpga.FFs[FF1].coord.second;
        int originalCLB = (FF1CoordSecond - 1) * fpga.CLBDim.first + (FF1CoordFirst - 1);
        fpga.FFs[FF1].coord.first = fpga.CLBs[CLB1].coord.first;
        fpga.FFs[FF1].coord.second = fpga.CLBs[CLB1].coord.second;
        ++fpga.CLBs[CLB1].constraint.second;
        --fpga.CLBs[originalCLB].constraint.second;
        double tmpHPWL = computeHPWL(fpga, nets);
        if (tmpHPWL > bestHPWL) {
            fpga.FFs[FF1].coord.first = FF1CoordFirst;
            fpga.FFs[FF1].coord.second = FF1CoordSecond;
            --fpga.CLBs[CLB1].constraint.second;
            ++fpga.CLBs[originalCLB].constraint.second;
            //++worseMove;
        }
        else {
            bestHPWL = tmpHPWL;
            //++betterMove;
        }
    }
    /*else
        ++failMove;*/
    return;
}
int main(int argc, char *argv[]) 
{
    clock_t startTime = clock();
    ifstream info_file(argv[1]);
    ifstream nets_file(argv[2]);
    ofstream output_file(argv[3]);
    stringstream ss;
    string line;
    FPGA fpga;
    vector<Net> nets;
    double bestHPWL = 0;
/*-------------------------------------------------------------------------------------------------------------*/
    int lineCnt = 0, type = 1;
    while (getline(info_file, line)) {
        string tmp;
        if (!lineCnt) {
            ss << line;
            ss >> tmp >> fpga.CLBDim.first;
            ss >> fpga.CLBDim.second;
            ss.str("");
            ss.clear();
            fpga.CLBs.resize(fpga.CLBDim.first * fpga.CLBDim.second);
        }
        else if (lineCnt == 1) {
            ss << line;
            ss >> tmp >> fpga.num_IOPad.first;
            ss >> fpga.num_IOPad.second;
            ss.str("");
            ss.clear();
        }
        else {
            if (type == 1) {
                ss << line;
                ss >> tmp >> fpga.num_PI;
                ss.str("");
                ss.clear();
                fpga.PI.resize(fpga.num_PI);
                for (int i = 0; i < fpga.num_PI; ++i) {
                    getline(info_file, line);
                    ss << line;
                    ss >> tmp >> fpga.PI[i].first;
                    ss >> fpga.PI[i].second;
                    ss.str("");
                    ss.clear();
                }
                type = 2;
            }
            else if (type == 2) {
                ss << line;
                ss >> tmp >> fpga.num_PO;
                ss.str("");
                ss.clear();
                fpga.PO.resize(fpga.num_PO);
                for (int i = 0; i < fpga.num_PO; ++i) {
                    getline(info_file, line);
                    ss << line;
                    ss >> tmp >> fpga.PO[i].first;
                    ss >> fpga.PO[i].second;
                    ss.str("");
                    ss.clear();
                }
                type = 3;
            }
            else if (type == 3) {
                ss << line;
                ss >> tmp >> fpga.num_Inst.first >> fpga.num_Inst.second;
                ss.str("");
                ss.clear();
                fpga.LUTs.resize(fpga.num_Inst.first);
                fpga.FFs.resize(fpga.num_Inst.second);
                for (int i = 0; i < fpga.num_Inst.first; ++i) {
                    getline(info_file, line);
                    ss << line;
                    ss >> fpga.LUTs[i].name;
                    ss.str("");
                    ss.clear();
                }
                for (int i = 0; i < fpga.num_Inst.second; ++i) {
                    getline(info_file, line);
                    ss << line;
                    ss >> fpga.FFs[i].name;
                    ss.str("");
                    ss.clear();
                }
            }
        }
        ++lineCnt;
    }
    while (getline(nets_file, line)) {
        ss << line;
        ss >> fpga.num_nets;
        ss.str("");
        ss.clear();
        nets.resize(fpga.num_nets);
        for (int i = 0; i < fpga.num_nets; ++i) {
            string tmp;
            getline(nets_file, line);
            ss << line;
            ss >> nets[i].name;
            while (ss >> tmp)
                nets[i].pins.push_back(tmp);
            ss.str("");
            ss.clear();
        }
    }
    for (int i = 0; i < fpga.CLBs.size(); ++i) {
        fpga.CLBs[i].coord.first = i % fpga.CLBDim.first + 1;
        fpga.CLBs[i].coord.second = i / fpga.CLBDim.first + 1;
    }
/*-------------------------------------------------------------------------------------------------------------*/
    srand(123);
    randomPlacer(fpga);
    bestHPWL = computeHPWL(fpga, nets);
    cout << "initial HPWL: " << bestHPWL << endl;

    //int globalPlace = 1000 * (fpga.num_Inst.first + fpga.num_Inst.second);
    //int globalIteration = 0, detailIteration = 0;
    double FFPropotion = (double) fpga.num_Inst.second / (fpga.num_Inst.first + fpga.num_Inst.second);
    while ((clock() - startTime) / CLOCKS_PER_SEC < 1000) {
        double actionSelect = (double) rand() / (RAND_MAX + 1.0);
        if (actionSelect >= 0.5) {
            double typeSelect = (double) rand() / (RAND_MAX + 1.0);
            if (typeSelect >= FFPropotion)
                randomSwapLUT(fpga, nets, bestHPWL);
            else
                randomSwapFF(fpga, nets, bestHPWL);
        }
        else {
            double typeSelect = (double) rand() / (RAND_MAX + 1.0);
            if (typeSelect >= FFPropotion)
                randomMoveLUT(fpga, nets, bestHPWL, 0);
            else
                randomMoveFF(fpga, nets, bestHPWL, 0);
        }
        //++globalIteration;
    }
    while ((clock() - startTime) / CLOCKS_PER_SEC < 1700) {
        double actionSelect = (double) rand() / (RAND_MAX + 1.0);
        if (actionSelect >= 0.5) {
            double typeSelect = (double) rand() / (RAND_MAX + 1.0);
            if (typeSelect >= FFPropotion)
                randomSwapLUT(fpga, nets, bestHPWL);
            else
                randomSwapFF(fpga, nets, bestHPWL);
        }
        else {
            double typeSelect = (double) rand() / (RAND_MAX + 1.0);
            if (typeSelect >= FFPropotion)
                randomMoveLUT(fpga, nets, bestHPWL, 1);
            else
                randomMoveFF(fpga, nets, bestHPWL, 1);
        }
        //++detailIteration;
    }
    /*cout << "-----finish-----" << endl;
    cout << "globalIteration: " << globalIteration << endl;
    cout << "detailIteration: " << detailIteration << endl;
    cout << "betterSwap: " << betterSwap << endl;
    cout << "worseSwap: " << worseSwap << endl;
    cout << "betterMove: " << betterMove << endl;
    cout << "worseMove: " << worseMove << endl;
    cout << "failMove: " << failMove << endl;
    cout << "-----result-----" << endl;*/
    cout << "bestHPWL: " << bestHPWL << endl;
    //cout << "final HPWL: " << computeHPWL(fpga, nets) << endl;
/*-------------------------------------------------------------------------------------------------------------*/
    for (int i = 0; i < fpga.num_Inst.first; ++i) {
        output_file << fpga.LUTs[i].name << " " << fpga.LUTs[i].coord.first <<  " " << fpga.LUTs[i].coord.second << "\n";
    }
    for (int i = 0; i < fpga.num_Inst.second; ++i) {
        output_file << fpga.FFs[i].name << " " << fpga.FFs[i].coord.first << " " << fpga.FFs[i].coord.second << "\n";
    }
    output_file.close();
/*-------------------------------------------------------------------------------------------------------------*/
    return 0;
}