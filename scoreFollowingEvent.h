//
// Created by zhangqianyi on 2017/3/17.
//

#ifndef SCOREFOLLOWINGEVENT_H
#define SCOREFOLLOWINGEVENT_H

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cstring>

#include "matchEvent.h"
#include "findPath.h"
#include "readData.h"

using namespace std;

/**
 * @brief 处理每一帧数据，给出乐谱跟踪结果
 * @param pitch                 新演奏的音频
 * @param scoreEvent            乐谱信息
 * @param iEventPre             上一个定位
 * @param candidate             定位候选数据
 * @param isPlayed              上一定位对应的音符是否被演奏
 * @param isSureFlag            当前演奏的定位是否确定
 * @return iEvent               返回乐谱跟踪的位置
 */
vector<vector<int> > scoreFollowingEvent(vector<int>& pitch, vector<vector<vector<double> > >& scoreEvent, vector<int> &sfResultOriginLocate,
                                         int iEventPre, Candidate& candidate, vector<int>& isPlayed, int& isSureFlag);

#endif //SCOREFOLLOWINGEVENT_H
