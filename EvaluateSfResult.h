//
// Created by zhangqianyi on 2017/3/31.
//

#ifndef EVALUATERESULT_H
#define EVALUATERESULT_H

#include <iostream>
#include <vector>
#include <set>
#include <cmath>
#include <string>
#include <algorithm>
#include <functional>
#include "ScoreFollowing.h"

using namespace std;

/**
 * 结构体存放演奏正确性
 * fullplayed表示当前位置的那些音符是否全被演奏了，如果是0表示不多不少，正好完全演奏
 * 如果是-1表示有漏，如果是1表示有多
 * excess中存放漏了或者多了的音符，omission中存放漏了的音符
 * mark中存放应该标哪些音符，如果fullplayed为0和为1都要全标上，如果为-1只标演奏的那几个音符
 */
struct Correctness {
    int jumpback;               // 是否回弹
    vector<int> excess;         // 多弹了的音符
    vector<int> omission;       // 少弹了的音符
    vector<int> intersection;   // 正确弹了的音符
};

struct BeatRhythm {
    int progress; // 进度快慢，为1表示快了，0表示正确，-1表示慢了
    double start; // 小节开始时间
    double end;   // 小节结束时间
    double during; // 小节持续时间，结束减去开始
	int beatnum;   //第几小节
};

class EvaluateSfResult {
public:
	EvaluateSfResult(ScoreFollowing& scoreFollowing1);

	/**
	* @author zhangqianyi
	* @date  2017/6/5
	* @brief 初始化函数，提取乐谱跟踪结果和乐谱信息数据
	*/
	void Init();
    /**
     * @author zhangqianyi
     * @date  2017/6/5
     * @return 返回Correctness数组表示乐谱跟踪结果sfResult每一个定位的演奏正确性
     * @brief 演奏正确性评价函数
     */
    vector<Correctness> EvaluateCorrectness();

	vector<Correctness> EvaluateCorrectnessModify();

    /**
     * @author zhangqianyi
     * @date  2017/6/27
     * @return 不忽略倍频错误，显示出回弹，重弹，跳弹
     * @brief 演奏正确性原始评价函数，能够正确显示回弹、重弹、跳弹，多弹漏弹了哪些音符
     */
    vector<Correctness> EvaluateCorrectnessOrigin(int maxPitchesInFrame);

    /**
     * @author zhangqianyi
     * @date  2017/6/6
     * @return 返回数组表示每小节是弹快了还是弹慢了
     * @brief 以小结为单位评价演奏节奏快慢，计算每一小节的演奏时长，求平均时长，若某一小节演奏时长与平均时长差异超过一定范围，则标记为快了或慢了
     */
    vector<BeatRhythm> EvaluateBeatRhythm();

    /**
     * @author zhangqianyi
     * @date  2017/6/26
     * @return 返回数组记录之前演奏的所有小节弹快了还是弹慢了
     * @brief 实时返回小节快慢演奏评价
     */
	vector<BeatRhythm> EvaluateBeatRhythmRealtime(int newLocation, double onset, int& newBeatIndex, vector<vector<int>> &PitchesPair);

    /**
     * @author zhangqianyi
     * @date  2017/6/6
     * @return 返回数组表示乐谱跟踪结果sfResult每一个定位的演奏节奏
     * @brief 以乐符为单位评价演奏节奏快慢，计算每两个位置之间的onset差值，与乐谱中的两个位置之间的差值作比较，看是否成比例
     */
    vector<int> EvaluateNoteRhythm();

    /**
     * @author zhangqianyi
     * @date  2017/6/16
     * @return 返回评价之后的分数
     * @brief 根据节奏评价、正确性评价的结果，给出一个分数
     */
    double CountScore(vector<Correctness> correctness, vector<BeatRhythm> beatRhythm);

    /**
     * @author zhangqianyi
     * @date  2017/6/27
     * @return 返回得到的星星数
     * @brief 根据评价之后的分数打星，0到5颗星
     */
    int GiveStars(double score);

    /**
     * @author zhangqianyi
     * @date  2017/6/19
     * @param filePath    文件路径
     * @return
     * @brief 保存正确性和小节级别节奏评价结果到文件中
     */
    int SaveEvaluateResult(const string& filePath, vector<Correctness> correctness, vector<BeatRhythm> beatRhythm);

private:

   
    /**
     * @author zhangqianyi
     * @date  2017/6/5
     * @param i             乐谱跟踪的位置
     * @return 返回当前位置是否回弹
     * @brief 判断当前位置是否回弹了
     */
    bool CheckJumpBack(int i);

    ScoreFollowing& scoreFollowing;                  // 乐谱跟踪类对象，组合形式传入

    vector<vector<vector<double>>> sfResult;        // 乐谱跟踪结果
    vector<vector<vector<double>>> scoreEvent;      // 乐谱信息数据

    vector<int> barFirst;                           // 节首位置
    vector<int> barEnd;                             // 节尾位置

    double beatToleranceRate;                       // 计算小节级别时间长短时，容许误差时间为平均时间乘上一个系数
    double beatMaxDiffTime;                         // 统计每个小节与其他几个小节的时间差距超过2s，如果有超过总小节数一半的数量，说明这个小节与其他小节时间有差距，不应该把他计算到平均时长中去

    double noteToleranceRate;                       // 计算音符级别评价时，容许误差倍率为平均倍率乘上一个系数

    double beatAvgTime;                             // 小节平均时长
    double beatToleranceTime;                       // 小节级别时间容许误差

    int realtimeBeatIndex;                          // 实时定位结果在第几小节
    vector<BeatRhythm> realtimeBeatRhythm;          // 实时的小节节奏评价
	vector<Correctness> RealtotalCorrectness;           //实时的小节正确性评价

	vector<int> sfResultLocate;

	map<int, int> barPair;
	map<int, vector<int>> MulitiFrq;

	vector<vector<vector<double>>> timePitchesPair;
};

#endif //EVALUATERESULT_H
