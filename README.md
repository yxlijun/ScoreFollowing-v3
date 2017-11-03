# README

乐谱跟踪算法

## 文件目录
Docs文件目录下存放开发文档、一些错误情况分析等
Data文件目录下存放测试数据，Data/scoreEvent中存放乐谱数据

## 算法原理：
考虑当前一直正确演奏，音调检测结果一直与乐谱中的音符对应（完全匹配）。
当下一个位置演奏的音调检测结果与乐谱中下一个位置的音符不对应了，此时需要去在乐谱中定位下一个演奏的位置。
计算下一个演奏的音符与全曲（整张乐谱搜索）中每个位置的匹配程度，但是此时的信息量不够，不足以确定定位。
等到下下个演奏的音符，我们也计算演奏的音符与全曲``的匹配程度，一直扩展到多个位置，直到我们能够确定连续演奏在乐谱中只有一个位置与演奏的音符相同。
此时对于后面的几个连续演奏的位置我们能够准确的确定了，对于前面的不能够确定的定位。
使用DTW算法，我们知道开始的不对应的位置，以及知道最后的确定定位的位置，知道起点和终点，计算一个匹配程度最高的路径。
对于路径上的每一个点就是之前不确定定位的点的在乐谱中的位置。

## 算法接口
接口的使用：
开始定义变量传入和传出给下一次调用使用
```
// 在遍历所有帧的外面设置这些变量
// 读入scoreEvent文件
string scoreEventFilePath = "you path to file";
vector<vector<vector<double> > > scoreEvent = getScoreEvent(scoreEventFilePath);
vector<int> sameOnset;
int iEventPre = 0;
Candidate candidate;
vector<int> isPlayed(3, 0);
int isSureFlag = 1;

// 整个乐谱跟踪结果，在调用完一次算法之后将结果添加到这个后面
vector<vector<vector<double> > > sfResult(3);

// 遍历所有帧
for (int iFrame = ....) {
    // 调用算法
    vector<vector<vector<double> > > sfResultTemp = scoreFollowingEventInterface(scoreEvent, nFrameCount, sameOnset, iEventPre, candidate, isPlayed, timeResolution, iFrame, isSureFlag);
    // 将一次结果添加到后面
    for (unsigned int i = 0; i < sfResultTemp.size(); ++i) {
        for (unsigned int j = 0; j < sfResultTemp[i].size(); ++j) {
            sfResult[i][j].push_back(sfResultTemp[i][j]);
        }
    }
}

```
对于多音调检测结果的每一帧iFrame，累加计算得到nFrameCount，当到达minDurFrame(默认为7)清零
每一帧的时间间隔为timeResolution

```
/**
 * @brief 算法接口，处理每一帧数据，实时给出乐谱跟踪结果
 * @param scoreEvent            标准乐谱信息
 * @param nFrameCount           计算当前pitch被演奏了多少帧，在函数中设置minDurFrame为7，表示当连续检测到7帧时算新检测到音符
 * @param sameOnset             待验证最短时长约束的帧序号，从小到大排序
 * @param iEventPre             上一个位置的定位，当为-1时表示不确定定位
 * @param candidate             候选信息
 * @param isPlayed              上一定位对应的音符是否被演奏
 * @param timeResolution        每帧的时间间隔
 * @param iFrame                当前处理帧的序号，当序号为-1时表示结束
 * @param isSureFlag            上一个位置是否确定
 * @return                      返回sfResult，第一行表示新演奏音符以及音符对应开始时间，第二行当前定位是否确定，第三行表示定位
 */
vector<vector<vector<double> > > scoreFollowingEventInterface(vector<vector<vector<double> > > scoreEvent, vector<int> nFrameCount,
                                                              vector<int>& sameOnset, int& iEventPre, Candidate& candidate,
                                                              vector<int>& isPlayed, double timeResolution, int iFrame, int& isSureFlag);
```


## 测试数据


## 分割音频输出多次演奏
现在需求是
用户选取一首曲子，按下"开始演奏"按钮之后开始演奏，演奏完一次中间停顿一下，接着演奏，直至用户按下"结束演奏"按钮
在用户结束演奏之后，对这一段连续录音音频分析，找到这一段音频中完整演奏的那几段音频截取出来

分析：
对乐谱跟踪结果分析定位到乐谱头部表示一段完整演奏的开始，定位到乐谱尾部表示一段完整演奏的结束。


### 判断是否是头、尾
可以添加两个判断条件：
1. 对于尾来说，不能直接判断一个定位结果是否为尾来确定尾的位置，这样误差太大。借助之前的辅助信息。
当乐谱跟踪的一串连续输出结果都比较接近乐谱的尾部时，并且在这一串结果后面突然定位消失（用户停止演奏，没有多音调检测输出）
或者更改了定位（有其他杂音还是出现了定位）


### 对整段录音输出生成的乐谱跟踪输出进行离线复核纠错
由于现在是离线输出，可以对乐谱跟踪输出进行判断、复核，将乐谱跟踪结果尽可能修正为用户实际演奏的结果。