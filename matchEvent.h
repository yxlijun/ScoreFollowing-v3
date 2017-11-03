//
// Created by zhangqianyi on 2017/3/16.
//

#ifndef MATCHEVENT_H
#define MATCHEVENT_H

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "findPath.h"

using namespace std;

/**
 * @author zhangqianyi
 * @date  2017/6/6
 * @param pitch    新演奏的音符
 * @param scoreEvent            乐谱的位置信息
 * @param iEventPre             前面的event
 * @param lia                   是否在乐谱中
 * @param locb                  在乐谱中第几个位置
 * @return                      位置
 * @brief 乐谱的第iEventPre位置上有一个或多个音符被演奏，判断当前新演奏的音符pitch是否全部存在于乐谱中iEventPre位置的音频中
 * 比如乐谱第3个位置演奏了 35 37 39（乐谱中同一位置演奏的音符没有时间先后顺序，我们将它由小到大排序
 * 现在新检测到了音符35 39，表示新演奏音符存在于乐谱iEventPre位置
 * 此时lia表示是否存在，结果为[1,0,1]，locb表示存在的位置，结果为[0,0,2]（索引从0开始）
 */
int isIEvent(vector<int> pitch, const vector<vector<vector<double>>>& scoreEvent, int iEventPre, vector<int>& lia, vector<int>& locb);
/**
* @author lijun
* @date  2017/8/1
* @param pitch    新演奏的音符
* @param scoreEvent            乐谱的位置信息
* @param iEventPre             前面的event
* @param lia                   是否在乐谱中
* @param locb                  在乐谱中第几个位置
* @return                      位置
* @brief 判断新演奏音符是否和下一个位置完全匹配，此处的完全匹配
* 是和乐谱中的音符完全相同，音符个数也相同
*/

int isIEventTotal(vector<int> pitch, const vector<vector<vector<double>>>& scoreEvent, int iEventPre, vector<int>& lia, vector<int>& locb);
/**
 * @author zhangqianyi
 * @date  2017/6/6
 * @param pitch                 新演奏的音符
 * @param scoreEvent            乐谱
 * @param iEvent                位置
 * @return                      匹配程度
 * @brief 遍历每个音符，计算每个音符与iEvent位置的每个音符的差值，保存最小差值
 * 然后通过distanceToMatching函数将音符差值转换成为匹配程度。然后在计算倍频匹配程度，将两个匹配程度做个权重，得到总体的匹配程度
 */
double matchIEvent(vector<int> pitch, vector<vector<vector<double> > > scoreEvent, int iEvent);

/**
 * @author zhangqianyi
 * @date  2017/6/6
 * @param distance              距离
 * @return                      匹配程度
 * @brief 将音符差值转换成为匹配程度
 */
double distanceToMatching(int distance);

/**
 *
 * @param performancePitches    新演奏的音符
 * @param performanceOctave     忽略倍频时新演奏的音符
 * @param scoreEvent            乐谱
 * @param beginIEvent
 * @param matching
 * @param path
 * @brief 计算和乐曲整首曲子 的匹配程度
 */
void matchAllCandidate(vector<int> pitch, vector<vector<vector<double> > > scoreEvent, int beginIEvent,
                        vector<vector<double> >& matching, vector<vector<int> >& path);

void matchScore(vector<vector<int> > pitches, vector<vector<vector<double> > > scoreEvent, vector<vector<double> >& matching, vector<vector<int> >& path);

void traceback(int pathEnd, vector<vector<double> > matching, int iCol,  vector<vector<int> >& path);


#endif //MATCHEVENT_H
