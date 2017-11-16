//
// Created by zhangqianyi on 2017/5/17.
//

#include "ScoreFollowing.h"
#include "EvaluateSfResult.h"

ScoreFollowing::ScoreFollowing()
{
}

ScoreFollowing::~ScoreFollowing()
{
}

int ScoreFollowing::Init(string scoreEventPath)
{
	// 读取乐谱
    if (ReadScoreEvent(scoreEventPath, scoreEvent) == -1) {
        cout << "Read score event error!" << endl;
        return -1;
    }
	SetFinger(2);
    // 最终输出的定位结果为3行
    sfResult.resize(3);
    sfResultOrigin.resize(3);
    iEventPre = 0; // 初始化上一个定位为0
    isSureFlag = 1; // 初始化确定定位
    // 找到乐谱中一个位置最多演奏的音符个数maxIsPlayedTmp，即为isPlayed的长度
    vector<int> isPlayedTmp;
    for (vector<int>::size_type i = 0; i < scoreEvent.size(); ++i) {
        isPlayedTmp.push_back(static_cast<int>(scoreEvent[i][g_Pitches].size()));
    }
    unsigned int maxIsPlayedTmp = static_cast<unsigned int>(*max_element(isPlayedTmp.begin(), isPlayedTmp.end()));
    isPlayed = vector<int>(maxIsPlayedTmp, 0);
    candidate.iEventLastSure = 0;     // 定位候选信息初始化，其余的为vector，不用初始化
	Clear();
    return 0;
}

void ScoreFollowing::Clear(){
	processNMF.Reset();
	vector<vector<vector<double>>>().swap(sfResult);
	sfResult.resize(3);
	vector<vector<vector<double>>>().swap(sfResultOrigin);
	sfResultOrigin.resize(3);
	iEventPre = 0; // 上一个定位恢复为0
	isSureFlag = 1; // 恢复确定定位
	// 清零isPlayed
	for (vector<int>::size_type i = 0; i < isPlayed.size(); ++i) isPlayed[i] = 0;
	// 清空定位候选信息
	candidate.iEventLastSure = 0;
	vector<vector<double> >().swap(candidate.matching);
	vector<vector<int> >().swap(candidate.path);
	vector<vector<int> >().swap(candidate.pitches);
	vector<vector<int>>().swap(PitchesPair);
	vector<int>().swap(scorepPitch);
	vector<int>().swap(scoreOctive);
	vector<double>().swap(IEventTime);
	map<int, int>().swap(scoreLocate);
	vector<int>().swap(sfResultLocate);
	vector<int>().swap(sfResultOriginLocate);
	vector<double>().swap(TotalmaxH);
	map<int, vector<int>>().swap(MulitiFrq);
	vector<double>().swap(RhyTime);
	Reset();
}

void ScoreFollowing::Reset()
{
	for (vector<int>::size_type i = 0; i < scoreEvent.size(); i++){
		scoreLocate.insert(pair<int, int>(i + 1, static_cast<int>(scoreEvent[i][6][0])));
	}
	for (vector<int>::size_type i = 0; i <scoreEvent.size(); i++){
		for (vector<int>::size_type j = 0; j < scoreEvent[i][g_Pitches].size(); j++)
			scorepPitch.push_back(scoreEvent[i][g_Pitches][j]);
		for (vector<int>::size_type j = 0; j < scoreEvent[i][4].size(); j++){
			scoreOctive.push_back(scoreEvent[i][4][j]);
		}
	}
	int multifrq[76][26] = {
		{ 13, 20, 25, 29, 32, 37, 39, 41, 44, 49, 50, 51, 52, 56, 58, 61, 62, 63, 64, 68, 71, 73, 83, 85 },
		{ 14, 21, 26, 30, 33, 38, 40, 42, 45, 50, 51, 52, 53, 57, 59, 62, 63, 64, 65, 69, 72, 74, 81, 84, 86 },
		{ 15, 22, 27, 31, 34, 39, 41, 43, 46, 51, 52, 53, 54, 58, 60, 63, 64, 65, 66, 70, 73, 75, 85, 87 },
		{ 16, 23, 28, 32, 35, 40, 42, 44, 47, 52, 53, 54, 55, 59, 61, 64, 65, 66, 67, 71, 74, 76, 86, 88 },
		{17, 24, 29, 33, 36, 41, 43, 45, 48, 53, 54, 55, 56, 60, 62, 65, 66, 67, 68, 72, 75, 77, 87 },
		{ 18, 25, 30, 34, 37, 42, 44, 46, 49, 54, 55, 56, 57, 61, 63, 66, 67, 68, 69, 73, 76, 78, 88 },
		{ 19, 26, 31, 35, 38, 43, 45, 47, 50, 55, 56, 57, 58, 62, 64, 67, 68, 69, 70, 74, 77, 79 },
		{ 20, 27, 32, 36, 39, 44, 46, 48, 51, 56, 57, 58, 59, 63, 65, 68, 69, 70, 71, 75, 78, 80 },
		{ 21, 28, 33, 37, 40, 45, 47, 49, 52, 57, 58, 59, 60, 64, 66, 69, 70, 71, 72, 76, 79, 81 },
		{ 22, 29, 34, 38, 41, 46, 48, 50, 53, 58, 59, 60, 61, 65, 67, 70, 71, 72, 73, 77, 80, 82 },
		{ 23, 30, 35, 39, 42, 47, 49, 51, 54, 59, 60, 61, 62, 66, 68, 71, 72, 73, 74, 78, 81, 83 },
		{ 24, 31, 36, 40, 43, 48, 50, 52, 55, 60, 61, 62, 63, 67, 69, 72, 73, 74, 75, 79, 82, 84 },
		{ 25, 32, 37, 41, 44, 49, 51, 53, 56, 61, 62, 63, 64, 68, 70, 73, 74, 75, 76, 80, 83, 85 },
		{ 26, 33, 38, 42, 45, 50, 52, 54, 57, 62, 63, 64, 65, 69, 71, 74, 75, 76, 77, 81, 84, 86 },
		{ 27, 34, 39, 43, 46, 51, 53, 55, 58, 63, 64, 65, 66, 70, 72, 75, 76, 77, 78, 82, 85, 87 },
		{ 28, 35, 40, 44, 47, 52, 54, 56, 59, 64, 65, 66, 67, 71, 73, 76, 77, 78, 79, 83, 86, 88 },
		{ 29, 36, 41, 45, 48, 53, 55, 57, 60, 65, 66, 67, 68, 72, 74, 77, 78, 79, 80, 84, 87 },
		{ 30, 37, 42, 46, 49, 54, 56, 58, 61, 66, 67, 68, 69, 73, 75, 78, 79, 80, 81, 85, 88 },
		{ 31, 38, 43, 47, 50, 55, 57, 59, 62, 67, 68, 69, 70, 74, 76, 79, 80, 81, 82, 86 },
		{ 32, 39, 44, 48, 51, 56, 58, 60, 63, 68, 69, 70, 71, 75, 77, 80, 81, 82, 83, 87 },
		{ 33, 40, 45, 49, 52, 57, 59, 61, 64, 69, 70, 71, 72, 76, 78, 81, 82, 83, 84, 88 },
		{ 34, 41, 46, 50, 53, 58, 60, 62, 65, 70, 71, 72, 73, 77, 79, 82, 83, 84, 85 },
		{ 35, 42, 47, 51, 54, 59, 61, 63, 66, 71, 72, 73, 74, 78, 80, 83, 84, 85, 86 },
		{ 36, 43, 48, 52, 55, 60, 62, 64, 67, 72, 73, 74, 75, 79, 81, 84, 85, 86, 87 },
		{ 37, 44, 49, 53, 56, 61, 63, 65, 68, 73, 74, 75, 76, 80, 82, 85, 86, 87, 88 },
		{ 38, 45, 50, 54, 57, 62, 64, 66, 69, 74, 75, 76, 77, 81, 83, 86, 87, 88 },
		{ 39, 46, 51, 55, 58, 63, 65, 67, 70, 75, 76, 77, 78, 82, 84, 87, 88 },
		{ 40, 47, 52, 56, 59, 64, 66, 68, 71, 76, 77, 78, 79, 83, 85, 88 },
		{ 41, 48, 53, 57, 60, 65, 67, 69, 72, 77, 78, 79, 80, 84, 86 },
		{ 42, 49, 54, 58, 61, 66, 68, 70, 73, 78, 79, 80, 81, 85, 87 },
		{ 43, 50, 55, 59, 62, 67, 69, 71, 74, 79, 80, 81, 82, 86, 88 },
		{ 44, 51, 56, 60, 63, 68, 70, 72, 75, 80, 81, 82, 83, 87 },
		{ 45, 52, 57, 61, 64, 69, 71, 73, 76, 81, 82, 83, 84, 88 },
		{ 46, 53, 58, 62, 65, 70, 72, 74, 77, 82, 83, 84, 85 },
		{ 47, 54, 59, 63, 66, 71, 73, 75, 78, 83, 84, 85, 86 },
		{ 48, 55, 60, 64, 67, 72, 74, 76, 79, 84, 85, 86, 87 },
		{ 49, 56, 61, 65, 68, 73, 75, 77, 80, 85, 86, 87, 88 },
		{ 50, 57, 62, 66, 69, 74, 76, 78, 81, 86, 87, 88 },
		{ 51, 58, 63, 67, 70, 75, 77, 79, 82, 87, 88 },
		{ 52, 59, 64, 68, 71, 76, 78, 80, 83, 88 },
		{ 53, 60, 65, 69, 72, 77, 79, 81, 84 },
		{ 54, 61, 66, 70, 73, 78, 80, 82, 85 },
		{ 55, 62, 67, 71, 74, 79, 81, 83, 86 },
		{ 56, 63, 68, 72, 75, 80, 82, 84, 87 },
		{ 57, 64, 69, 73, 76, 81, 83, 85, 88 },
		{ 58, 65, 70, 74, 77, 82, 84, 86 },
		{ 59, 66, 71, 75, 78, 83, 85, 87 },
		{ 60, 67, 72, 76, 79, 84, 86, 88 },
		{ 61, 68, 73, 77, 80, 85, 87 },
		{ 62, 69, 74, 78, 81, 86, 88 },
		{ 63, 70, 75, 79, 82, 87 },
		{ 64, 71, 76, 80, 83, 88 },
		{ 65, 72, 77, 81, 84 },
		{ 66, 73, 78, 82, 85 },
		{ 67, 74, 79, 83, 86 },
		{ 68, 75, 80, 84, 87 },
		{ 69, 76, 81, 85, 88 },
		{ 70, 77, 82, 86 },
		{ 71, 78, 83, 87 },
		{ 72, 79, 84, 88 },
		{ 73, 80, 85 },
		{ 74, 81, 86 },
		{ 75, 82, 87 },
		{ 76, 83, 88 },
		{ 77, 84 },
		{ 78, 85 },
		{ 79, 86 },
		{ 80, 87 },
		{ 81, 88 },
		{ 82 },
		{ 83 },
		{ 84 },
		{ 85 },
		{ 86 },
		{ 87 },
		{ 88 }
	};
	for (int i = 0; i < 76;i++){
		vector<int> pitch;
		for (int j = 0; j < 26; j++){
			if (multifrq[i][j] != 0){
				pitch.push_back(multifrq[i][j]);
			}
		}
		MulitiFrq.insert(pair<int, vector<int>>(i + 1, pitch));
	}
	int firstbar = scoreEvent[0][g_BarFirst][0];
	int firstindex = 0;
	for (vector<int>::size_type i = 0; i < scoreEvent.size(); i++){
		if (firstbar != scoreEvent[i][g_BarFirst][0]){
			RhyTime.push_back(scoreEvent[i][0][0] - scoreEvent[firstindex][0][0]);
			firstbar = scoreEvent[i][g_BarFirst][0];
			firstindex = i;
		}
	}
	RhyTime.push_back(0.0);
	map<double, int> dataMap;
	for (int i = 0; i < RhyTime.size(); i++){
		dataMap[RhyTime[i]]++;
	}
	int counts = 0;
	double MaxTime;
	for (map<double,int>::iterator it = dataMap.begin(); it!=dataMap.end(); it++){
		if (it->second > counts){
			counts = it->second;
			MaxTime = it->first;
		}
	}
	for (vector<int>::size_type i = 0; i < RhyTime.size(); i++){
		if (abs(RhyTime[i] - MaxTime) < 1e-2){
			RhyTime[i] = MaxTime;
		}
	}
}


void ScoreFollowing::SetFinger(int flag){  // flag = 0 为左手  flag = 1位右手  flag = 2 为双手
	vector<vector<vector<double>>> scoreEventModify;
	map<int,int>().swap(BarNum);
	if (flag == 0){
		for (vector<int>::size_type i = 0; i < scoreEvent.size(); i++){
			vector<double>::iterator it = find(scoreEvent[i][7].begin(), scoreEvent[i][7].end(), 2);
			if (it != scoreEvent[i][7].end()){
				vector<vector<double>> tempScoreEvent;
				for (size_t z = 0; z < 3; z++){
					tempScoreEvent.push_back(scoreEvent[i][z]);
				}
				vector<double> temp1;
				vector<double> temp2;
				vector<double> temp3;
				for (vector<int>::size_type j = 0; j < scoreEvent[i][7].size(); j++){
					if (scoreEvent[i][7][j] == 2){
						temp1.push_back(scoreEvent[i][3][j]);
						temp2.push_back(scoreEvent[i][4][j]);
						temp3.push_back(scoreEvent[i][7][j]);
					}
				}
				tempScoreEvent.push_back(temp1);
				tempScoreEvent.push_back(temp2);
				for (size_t z = 5; z < 7; z++){
					tempScoreEvent.push_back(scoreEvent[i][z]);
				}
				tempScoreEvent.push_back(temp3);
				scoreEventModify.push_back(tempScoreEvent);
			}
		}
	}
	else if (flag == 1){
		for (vector<int>::size_type i = 0; i < scoreEvent.size(); i++){
			vector<double>::iterator it = find(scoreEvent[i][7].begin(), scoreEvent[i][7].end(), 1);
			if (it != scoreEvent[i][7].end()){
				vector<vector<double>> tempScoreEvent;
				for (size_t z = 0; z < 3; z++){
					tempScoreEvent.push_back(scoreEvent[i][z]);
				}
				vector<double> temp1;
				vector<double> temp2;
				vector<double> temp3;
				for (vector<int>::size_type j = 0; j < scoreEvent[i][7].size(); j++){
					if (scoreEvent[i][7][j] == 1){
						temp1.push_back(scoreEvent[i][3][j]);
						temp2.push_back(scoreEvent[i][4][j]);
						temp3.push_back(scoreEvent[i][7][j]);
					}
				}
				tempScoreEvent.push_back(temp1);
				tempScoreEvent.push_back(temp2);
				for (size_t z = 5; z < 7; z++){
					tempScoreEvent.push_back(scoreEvent[i][z]);
				}
				tempScoreEvent.push_back(temp3);
				scoreEventModify.push_back(tempScoreEvent);
			}
		}
	}
	else{
		scoreEventModify = scoreEvent;
	}
	if (scoreEventModify.size() > 0){
		vector<int> barfirt;
		vector<int> barfirt1;
		for (vector<int>::size_type i = 0; i < scoreEventModify.size(); i++){
			barfirt.push_back(static_cast<int>(scoreEventModify[i][g_BarFirst][0]));
		}
		for (vector<int>::size_type i = 0; i < scoreEvent.size(); i++){
			barfirt1.push_back(static_cast<int>(scoreEvent[i][g_BarFirst][0]));
		}
		sort(barfirt.begin(), barfirt.end());
		sort(barfirt1.begin(), barfirt1.end());
		barfirt.erase(unique(barfirt.begin(), barfirt.end()), barfirt.end());
		barfirt1.erase(unique(barfirt1.begin(), barfirt1.end()), barfirt1.end());
		map<int, int> barnum;
		for (vector<int>::size_type i = 0; i < barfirt.size(); i++){
			for (vector<int>::size_type j = 0; j < barfirt1.size(); j++){
				if (barfirt[i] == barfirt1[j]){
					barnum.insert(pair<int, int>(barfirt[i], j));
				}
			}
		}
		vector<int> barFirst;
		int counts = 1;
		vector<int> temp;
		barFirst.push_back(static_cast<int>(scoreEventModify[0][g_BarFirst][0]));
		for (vector<int>::size_type i = 1; i < scoreEventModify.size(); i++){
			if (scoreEventModify[i][5][0] != barFirst.back()){
				barFirst.push_back(scoreEventModify[i][5][0]);
				temp.push_back(counts);
				counts = 1;
			}
			else{
				counts++;
			}
		}
		for (int i = 0; i < barFirst.size(); i++){
			for (map<int, int>::iterator iter = barnum.begin(); iter != barnum.end(); iter++){
				if (iter->first == barFirst[i]){
					BarNum.insert(pair<int, int>(i, iter->second));
					break;
				}
			}
		}
		temp.push_back(counts);
		vector<int> barcount;
		int sum = 1;
		for (int i = 0; i < temp.size(); i++){
			barcount.push_back(sum);
			sum += temp[i];
		}
		vector<int> barfirst;
		int index = 0;
		barfirst.push_back(static_cast<int>(scoreEventModify[0][g_BarFirst][0]));
		for (vector<int>::size_type i = 0; i < scoreEventModify.size(); i++){
			if (scoreEventModify[i][5][0] != barfirst.back()){
				barfirst.push_back(scoreEventModify[i][5][0]);
				index++;
			}
			scoreEventModify[i][5][0] = barcount[index];
		}
		vector<vector<vector<double>>>().swap(scoreEvent);
		scoreEvent = scoreEventModify;
	}
}

void ScoreFollowing::ScoreFollowingOffline(const vector<vector<double> > &H, vector<double> & error,double timeResolution, int maxPitchesInEvent)
{
	EvaluateSfResult evaluateSfResult(*this);
	evaluateSfResult.Init();
    int sfResultSize = sfResult[0].size();
    int stopFrame = 0;
    for (vector<int>::size_type i = 0; i < H.size(); ++i) {
        int iFrame = i;
        if (i == H.size()-1) {
            iFrame = -1;
        }
        processNMF.ProcessingFrame(H[i], iFrame, timeResolution, maxPitchesInEvent);
        vector<int> pitches;
        bool flag = processNMF.UpdateEventFlag(pitches, iFrame);
        if (flag) {
			MinusDoubleFreq(pitches, iEventPre);
			/*if (error.back() > 200)
				MinusPeopleNoise(pitches, iEventPre);*/
			PitchesPair.push_back(pitches);
			if (pitches.size()>0)
				UpdateEvent(pitches, iFrame);
        }
        if (sfResult[2].size() > sfResultSize) { // 有新的定位产生
			int newLocation = static_cast<int>(sfResultLocate.back()); // 新的定位结果

			vector<vector<double>> timePitchPairs = processNMF.GetTimePitchesPair();

			double onset = timePitchPairs.back()[0]; // 定位时间
			for (vector<int>::size_type j = 0; j < timePitchPairs.back().size(); j += 2) {
				if (timePitchPairs.back()[j] < onset) {
					onset = timePitchPairs.back()[j];
                }
            }
            int newBeatIndex = -1;
			vector<BeatRhythm> realtimeBeatRhythm = evaluateSfResult.EvaluateBeatRhythmRealtime(newLocation, onset, newBeatIndex, PitchesPair);
			if (newBeatIndex >= 0){
				IEventTime.push_back(realtimeBeatRhythm[newBeatIndex].during);
			}
            if (newBeatIndex != -1) {
                cout << "Beat rhythm: " << newBeatIndex << "\t";
                cout << realtimeBeatRhythm[newBeatIndex].progress << "\t" << realtimeBeatRhythm[newBeatIndex].start << "\t";
                cout << realtimeBeatRhythm[newBeatIndex].end << "\t" << realtimeBeatRhythm[newBeatIndex].during << endl;
            }

            sfResultSize = sfResult[2].size();
            stopFrame = 0;
        } else { // 没有新定位产生，记录有多少帧没有新定位
            ++stopFrame;
			if (!this->SetStopFrame())
				stopFrame = 0;
        }
        // 当没有新定位产生帧数较多，同时定位快接近结尾时，认为结束了
		int stopindex = GetStopFrame(H[i]);
		if (stopFrame >= stopindex && this->CheckLocatingEnd()) {
            cout << "end of wav" << endl;
			break;
        }
    }
    SetSfResultPair(processNMF.GetTimePitchesPair());
    sfResultOrigin[0] = sfResult[0];
    sfResultOrigin[1] = sfResult[1];
//    Save3DVector("../Data/Pad/result/Santa_Lucia_timeHPeakPair.txt", processNMF.GetTimeHPeakPair());
}

void ScoreFollowing::MinusDoubleFreq(vector<int> &pitches, int location){
	vector<int> newpitches = pitches;
	vector<int> nowOctive;
	for (vector<int>::size_type i = 0; i < pitches.size(); i++){
		bool findPitch = find(scorepPitch.begin(), scorepPitch.end(), pitches[i]) == scorepPitch.end();
		bool findOctive = find(scoreOctive.begin(), scoreOctive.end(), pitches[i] % 12) != scoreOctive.end();
		if (findPitch && findOctive){
			vector<int>::iterator pos = remove(newpitches.begin(), newpitches.end(), pitches[i]);
			newpitches.erase(pos, newpitches.end());
		}
	}
	int begin = location - 3 >= 0 ? location - 3 : 0;
	int end = location + 3 < scoreEvent.size() ? location + 3 : scoreEvent.size() - 1;
	bool lineOctive = true;

	if (location < scoreEvent.size() - 1){
		vector<int>().swap(pitches);
		for (int i = 0; i < newpitches.size(); i++){
			nowOctive.push_back(newpitches[i] % 12);
		}
		vector<int> temp(12, 0);
		for (int i = 0; i < nowOctive.size(); i++){
			temp[nowOctive[i]]++;
		}
		vector<int> it;
		for (int i = 0; i < temp.size(); i++){
			if (temp[i]>1){
				it.push_back(i);
			}
		}
		vector<int> pitch;
		if (!it.empty()){
			for (int i = 0; i < newpitches.size(); i++){
				int octive = newpitches[i] % 12;
				bool once = true;
				for (int j = 0; j < it.size(); j++){
					if (octive == it[j]){
						once = false;
						int tmpbegin = location - 3 >= 0 ? location - 3 : 0;
						int tmpend = location + 3 < scoreEvent.size() ? location + 3 : scoreEvent.size() - 1;
						for (int n = tmpbegin; n <= tmpend; n++){
							vector<double>::iterator it1 = find(scoreEvent[n][g_Pitches].begin(), scoreEvent[n][g_Pitches].end(), newpitches[i]);
							if (it1 != scoreEvent[n][g_Pitches].end()){
								pitch.push_back(newpitches[i]);
								break;
							}
						}
					}
				}
				if (once){
					pitch.push_back(newpitches[i]);
				}
			}
		}
		else{
			pitch = newpitches;
		}
		
		pitches = pitch;
	}
	/*if (location < scoreEvent.size() - 1){
		vector<int>().swap(pitches);
		set<int> tempitches;
		for (int i = 0; i < newpitches.size(); i++){
			nowOctive.push_back(newpitches[i] % 12);
			tempitches.insert(newpitches[i] % 12);
		}
		vector<int> temp(12, 0);
		for (int i = 0; i < nowOctive.size(); i++){
			temp[nowOctive[i]]++;
		}
		int it = 0;
		for (int i = 0; i < temp.size(); i++){
			if (temp[i]>1){
				it = i;
				break;
			}
		}
		for (vector<int>::size_type i = begin; i < end; i++){
			vector<int> octive;
			for (vector<int>::size_type j = 0; j < scoreEvent[i][4].size(); j++){
				octive.push_back(static_cast<int>(scoreEvent[i][4][j]));
			}
			if (count(octive.begin(), octive.end(), it)>1){
				lineOctive = false;
			}
		}
		vector<int> loc;
		if (tempitches.size() != newpitches.size() && lineOctive){
			for (int i = 0; i < newpitches.size(); i++){
				if (it != newpitches[i] % 12)
					pitches.push_back(newpitches[i]);
				else{
					int tmpbegin = location - 2 >= 0 ? location - 2 : 0;
					int tmpend = location + 2 < scoreEvent.size() ? location + 2 : scoreEvent.size() - 1;
					for (int j = tmpbegin; j <= tmpend; j++){
						vector<double>::iterator it1 = find(scoreEvent[j][g_Pitches].begin(), scoreEvent[j][g_Pitches].end(), newpitches[i]);
						if (it1 != scoreEvent[j][g_Pitches].end()){
							loc.push_back(i);
							break;
						}
					}
				}
			}
			if (loc.size() == 0){
				for (int j = 0; j < nowOctive.size(); j++){
					if (it == nowOctive[j]){
						pitches.push_back(newpitches[j]);
						break;
					}
				}
			}
			else{
				for (int j = 0; j < loc.size(); j++){
					for (int n = 0; n < newpitches.size(); n++){
						if (loc[j] == n){
							pitches.push_back(newpitches[n]);
							break;
						}
					}
				}
			}
		}
		else
			pitches = newpitches;
	}*/
	sort(pitches.begin(), pitches.end());
	pitches.erase(unique(pitches.begin(),pitches.end()),pitches.end());
}

void ScoreFollowing::UpdateEvent(vector<int> pitches, int iFrame)
{
    vector<vector<int> > iEvent;
	map<int, int>::iterator iter;
    if (iFrame != -1) {
		iEvent = scoreFollowingEvent(pitches, scoreEvent, sfResultOriginLocate, iEventPre, candidate, isPlayed, isSureFlag);
        sfResult[1].push_back(vector<double>{static_cast<double>(isSureFlag)}); // 是否在当帧确定定位
        bool isEmpty = iEvent.empty();
        if (!isEmpty) {
			for (iter = scoreLocate.begin(); iter != scoreLocate.end(); iter++){
				if (iter->first == iEvent[0][0] + 1){
					sfResult[2].push_back(vector<double>{static_cast<double>(iter->second)}); // 定位
					sfResultLocate.push_back(iEvent[0][0] + 1);
					break;
				}
			}
            for (vector<int>::size_type i = 0; i < iEvent[0].size(); i++) { // 如果有n个定位输出，将前面的n-1个修改
                int index = sfResult[2].size() - 1 - (iEvent[0].size() - 1 - i);
                // if (index < 0) continue;
				for (iter = scoreLocate.begin(); iter !=scoreLocate.end(); iter++){
					if (iter->first == iEvent[0][i] + 1){
						sfResult[2][index] = vector<double>{static_cast<double>(iter->second)};
						sfResultLocate[index] = iEvent[0][i] + 1;
					}
				}
			}
			for (iter = scoreLocate.begin(); iter != scoreLocate.end();iter++){
				if (iter->first == iEvent[0].back() + 1){
					sfResultOrigin[2].push_back(vector<double>{static_cast<double>(iter->second)});
					sfResultOriginLocate.push_back(iEvent[0].back() + 1);
				}
			}
            iEventPre = iEvent[0][iEvent[0].size() - 1];
            // 存下实时的定位结果，就是给出定位的最后一个
        }
    } else if (isSureFlag == 0) {
        int barFirst = static_cast<int>(scoreEvent[candidate.iEventLastSure][g_BarFirst][0]) - 1;
        iEvent = findPath(candidate, -1, barFirst);
        if (!iEvent.empty()) {
            // 之前push_back添加了元素，现在不需要添加，只需要修改即可
            for (vector<int>::size_type i = 0; i < iEvent[0].size(); ++i) { // 如果有n个定位输出，将前面的n-1个修改
                int index = sfResult[2].size()-1 - (iEvent[0].size()-1 - i);
                if (index < 0) continue;
				for (iter = scoreLocate.begin(); iter != scoreLocate.end(); iter++){
					if (iter->first == iEvent[0][i] + 1){
						sfResult[2][index] = vector<double>{static_cast<double>(iter->second)};
						break;
					}
				}
            }
        }
    }
}

//vector<int> ScoreFollowing::GetNoteInScore()
//{
//	vector<int> noteInScore;
//	if (scoreEvent.empty()) return noteInScore;
//
//	int bitmap[88] = { 0 };
//	for (vector<int>::size_type i = 0; i < scoreEvent.size(); ++i) {
//		for (vector<int>::size_type j = 0; j < scoreEvent[i][3].size(); ++j) {
//			int temp = static_cast<int>(scoreEvent[i][3][j]);
//			++bitmap[temp];
//		}
//	}
//	for (int i = 0; i < 88; ++i) {
//		if (bitmap[i] != 0) {
//			noteInScore.push_back(i);
//		}
//	}
//	vector<int> result;
//	if (!noteInScore.empty()) {
//		int begin = noteInScore[0] - 4;
//		int end = *(noteInScore.end() - 1) + 4;
//		begin = begin > 0 ? begin : 1; // 音符最低只能为1
//		end = end < 89 ? end : 88; // 音符最高只能为88
//		for (int i = begin; i <= end; ++i) {
//			result.push_back(i);
//		}
//	}
//	return result;
//}


void ScoreFollowing::MinusPeopleNoise(vector<int> &pitches, int location){
	vector<int> scorePitches;
	vector<int> newPitches;
	if (location == 0){
		for (vector<int>::size_type i = 0; i <2; i++){
			for (vector<int>::size_type j = 0; j < scoreEvent[i][3].size(); j++){
				scorePitches.push_back(static_cast<int>(scoreEvent[i][3][j]));
			}
		}
	}
	else if (location == 1){
		for (vector<int>::size_type i = 1; i <location + 2; i++){
			for (vector<int>::size_type j = 0; j < scoreEvent[i][3].size(); j++){
				scorePitches.push_back(static_cast<int>(scoreEvent[i][3][j]));
			}
		}
	}
	else if (location >= scoreEvent.size() - 1){
		for (vector<int>::size_type i = scoreEvent.size() - 2; i <scoreEvent.size(); i++){
			for (vector<int>::size_type j = 0; j < scoreEvent[i][3].size(); j++){
				scorePitches.push_back(static_cast<int>(scoreEvent[i][3][j]));
			}
		}
	}
	else{
		for (vector<int>::size_type i = location; i <= location + 1; i++){
			for (vector<int>::size_type j = 0; j < scoreEvent[i][3].size(); j++){
				scorePitches.push_back(static_cast<int>(scoreEvent[i][3][j]));
			}
		}
	}
	for (vector<int>::size_type i = 0; i < pitches.size(); i++){
		vector<int>::iterator it = find(scorePitches.begin(), scorePitches.end(), pitches[i]);
		if (it != scorePitches.end())
			newPitches.push_back(pitches[i]);
	}
	pitches = newPitches;
}

vector<int> ScoreFollowing::GetNoteInScore()
{
	vector<int> noteInScore;
	if (scoreEvent.empty()) return noteInScore;

	int bitmap[88] = { 0 };
	for (vector<int>::size_type i = 0; i < scoreEvent.size(); ++i) {
		for (vector<int>::size_type j = 0; j < scoreEvent[i][3].size(); ++j) {
			int temp = static_cast<int>(scoreEvent[i][3][j]);
			++bitmap[temp];
		}
	}
	for (int i = 0; i < 88; ++i) {
		if (bitmap[i] != 0) {
			noteInScore.push_back(i);
		}
	}
	set<int> tempresult;
	vector<int> result;
	if (!noteInScore.empty()){
		for (int i = 0; i < noteInScore.size(); i++){
			int begin = noteInScore[i] - 4;
			int end = noteInScore[i] + 4;
			begin = begin > 0 ? begin : 1;
			end = end < 89 ? end : 88;
			for (int j = begin; j <= end; j++){
				tempresult.insert(j);
			}
		}
	}
	for (set<int>::iterator it = tempresult.begin(); it != tempresult.end(); it++){
		result.push_back(*it);
	}
	sort(result.begin(), result.end());
	return result;
}

int ScoreFollowing::GetMaxPitchesInFrame()
{
    int maxPitches = 0;
    for (vector<int>::size_type i = 0; i < scoreEvent.size(); ++i) {
        if (scoreEvent[i][g_Pitches].size() > maxPitches) {
            maxPitches = scoreEvent[i][g_Pitches].size();
        }
    }
    return maxPitches + 1;
}

int ScoreFollowing::EndOfScore()
{
    return scoreEvent.size();
}

bool ScoreFollowing::CheckLocatingEnd()
{
    int k = 3;
    // 定位信息都不到k个，肯定没到乐谱结尾，直接返回false
    if (sfResult[2].size() < k) return false;
    // 定位信息超过k个，我们判断k个定位是否连续，并且比较靠近乐谱结尾
    vector<double> lastKLocation;
	vector<int>::iterator it = sfResultLocate.end() - k;
	for (; it != sfResultLocate.end(); ++it) {
        lastKLocation.push_back((*it));
    }
    vector<double> distance; // 保存定位结果与乐谱结尾的差距
    for (vector<double>::size_type i = 0; i < lastKLocation.size(); ++i) {
		distance.push_back(this->EndOfScore() - lastKLocation[i]);
    }
	
    bool nearToEnd = all_of(distance.begin(), distance.end(), [k](int x) {return x <= k;});
    // 如果最后一个定位为乐谱结尾，或者倒数k个定位都比较靠近结尾，我们认为演奏结束
    int lastLocation = static_cast<int>(sfResultLocate.back()); // 最后一个定位
    if (lastLocation == this->EndOfScore() || nearToEnd) {
        return true;
    }
    return false;
//    int lastLocation = static_cast<int>(sfResult[2].back()[0]); // 最后一个定位
//    if (lastLocation == this->EndOfScore()) return true;
//    return false;
}

void ScoreFollowing::SetSfResultPair(vector<vector<double> > timePitchPair)
{
	vector<vector<double> > timePitchesPair;

	for (vector<int>::size_type i = 0; i < timePitchPair.size() && i<PitchesPair.size(); i++){
		vector<double> timepitch;
		for (int j = 0; j < PitchesPair[i].size(); j++){
			vector<double>::iterator it = find(timePitchPair[i].begin(), timePitchPair[i].end(), PitchesPair[i][j]);
			if (it != timePitchPair[i].end()){
				bool booltim = find(timePitchPair[i].begin(), timePitchPair[i].end(), *(it - 1)) != timePitchPair[i].end();
				bool boolpitch = find(timePitchPair[i].begin(), timePitchPair[i].end(), *it) != timePitchPair[i].end();
				if (booltim && boolpitch){
					timepitch.push_back(*(it - 1));
					timepitch.push_back(*it);
				}
			}
		}
		if (timepitch.size() > 0)
			timePitchesPair.push_back(timepitch);
	}
	sfResult[0] = timePitchesPair;
}

vector<vector<vector<double>>> ScoreFollowing::GetSfResult()
{
    return sfResult;
}

vector<vector<vector<double>>> ScoreFollowing::GetSfResultOrigin()
{
    return sfResultOrigin;
}

vector<vector<vector<double>>> ScoreFollowing::GetScoreEvent()
{
    return scoreEvent;
}

void ScoreFollowing::SetPitchesPair(vector<vector<int>> pitchesPair){
	this->PitchesPair = pitchesPair;
}

vector<vector<int>> ScoreFollowing::GetPitchesPair(){
	return PitchesPair;
}

int ScoreFollowing::GetStopFrame(vector<double> H){
	int stopindex;
	if (this->CheckLocatingEnd()){
		/*double avetime = static_cast<double>(accumulate(IEventTime.begin(), IEventTime.end(), 0)/ static_cast<double>(IEventTime.size()));*/
		if (this->ScoreEventFinish()){
			SetSfResultPair(processNMF.GetTimePitchesPair());
			vector<vector<vector<double>>> sfresult = GetSfResult();
			double maxH = *max_element(H.begin(), H.end());
			if (maxH <= 10){
				TotalmaxH.push_back(maxH);
			}
			if (TotalmaxH.size() >= 3){
				stopindex = 30;
			}
			else{
				stopindex = static_cast<int>(2.5 * 44100 / 512);
			}
		}
		else if (this->NearScoreEventFinish()){
			stopindex = static_cast<int>(3 * 44100 / 512);
		}
	}
	else
		stopindex = 344;
	return stopindex;
}

bool ScoreFollowing::SetStopFrame(){
	bool allfinished = true;
	if (this->CheckLocatingEnd()){
		this->SetSfResultPair(processNMF.GetTimePitchesPair());
		vector<vector<double>> sfResultTime = sfResult[0];
		vector<vector<vector<double>>> timer = processNMF.GetTimeHPeakPair();
		for (vector<int>::size_type i = 1; i <sfResultTime.back().size() ; i+=2){
			for (size_t j = 0; j < timer[sfResultTime.back()[i]-1].size(); j++){
				if (timer[sfResultTime.back()[i] - 1][j].size() != 4){
					allfinished = false;
				}
			}
		}
	}
	return allfinished;
}
vector<int> ScoreFollowing::GetSfResultLocate(){
	return sfResultLocate;
}

vector<int> ScoreFollowing::GetSfResultOriginLocate(){
	return sfResultOriginLocate;
}

map<int, int> ScoreFollowing::GetBarNum(){
	return BarNum;
}

bool ScoreFollowing::ScoreEventFinish(){
	int lastLocation = static_cast<int>(sfResultLocate.back()); // 最后一个定位
	if (lastLocation == this->EndOfScore()) {
		return true;
	}
	return false;
}

bool ScoreFollowing::NearScoreEventFinish(){
	int k = 3;
	// 定位信息都不到k个，肯定没到乐谱结尾，直接返回false
	if (sfResult[2].size() < k) return false;
	// 定位信息超过k个，我们判断k个定位是否连续，并且比较靠近乐谱结尾
	vector<double> lastKLocation;
	vector<int>::iterator it = sfResultLocate.end() - k;
	for (; it != sfResultLocate.end(); ++it) {
		lastKLocation.push_back((*it));
	}
	vector<double> distance; // 保存定位结果与乐谱结尾的差距
	for (vector<double>::size_type i = 0; i < lastKLocation.size(); ++i) {
		distance.push_back(this->EndOfScore() - lastKLocation[i]);
	}

	bool nearToEnd = all_of(distance.begin(), distance.end(), [k](int x) {return x <= k; });
	// 如果最后一个定位为乐谱结尾，或者倒数k个定位都比较靠近结尾，我们认为演奏结束
	if (nearToEnd) {
		return true;
	}
	return false;
}

map<int, vector<int>> ScoreFollowing::GetMultiFreq(){
	return MulitiFrq;
}

vector<vector<vector<double>>> ScoreFollowing::GettimePitchesPair(){
	vector<vector<double> > timePitchesPair = processNMF.GetTimePitchesPair();
	timeHPeakPair = processNMF.GetTimeHPeakPair();
	for (vector<int>::size_type i = 0; i < timePitchesPair.size() && i<PitchesPair.size(); i++){
		for (int j = 1; j < timePitchesPair[i].size(); j+=2){
			vector<int>::iterator it = find(PitchesPair[i].begin(), PitchesPair[i].end(), static_cast<int>(timePitchesPair[i][j]));
			if (it == PitchesPair[i].end()){
				int location = static_cast<int>(timePitchesPair[i][j]);
				for (int k = 0; k < timeHPeakPair[location - 1].size();k++){
					for (vector<double>::iterator it2 = timeHPeakPair[location - 1][k].begin(); it2 != timeHPeakPair[location - 1][k].end();){
						if (timePitchesPair[i][j-1] == it2[0])
							it2 = timeHPeakPair[location - 1][k].erase(it2);
						else
							it2++;
					}
				}
			}
		}
	}
	return timeHPeakPair;
}

vector<double> ScoreFollowing::GetRhyTime(){
	return RhyTime;
}
