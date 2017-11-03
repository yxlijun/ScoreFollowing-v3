//
// Created by zhangqianyi on 2017/3/16.
//

#ifndef DYNAMICPROGRAMMING_H
#define DYNAMICPROGRAMMING_H


#include <iostream>
#include <vector>
#include "matchEvent.h"

using namespace std;

/**
 * @param matching          匹配程度
 * @param pathEnd           寻找到尾部，当为-1时表示找到尾
 * @param maxD              匹配程度最高
 * @return iEvent           返回匹配程度最高的路径
 * @brief 动态规划寻找匹配程度最高的路径
 */
vector<vector<int> > dynamicProgramming(vector<vector<double> > matching, int pathEnd, double& maxD);

/**
 * @param pathStart
 * @param phi
 * @param iCol
 * @param nCol
 * @param iEvent
 * @return
 * @brief DTW从后向前寻找路径
 */
vector<vector<int> > traceback(int pathStart, vector<vector<vector<double> > > phi, int iCol, int nCol, vector<vector<int> > iEvent);

#endif //DYNAMICPROGRAMMING_H
