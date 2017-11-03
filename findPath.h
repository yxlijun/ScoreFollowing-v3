//
// Created by zhangqianyi on 2017/3/16.
//

#ifndef FINDPATH_H
#define FINDPATH_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include "dynamicProgramming.h"

using namespace std;

// 声明全局变量
extern const int g_Pitches; // scoreEvent乐谱信息中pitches在第几行
extern const int g_PitchesOctave; // scoreEvent乐谱信息中octave在第几行
extern const int g_BarFirst; // 节首

/**
 * @author zhangqianyi
 * @date  2017/6/6
 * @param iEventLastSure        上一确定的定位
 * @param path                  完全匹配的路径
 * @param pitches               新演奏的音符
 * @param matching              与乐谱中各位置的匹配程度
 * @brief candidate结构体不确定定位时，定位候选信息
 */
struct Candidate {
    int iEventLastSure = 0;
    vector<vector<int> > path;
    vector<vector<int> > pitches;
    vector<vector<double> > matching;
};

/**
 * @author zhangqianyi
 * @date  2017/6/6
 * @param candidate             候选信息
 * @param pathEnd               当前演奏的确定定位（相对值，候选值序号）
 * @return iEvent               当前演奏的定位
 * @brief 当前演奏确定定位为pathEnd，对于此前未确定定位的演奏，计算匹配程度最高的路径
 */
vector<vector<int> > findPath(const Candidate& candidate, int pathEnd, int barFirst);

/**
 * @author zhangqianyi
 * @date  2017/6/6
 * @param iEvent
 * @param candidate
 * @param barFirst
 * @brief 后处理函数来选择一条最终的路径，选择策略是选取一条不同位置最多的路径
 */
void postProcessingIEvent(vector<vector<int> >& iEvent, const Candidate& candidate, int barFirst);

/**
 * @author zhangqianyi
 * @date  2017/6/6
 * @param path                  此前未确定定位的演奏与乐谱完全匹配的路径
 * @param newNode               新演奏的音符与乐谱完全匹配的可能的位置
 * @brief 记录下这一段区间的匹配程度中完全匹配的地方，然后用完全匹配（上述匹配程度计算函数返回值为1）的位置更新路径
 * 将完全匹配的位置存放到路径中，下一次调用函数更新路径的时候，判断新的完全匹配位置newNode是否在，
 * 上次结果这个路径的尾pathEnd，pathEnd+1，pathEnd+2这三个位置中（不考虑回弹，考虑跳一个位置）
 * 如果在里面就说明这个位置是连续的，将它加到路径中，如果不是的话不更新路径。
 */
void updatePath(vector<vector<int> >& path, vector<int> newNode);

/**
 * @author zhangqianyi
 * @date  2017/6/6
 * @param candidate             候选信息
 * @param minNMatch             确定定位时，完全匹配的unique event数>=minNMatch
 * @param isSureFlag            当前演奏的定位是否确定
 * @param iEventPre             上一次的定位
 * @return iEvent               当前演奏的定位
 * @brief isSure 已知未确定定位的演奏 及其与乐谱完全匹配的路径，是否能确定当前演奏的位置
 */
vector<vector<int> > isSure(const Candidate& candidate, int minNMatch, int& isSureFlag, int iEventPre, int barFirst, vector<int> &sfResultOrigin);

/**
 * @author zhangqianyi
 * @date  2017/6/6
 * @param arr                   二维数组，函数运行完之后被修改成去重之后的数组
 * @return indexArr             返回二维数组arr中第一次出现的行的索引
 * @brief matlab unique函数，输入二维数组，对二维数组每一行求unique，最后返回unique的索引
 */
vector<int> matlabUnique(vector<vector<int> >& arr);

/*
 * @author  lijun
 * @date   2017/8/3
 *param   IEvent    通过动态规划后返回的路径
 *param   candidate  候选信息
 *param  sfResultOrigin  实时定位保存的信息
 *param  IEventPre      上一次定位
 *param  barFirst    节首
 *@brief   若当前路径中第一个位置小于上一次定位，且当前位置或下一个位置匹配程度为1,则修改IEvent

*/
void ProcessEvent(vector<vector<int>> &IEvent, Candidate& candidate, vector<int> &sfResultOrigin, int IEventPre, int barFirst,int iEventLastSure);
#endif //FINDPATH_H
