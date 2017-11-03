//
// Created by zhangqianyi on 2017/5/23.
//

#ifndef PROCESSNMF_H
#define PROCESSNMF_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

using namespace std;

class ProcessNMF {
public:
    ProcessNMF();
    ~ProcessNMF();

    /**
     * @author zhangqianyi
     * @date 2017/5/26
     * @param xH                    一帧多音调检测结果H，为大小为88的double型数组
     * @param iFrame                帧号，表示是第多少帧
     * @param timeResolution        时间分辨率，用来计算音符onset时间
     * @param maxPitchesInEvent     一个位置中的最多音符个数
     * @brief 连续处理多音调检测结果，不断更新输出新音符
     */
    void ProcessingFrame(const vector<double>& xH, int iFrame, double timeResolution, int maxPitchesInEvent);

    /**
     * @author zhangqianyi
     * @date 2017/5/23
     * @brief 重置函数
     */
    void Reset();

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @param k1
     * @param k2
     * @param k3
     * @brief 切分音符的参数设置，有时候连续演奏的音符是一样的，并且H值都比较大，没有低于阈值，所以会导致检测为一个连续的音符我们要将它切分开来
     */
    void SetSplitFrameCountParams(double k1, double k2, double k3);

    /**
     * @author zhangqianyi
     * @date  2017/5/23
     * @param threshCoeff       阈值系数
     * @param minThresh         最低阈值
     * @brief H阈值处理的参数设置，设置阈值才能更新nFrameCount
     */
    void SetThreshParams(double threshCoeff, double minThresh);

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @param interval		    上一次出现该音符的offset时间和这一次出现的onset时间最小间隔
     * @param hPeakCoeff			H前7帧的峰值之间的系数
     * @brief 删除延续音符的参数设置，有时候多检到延续的演奏，要将它删除
     */
    void SetRemovePitchParams(double interval, double hPeakCoeff);

    /**
     * @author zhangqianyi
     * @date  2017/5/26
     * @param iFrame            帧号，当为-1的时候表示最后一帧
     * @param pitches           如果允许调用乐谱跟踪算法，更新pitches值
     * @return 返回能否开始调用乐谱跟踪算法来更新结果。
     * @brief 通过判断音符是否合并完毕（sameOnset是否为空）来确定能否开始调用算法，如果可以调用算法，就将合并完了的新音符赋值给pitches
     */
    bool UpdateEventFlag(vector<int> &pitches, int iFrame);

    /**
     * @author zhangqianyi
     * @date  2017/5/26
     * @return
     * @brief 获取时间音符点对，作为sfResult的第一行
     */
    vector<vector<double> > GetTimePitchesPair();

    /**
     * @author zhangqianyi
     * @date  2017/5/26
     * @return
     * @brief 获取音符的offset时间H峰值点对
     */
    vector<vector<vector<double> > > GetTimeHPeakPair();


private:
    /**
     * @author zhangqianyi
     * @date 2017/5/23
     * @brief 初始化函数
     */
    void Init();

    /**
     * @author zhangqianyi
     * @date 2017/5/23
     * @param xH                一帧多音调检测结果H，为大小为88的double型数组
     * @param iFrame            帧号
     * @brief 连续处理NMF输出的结果H，更新每个音符连续出现了多少帧nFrameCount
     */
    void UpdateFrameCount(const vector<double> &xH, int iFrame, double timeResolution, int maxPitchesInEvent);

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @param nFrameCount       每个音符出现帧数计数器
     * @param minDur            最短出现多少帧我们认为是一个音符
     * @param xH                一帧多音调检测结果H，为大小为88的double型数组
     * @param iFrame            帧号
     * @brief 清空nFrameCount，用来切分两个连续演奏的相同音符，有时候连续演奏的音符是一样的，并且H值都比较大，没有低于阈值，所以会导致检测为一个连续的音符我们要将它切分开来
     */
    void ClearFrameCount(vector<int> &nFrameCount, int minDur, vector<double> xH, int iFrame, double timeResolution);

    /**
     * @author zhangqianyi
     * @date 2017/5/23
     * @param xH                一帧多音调检测结果H，为大小为88的double型数组
     * @param iFrame            帧号
     * @brief 根据ProcessingNMF处理NMF输出结果函数输出的nFrameCount，合并onset得到新演奏的音符
     */
    void GenerateNewPitches(const vector<double> &xH, int iFrame);

    /**
     * 下面是一些类的成员变量，在处理NMF中使用
     */

    vector<double> xHPeak;                              // xH的峰值，为了后面检测出新音符使用
    vector<double> xHPre7Peak;                          // xH前7帧的峰值，判断是否是一个新音符，还是上一个音符的延续
    vector<int> nFrameCount;                            // 音符被连续演奏次数

    const int minDurFrame;                              // 最短时长约束，设置nFrameCount超过minDurFrame这么多次才算一个完整音符

    vector<int> sameOnset;                              // 合并onset，为了计算出新音符
    vector<int> newPitches;                             // 合并onset之后的新演奏的音符

    vector<vector<double>> pitchMaxMin;                 // 切分音符用，4行88列
    double splitNFrameCountK1;                          // 切分音符用的系数
    double splitNFrameCountK2;
    double splitNFrameCountK3;
    double xHThreshCoeff;                               // 求阈值时的最大xH系数
    double xHMinThresh;                                 // xH的最低阈值

    double minInterval;									// 上一次出现该音符的offset时间和这一次出现的onset时间最小间隔
    double xHPeakCoeff;									// H前7帧的峰值之间的系数

    vector<vector<double> > timePitchesPair;            // 新演奏音符和onset开始时间点对，作为sfResult的第一行
    vector<vector<vector<double> > > timeHPeakPair;     // 演奏的音符offset时间和H峰值的点对
};


#endif //PROCESSNMF_H
