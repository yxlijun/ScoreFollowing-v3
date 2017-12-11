//
// Created by zhangqianyi on 2017/5/17.
//

#ifndef SCOREFOLLOWING_H
#define SCOREFOLLOWING_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <map>

#include "findPath.h"
#include "readData.h"
#include "scoreFollowingEvent.h"
#include "ProcessNMF.h"

using namespace std;

class ScoreFollowing {
public:
    ScoreFollowing();
    ~ScoreFollowing();

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @brief 初始化函数，定义类对象时初始化一些成员变量
     * @return 返回值表示执行是否成功，返回0表示正常执行，返回-1表示出错(读取文件失败)
     */
    int Init(string scoreEventPath);

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @brief 重置对象的成员变量，当停止录音，然后要重新开始录音的时候调用
     */
    void Reset();

	void Clear();
	void SetFinger(int flag);
    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @param H
     * @param timeResolution
     * @brief 离线测试用乐谱跟踪函数，输入NMF的输出结果：完整的H，执行乐谱跟踪算法
     */
	void ScoreFollowingOffline(const vector<vector<double> > &H, vector<double> & error,double timeResolution, int maxPitchesInEvent);

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @return
     * @brief 通过解析乐谱，得到乐谱中音符范围（最低音符-2 到 最高音符+2）
     */
    vector<int> GetNoteInScore();

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @return
     * @brief 通过解析乐谱，得到乐谱中一帧中出现的最多的音符
     */
    int GetMaxPitchesInFrame();

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @return
     * @brief 返回乐谱跟踪结果
     */
    vector<vector<vector<double>>> GetSfResult();
	
	/**
     * @author zhangqianyi
     * @date  2017/5/17
     * @return
     * @brief 
     */
    void SetSfResultPair(vector<vector<double> > timePitchPair);

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @return
     * @brief 返回原始乐谱跟踪结果，实时给出的
     */
    vector<vector<vector<double>>> GetSfResultOrigin();

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @return
     * @brief 返回乐谱信息数据
     */
    vector<vector<vector<double>>> GetScoreEvent();

    /**
     * @author zhangqianyi
     * @date  2017/5/17
     * @param newPitches
     * @param iFrame
     * @brief 乐谱跟踪更新定位函数，输入为新演奏的音符和帧号
     */
    void UpdateEvent(vector<int> newPitches, int iFrame);

    /**
     * @author zhangqianyi
     * @date  2017/6/6
     * @return 返回乐谱的结尾位置
     */
    int EndOfScore();

    /**
     * @author zhangqianyi
     * @date  2017/6/6
     * @return 乐谱跟踪是否定位到了乐谱结尾
     * @brief 判断到达结尾条件：1. 定位的最后一个是乐谱的长度this->EndOfScore()
     * 2. 定位的最后k个距离乐谱结尾比较近
     * 同时要满足后面一段时间没有检测到音符
     */
	/*
	* @author lijun
	* @date 2017/7/26
	* param pitches  新产生的音符
	* param location  上一次定位
	* brief 在新产生的音符进行定位前进行滤除倍频的操作，若新音符在乐谱中没有出现，且是某个音符的倍频
      则可以删掉，若新音符本身就是倍频关系，则在乐谱中查看上一次定位前后几个定位附近是否有同一个位置音符是倍频关系，
	  若没有，则查看前后两个定位是否出现该音符，将未出现的倍频音符删掉。
	*/
	void MinusDoubleFreq(vector<int> &pitches, int location);

	/*
	* @author lijun
	* @date 2017/8/2
	* brief 设置滤除倍频后所有的音符集合
	*/
	void SetPitchesPair(vector<vector<int>> pitchesPair);

	/*
	* @author lijun
	* @date  2017/8/2
	* brief  获取滤除倍频后的所有音符集合
	*/
	vector<vector<int>> GetPitchesPair();

	int GetStopFrame(vector<double> H);

    bool CheckLocatingEnd();


	vector<int> GetSfResultLocate();

	vector<int> GetSfResultOriginLocate();

    ProcessNMF processNMF;                              // 处理NMF类的对象，使用组合形式，在类中定义另一个类的对象

	vector<double> IEventTime;                    //所有演奏小节的时长

	vector<vector<vector<double>>> GetscoreEventOrigin();

	map<int, int> GetBarNum();                  //获取对应小节数

	bool ScoreEventFinish();

	bool NearScoreEventFinish();

	bool SetStopFrame();

	void MinusPeopleNoise(vector<int> &pitches, int location);  //滤除人声函数

	map<int, vector<int>> GetMultiFreq();

	vector<vector<vector<double>>> GettimePitchesPair();

	vector<double> GetRhyTime();                // 获取小节节拍时长


private:
    /**
     * 下面是一些类的成员变量，在乐谱跟踪算法中使用
     */

    vector<vector<vector<double> > > scoreEvent;        // 乐谱信息
    int iEventPre;                                      // 上一个位置的定位，当为-1时表示不确定定位
    Candidate candidate;                                // 定位候选数据
    vector<int> isPlayed;                               // 上一定位对应的音符是否被演奏
    int isSureFlag;                                     // 是否确定定位
    vector<vector<vector<double> > > sfResult;          // 乐谱跟踪结果
    vector<vector<vector<double> > > sfResultOrigin;    // 原始的乐谱跟踪结果，实时给出的定位

	vector<vector<int> > PitchesPair;            // 修正后去除倍频的音符

	vector<int> scorepPitch;                       //乐谱所有音符
	vector<int> scoreOctive;                      //乐谱所有倍频

	map<int, int> scoreLocate;                     //对于反复时定位位置和实际乐谱中的位置的映射
	vector<int> sfResultLocate;                    //反复过程中的定位
	vector<int> sfResultOriginLocate;               //反复过程中的实时定位

	map<int, int> BarNum;                          //反复过程中小节与实际乐谱小节的映射

	vector<double> TotalmaxH;

	map<int, vector<int>> MulitiFrq;
	vector<vector<vector<double>>> timeHPeakPair;

	vector<double> RhyTime;           ///小节节拍时长

};


#endif //SCOREFOLLOWING_H
