//
// Created by zhangqianyi on 2017/5/23.
//

#include "ProcessNMF.h"

ProcessNMF::ProcessNMF() : minDurFrame(7)
{
    Init();
}

ProcessNMF::~ProcessNMF()
{

}

void ProcessNMF::Init()
{
    SetSplitFrameCountParams(0.4, 0.6, 0.2); // 将音符切割三个参数初始化为设定值
    SetThreshParams(0.25, 400); // 设置阈值参数
    SetRemovePitchParams(0.5, 0.5); // 设置删除延续音符参数

    nFrameCount = vector<int>(88, 0);
    xHPeak = vector<double>(88, 0);
    xHPre7Peak = vector<double>(88, 0);
    timeHPeakPair.resize(88);
    // 切分音符用，4行88列
    pitchMaxMin.resize(4);
    for (int i = 0; i < 4; ++i) pitchMaxMin[i].resize(88);
}

void ProcessNMF::Reset()
{
    nFrameCount = vector<int>(88, 0);
    xHPeak = vector<double>(88, 0);
    xHPre7Peak = vector<double>(88, 0);
    vector<vector<double>>().swap(timePitchesPair);
    vector<vector<vector<double>>>().swap(timeHPeakPair);
    timeHPeakPair.resize(88);
    // 切分音符用，4行88列
    pitchMaxMin.resize(4);
    for (int i = 0; i < 4; ++i) pitchMaxMin[i].resize(88);
}

bool ProcessNMF::UpdateEventFlag(vector<int> &pitches, int iFrame)
{
    bool flag = (!newPitches.empty() && sameOnset.empty()) || (iFrame == -1);
    if (flag) {
        pitches = newPitches;
    }
    return flag;
}

void ProcessNMF::ProcessingFrame(const vector<double>& xH, int iFrame, double timeResolution, int maxPitchesInEvent)
{
    UpdateFrameCount(xH, iFrame, timeResolution, maxPitchesInEvent);
    GenerateNewPitches(xH, iFrame);
}

void ProcessNMF::UpdateFrameCount(const vector<double> &xH, int iFrame, double timeResolution, int maxPitchesInEvent)
{
    // 得到当前帧的最大H值
    double maxH = xH[0];
    for (vector<int>::size_type i = 0; i < xH.size(); ++i) {
        if (xH[i] > maxH) {
            maxH = xH[i];
        }
    }
    // 对xH加阈值，得到pianoRoll
    // 阈值设定规则为当前帧最大H值乘以一个系数，或者是一个预定的最低阈值，取两者最大值
    double thisThresh = (maxH * xHThreshCoeff) > xHMinThresh ? (maxH * xHThreshCoeff) : xHMinThresh;

    // 限制一帧数据中出现的音符的个数，只取最大的前maxPitchesInEvent个，其余的pianoRoll置0
    int pianoRollSum = 0;
    for (vector<int>::size_type i = 0; i < xH.size(); ++i) {
        if (xH[i] > thisThresh) {
            ++pianoRollSum;
        }
    }
    vector<int> pianoRoll(xH.size(), 0);
    if (pianoRollSum > maxPitchesInEvent) {
        // 先对H[i]排序，然后将maxPitches后面的pianoRoll设置为0
        vector<double> temp(xH);
        sort(temp.begin(), temp.end(), greater<double>()); // 由大到小排序
        for (int index = 0; index < maxPitchesInEvent; ++index) {
            for (vector<int>::size_type i = 0; i < xH.size(); ++i) {
                if (xH[i] == temp[index]) {
                    pianoRoll[i] = 1;
                    break;
                }
            }
        }
    } else {
        for (vector<int>::size_type i = 0; i < xH.size(); ++i) {
            pianoRoll[i] = xH[i] > thisThresh;
        }
    }


    // 通过上面处理的pianoRoll来累加更新nFrameCount
    for (vector<int>::size_type i = 0; i < nFrameCount.size(); ++i) {
        if (pianoRoll[i] == 1) {
            // 检测到音符为i+1，检测到的次数加1
            nFrameCount[i] += 1;

            if (nFrameCount[i] == minDurFrame) {
                // 计数到达minDurFrame，新音符出现，存下新音符的onset、前7帧H峰值
                double onset = (iFrame - minDurFrame + 1)*timeResolution;
                timeHPeakPair[i].push_back(vector<double>{onset, xHPre7Peak[i]});
            }

            // 计数超过了minDurFrame的音符，表示这个音符仍然在计数，我们记录这个音的H峰值
            if (nFrameCount[i] > minDurFrame) {
                if (xHPeak[i] < xH[i]) {
                    xHPeak[i] = xH[i];
                }
            } else { // 记录前7帧的H峰值
                if (xHPre7Peak[i] <= xH[i]) {
                    xHPre7Peak[i] = xH[i];
                }
            }
        } else {
            // 如果之前计数超过了minDurFrame的音符，表示是一个音的结束位置
            if (nFrameCount[i] >=  minDurFrame) {
                // 记录结束时间offset和H的峰值
                double offset = iFrame * timeResolution;
                timeHPeakPair[i].back().push_back(offset);
                timeHPeakPair[i].back().push_back(xHPeak[i]);

                if (timeHPeakPair[i].back().size() > 4) {
                    // 说明当前音符是延音被删除了，更新上一个音符的offset时间
                    timeHPeakPair[i].back()[2] = timeHPeakPair[i].back()[4];
                    // 删除多出的两个
                    timeHPeakPair[i].back().erase(timeHPeakPair[i].back().end()-1);
                    timeHPeakPair[i].back().erase(timeHPeakPair[i].back().end()-1);
                }

                xHPeak[i] = 0; // 清零
                xHPre7Peak[i] = 0; // 清零
            }

            // 现在没有检测到这个音符，将检测到的次数清零
            nFrameCount[i] = 0;
        }
    }

    // 切分两个连续演奏的相同音符
    ClearFrameCount(nFrameCount, minDurFrame, xH, iFrame, timeResolution);
}

void ProcessNMF::GenerateNewPitches(const vector<double> &xH, int iFrame)
{
    // 检测新演奏的音符，连续检测到minDurFrame帧才算是一个完整的音符
    vector<int> thisNewPitches;
    // 减少nFrameCount比较次数
    if (sameOnset.empty() || (!sameOnset.empty() && sameOnset[0] == iFrame)) {
        for (vector<int>::size_type i = 0; i < nFrameCount.size(); ++i) {
            // 当nFrameCount计数等于minDurFrame，表示是新演奏的音符，音符为数组下标加1(数组从0存到87，音符从1到88)
            if (nFrameCount[i] == minDurFrame) {
                thisNewPitches.push_back(i + 1);
            }
        }
    } else {
        // 清空新演奏音符
        vector<int>().swap(thisNewPitches);
    }

    // 合并onset间隔小于最短时长约束的event，如果两个音符间隔很短，我们认为这两个音符是同时被演奏的
    // 策略：当音符出现次数为minDurFrame的时候，我们要看其他的音符出现次数为多少，如果其他音符也出现了，但是次数小于7
    // 我们要等到他变为7，这个时候才把这两个音符当作新演奏的音符
    // onset表示为音符开始的帧号，在nFrameCount中数值超过1的音符，他第一次出现的帧号记录下来
    if (sameOnset.empty()) { // 没有onset相同的音符
        if (!thisNewPitches.empty()) {
            // 将新演奏的音符存入到sfResult结果中，存放到第一行，存放格式为 onset和其对应的音符交替存放
//            vector<double> sfResultTemp;
//            for (vector<int>::size_type i = 0; i < thisNewPitches.size(); ++i) {
//                double onset = (iFrame - minDurFrame + 1) * timeResolution;
//                sfResultTemp.push_back(onset);
//                sfResultTemp.push_back(static_cast<double>(thisNewPitches[i]));
//            }
//            timePitchesPair.push_back(sfResultTemp);

            // 更新sameOnset信息，将出现次数为1到minDurFrame-1的这些音符（nFrameCount值为1到minDurFrame-1）的onset记录下来
            vector<int> newPitchesCandidate; // 出现次数为1到minDurFrame-1的音符
            for (vector<int>::size_type i = 0; i < nFrameCount.size(); ++i) {
                if (nFrameCount[i] > 0 && nFrameCount[i] < minDurFrame) {
                    newPitchesCandidate.push_back(i + 1); // 下标要加1，同141行注释
                }
            }
            if (!newPitchesCandidate.empty()) {
                vector<int> onsetTemp;
                for (vector<int>::size_type i = 0; i < newPitchesCandidate.size(); ++i) {
                    // 之前下标加了1，现在要把newPitchesCandidate当作下标来使用，要减1
                    onsetTemp.push_back(nFrameCount[newPitchesCandidate[i]-1]);
                    onsetTemp[i] = iFrame + minDurFrame - onsetTemp[i];
                }
                // 去掉重复的onset，三步操作，sort，unique，erase
                sort(onsetTemp.begin(), onsetTemp.end());
                vector<int>::iterator it = unique(onsetTemp.begin(), onsetTemp.end());
                onsetTemp.erase(it, onsetTemp.end());
                for (vector<int>::size_type i = 0; i < onsetTemp.size(); i++) {
                    sameOnset.push_back(onsetTemp[i]);
                }
            }
        }
        newPitches = thisNewPitches;
    } else { // 有onset相同的音符，要把他们当作同时演奏
        if (sameOnset[0] == iFrame) { // 最近的要到7次的帧号
            if (!thisNewPitches.empty()) {
                newPitches.insert(newPitches.end(), thisNewPitches.begin(), thisNewPitches.end());
                // 去掉重复的onset，三步操作，sort，unique，erase
                sort(newPitches.begin(), newPitches.end());
//                vector<int>::iterator it = unique(newPitches.begin(), newPitches.end());
//                newPitches.erase(it, newPitches.end());

//                // 存入新音符到sfResult中
//                for (unsigned i = 0; i < thisNewPitches.size(); ++i) {
//                    if (!timePitchesPair.empty()) {
//                        timePitchesPair[timePitchesPair.size() - 1].push_back((iFrame - minDurFrame + 1) * timeResolution);
//                        double thisNewPitch = thisNewPitches[i];
//                        timePitchesPair[timePitchesPair.size() - 1].push_back(thisNewPitch);
//                    }
//                    else {
//                        double thisNewPitch = thisNewPitches[i];
//                        timePitchesPair.push_back(vector<double>{(iFrame - minDurFrame + 1) * timeResolution, thisNewPitch});
//                    }
//                }
            }
            sameOnset.erase(sameOnset.begin());
        }
        if (any_of(nFrameCount.begin(), nFrameCount.end(), [](int x){return x==1;})) {
            sameOnset.push_back(iFrame + minDurFrame - 1);
        }
    }

    // 删除多检测的音符
    // 如果出现过了33，然后过了很短的时间0.06s之后又出现了33，并且后面的33的h峰值很小（虽然超过了阈值），不到前面的1/3
    // 我们认为后面的33是前面的33的延续，而不是一个新检测的音符
    for (vector<int>::iterator it = newPitches.begin(); it != newPitches.end(); ) {
        // 音符序号比索引值index大1，所以index为newPitches[i]-1
        int index = *it - 1;

        if (timeHPeakPair[index].size() > 1) { // 前面有这个音符出现
            double currOnset = timeHPeakPair[index].back()[0]; // 这次出现的onset
            double currHPre7Peak = timeHPeakPair[index].back()[1]; // 这次出现的前7帧H峰值
            double preOffset = timeHPeakPair[index][timeHPeakPair[index].size()-2][2]; // 上次出现的offset
            double preHPre7Peak = timeHPeakPair[index][timeHPeakPair[index].size()-2][1]; // 上次出现的H前7帧峰值
            double timeInterval = currOnset - preOffset; // 上次offset和这次onset的时间间隔
            // 如果间隔时间很短，并且前7帧峰值差距比较大，说明这次的音是上一次的延续，而不是新音符，要删除这个音符
            if (timeInterval < minInterval && preHPre7Peak * xHPeakCoeff > currHPre7Peak) {
                it = newPitches.erase(it); // 删除当前音符，不能把他当作新音符
                // 在timeHPeakPair中删除当前值，也就是timeHPeakPair[index]的最后一个元素timeHPeakPair[index].end()-1
                timeHPeakPair[index].erase(timeHPeakPair[index].end()-1);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    if (sameOnset.empty()) {
        // 存入新音符到sfResult中
        vector<double> pairTemp;
        vector<int> appearance(88, 0); // 当前音符出现次数，有可能有的音符出现了两次
        for (vector<int>::size_type i = 0; i < newPitches.size(); ++i) {
            double pitch = newPitches[i];
            ++appearance[newPitches[i] - 1]; // 出现一次索引向前移动一位
            int index = timeHPeakPair[newPitches[i] - 1].size() - appearance[newPitches[i]-1];
            double onset = timeHPeakPair[newPitches[i] - 1][index][0]; // 这次出现的onset
            pairTemp.push_back(onset);
            pairTemp.push_back(pitch);
        }
        if (!pairTemp.empty()) timePitchesPair.push_back(pairTemp);
    }
}

void ProcessNMF::SetSplitFrameCountParams(double k1, double k2, double k3)
{
    splitNFrameCountK1 = k1;
    splitNFrameCountK2 = k2;
    splitNFrameCountK3 = k3;
}

void ProcessNMF::SetThreshParams(double threshCoeff, double minThresh)
{
    xHThreshCoeff = threshCoeff;
    xHMinThresh = minThresh;
}

void ProcessNMF::SetRemovePitchParams(double interval, double hPeakCoeff)
{
    xHPeakCoeff = hPeakCoeff;
    minInterval = interval;
}

void ProcessNMF::ClearFrameCount(vector<int> &nFrameCount, int minDur, vector<double> xH, int iFrame, double timeResolution)
{
    for (vector<int>::size_type i = 0; i < nFrameCount.size(); ++i) {
        if (nFrameCount[i] >= minDur) {
            double nowH = xH[i];
            double maxC = pitchMaxMin[0][i]; // 判断最大值是否被赋值
            double minC = pitchMaxMin[2][i]; // 判断最小值是否被赋值
            if (maxC == 0) { // 最大值最小值都未被赋值，把当前值存入最大值
                pitchMaxMin[0][i] = nowH;
                pitchMaxMin[1][i] = iFrame;
            } else if (minC == 0) { // 已有最大值但无最小值时
                if (nowH > pitchMaxMin[0][i]) { // 出现更大的最大值，更新
                    pitchMaxMin[0][i] = nowH;
                    pitchMaxMin[1][i] = iFrame;
                } else if (nowH < pitchMaxMin[0][i] * splitNFrameCountK1) { // 出现符合条件的最小值，存入最小值
                    pitchMaxMin[2][i] = nowH;
                    pitchMaxMin[3][i] = iFrame;
                }
            } else { // 最大值最小值都已存在
                if (nowH < pitchMaxMin[2][i]) { // 出现更小的最小值，进行最小值更新
                    pitchMaxMin[2][i] = nowH;
                    pitchMaxMin[3][i] = iFrame;
                } else if ((nowH > pitchMaxMin[0][i] * splitNFrameCountK2) &&
                           (nowH - pitchMaxMin[2][i] > pitchMaxMin[0][i] * splitNFrameCountK3)) {
                    // 为了后续能检测出来，当前音的帧数需要变化
                    int diffFrame = iFrame - static_cast<int>(pitchMaxMin[3][i]);
                    // 如果超过了最低连续帧数，就把这个设置为当前帧数，在这一帧检测出来
                    // 没超过的时候，设置为帧数差。
                    if (diffFrame > minDur) {
                        nFrameCount[i] = minDur;
                    } else {
                        nFrameCount[i] = diffFrame;
                    }

                    // 添加offset值到timeHPeakPair中
                    // offset为极小值的帧数，就是pitchMaxMin[3][i]
                    timeHPeakPair[i].back().push_back(pitchMaxMin[3][i]*timeResolution);
                    // 存下H峰值，就是pitchMaxMin[0][i]
                    timeHPeakPair[i].back().push_back(pitchMaxMin[0][i]);
					if (diffFrame > minDur) {
						timeHPeakPair[i].push_back(vector<double>{pitchMaxMin[3][i] * timeResolution, nowH});
					}
                    // 存入最新的最大值并清零最小值
                    pitchMaxMin[0][i] = nowH;
                    pitchMaxMin[1][i] = iFrame;
                    pitchMaxMin[2][i] = 0;
                    pitchMaxMin[3][i] = 0;
                }
            }
        } else if (nFrameCount[i] == 0) { // 清零
            pitchMaxMin[0][i] = 0;
            pitchMaxMin[1][i] = 0;
            pitchMaxMin[2][i] = 0;
            pitchMaxMin[3][i] = 0;
        }
    }
}

vector<vector<double> > ProcessNMF::GetTimePitchesPair()
{
    return timePitchesPair;
}

vector<vector<vector<double> > > ProcessNMF::GetTimeHPeakPair()
{
    return timeHPeakPair;
}