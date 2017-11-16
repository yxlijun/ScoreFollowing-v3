//
// Created by zhangqianyi on 2017/3/31.
//

#include "EvaluateSfResult.h"

EvaluateSfResult::EvaluateSfResult(ScoreFollowing& scoreFollowing1) : scoreFollowing(scoreFollowing1)
{
	//Init();
}


void EvaluateSfResult::Init()
{
    sfResult = scoreFollowing.GetSfResult();
    scoreEvent = scoreFollowing.GetScoreEvent();
	sfResultLocate = scoreFollowing.GetSfResultLocate();
	MulitiFrq = scoreFollowing.GetMultiFreq();

	RhyTime = scoreFollowing.GetRhyTime();

	timePitchesPair = scoreFollowing.GettimePitchesPair();
	barPair = scoreFollowing.GetBarNum();
    beatToleranceRate = 0.25;
    beatMaxDiffTime = 3;
    noteToleranceRate = 0.25;

    beatAvgTime = 0;
    beatToleranceTime = 0;

    // 获取节首节尾信息
    // 从乐谱中读取节首信息，节首位于scoreEvent的第g_BarFirst行，数据为连续的1，连续的5之类的，所以遍历一遍，然后把不同的数值存下来即可
	barFirst.push_back(static_cast<int>(scoreEvent[0][g_BarFirst][0]));
	for (vector<int>::size_type i = 1; i < scoreEvent.size(); ++i) {
		if (barFirst.back() != scoreEvent[i][g_BarFirst][0]) {
			barFirst.push_back(static_cast<int>(scoreEvent[i][g_BarFirst][0]));
		}
	}
	// 第一节结尾位置就是第二节节首位置减1，barEnd[i] = barFirst[i+1]-1
	for (vector<int>::size_type i = 0; i < barFirst.size() - 1; ++i) {
		barEnd.push_back(barFirst[i + 1] - 1);
	}
	barEnd.push_back(scoreEvent.size()); // 最后一个节尾就是乐谱的位置个数 
    realtimeBeatIndex = 0; // 实时定位结果在第几小节，初始化为0
    realtimeBeatRhythm = vector<BeatRhythm>(barFirst.size()); // 实时的小节节奏评价
}

vector<Correctness> EvaluateSfResult::EvaluateCorrectness()
{
    // 存放演奏正确性评价表，每一行表示当前位置的正确性
    vector<Correctness> totalCorrectness;
	vector<Correctness> totalCorrectness1;
	vector<int> AllLocation;
	map<int, bool> LocationMap;
	for (int i = 0; i < sfResult[0].size(); i++){
		AllLocation.push_back(static_cast<int>(sfResultLocate[i]));
		LocationMap.insert(pair<int, int>(sfResultLocate[i], true));
	}
    // 遍历sfResult的每一个位置
    for (vector<int>::size_type i = 0; i < sfResult[2].size(); ++i) {
        // 评价乐谱中第iEventScore个位置的演奏正确性
        int location = static_cast<int>(sfResultLocate[i]); // 当前演奏的音符在乐谱中的位置
		
        // 遍历当前位置演奏的音符
        set<int> pitchesPerformance;
        for (vector<int>::size_type j = 1; j < sfResult[0][i].size(); j += 2) {
            pitchesPerformance.insert(static_cast<int>(sfResult[0][i][j]));
        }

        // 如果当前定位和上一个定位位置相同，我们将上个定位结果的正确音符也加到这个位置上
        if (i > 0 && sfResultLocate[i] == sfResultLocate[i-1]) {
            // 上个定位结果的正确音符为 totalCorrectness.back().intersection
            for (vector<int>::size_type j = 0; j < totalCorrectness.back().intersection.size(); ++j) {
                pitchesPerformance.insert(totalCorrectness.back().intersection[j]);
            }
        }

        // 与乐谱中当前位置的标准音符对比，是否正确演奏，漏弹，多弹，错弹
        set<int> pitchesInScore;
        for (vector<int>::size_type j = 0; location <= scoreEvent.size() && j < scoreEvent[location - 1][g_Pitches].size(); ++j) {
            pitchesInScore.insert(static_cast<int>(scoreEvent[location - 1][g_Pitches][j]));
        }

        Correctness* correctness = new Correctness();
        // 判断是否是回弹，我们认为当前定位比上一次定位值小就出现了回弹
        if (i > 0 && sfResultLocate[i] - sfResultLocate[i-1] < 0) {
            correctness->jumpback = 1;
        } else {
            correctness->jumpback = 0;
        }
        int intersection[20]; // 交集，预分配20大小，演奏的音符和乐谱中相应位置的音符交集不会超过20个音符
        int *end = set_intersection(pitchesInScore.begin(), pitchesInScore.end(), pitchesPerformance.begin(),
                                    pitchesPerformance.end(), intersection);
        for (int *p = intersection; p != end; ++p) {
            correctness->intersection.push_back(*p);
        }

        int difference1[10]; // 差集，预分配10大小
        end = set_difference(pitchesInScore.begin(), pitchesInScore.end(), pitchesPerformance.begin(),
                             pitchesPerformance.end(), difference1);
        for (int *p = difference1; p != end; ++p) {
            correctness->omission.push_back(*p);
        }
        int difference2[10];
        end = set_difference(pitchesPerformance.begin(), pitchesPerformance.end(), pitchesInScore.begin(),
                             pitchesInScore.end(), difference2);
        for (int *p = difference2; p != end; ++p) {
			correctness->excess.push_back(*p);
        }


        // 如果当前位置多弹了的音符在上一个位置的应该被演奏的音符中，我们不认为是多弹了
        // 比如当前位置为7，检测到了33,34,35，但是乐谱7位置的音符是34,35，乐谱6位置是32,33
        // 可能是由于划分位置不恰当导致我们误认为位置7多弹了33，实际上这个33是位置6的延音，我们将这个音从多检的数组中删除
        // 另外要考虑倍频错误，如果检测到33,45，如果认为45是多演奏的是不对的，45是倍频可能是多音调33的倍频错误，将这个值删除
        // 最后还有考虑上一个位置延音的倍频错误

        vector<int> lastPitchesInScore; // 上一个位置的音符
		vector<int> nextPitchesInScore; // 下一个位置的音符
        vector<int> lastOctavesInScore; // 上一个位置的octave
		vector<int> nextOctavesInScore;  // 下一个位置的octave
        vector<int> currOctavesInScore; // 当前位置的octave

    
		for (vector<int>::size_type z = 1; z <=2; z++){
			for (vector<int>::size_type j = 0; location >2 && location - z - 1<scoreEvent.size() && j <scoreEvent[location - z - 1][g_Pitches].size(); j++) {
				int lastpitch = static_cast<int>(scoreEvent[location -z - 1][g_Pitches][j]);
				lastPitchesInScore.push_back(lastpitch);
				while (lastpitch >= 12) lastpitch -= 12;
				lastOctavesInScore.push_back(lastpitch);
			}
		}
		
		for (vector<int>::size_type z = 0; z <2; z++){
			for (vector<int>::size_type j = 0; location<scoreEvent.size()-2 && j < scoreEvent[location+z][g_Pitches].size(); j++) {
				int nextpitch = static_cast<int>(scoreEvent[location+z][g_Pitches][j]);
				nextPitchesInScore.push_back(nextpitch);
				while (nextpitch >= 12) nextpitch -= 12;
				nextOctavesInScore.push_back(nextpitch);
			}
		}
		for (set<int>::iterator it = pitchesInScore.begin(); it != pitchesInScore.end(); it++){
			int nowpitch = *it;
			while (nowpitch >= 12) nowpitch -= 12;
			currOctavesInScore.push_back(nowpitch);
		}
		Correctness correctness1 = *correctness;
		bool DeletePitch = true;
		for (vector<int>::iterator it = correctness->excess.begin(); it != correctness->excess.end();) {
			int repeatLoc = count(AllLocation.begin(), AllLocation.end(), location);
			if (repeatLoc >= 4){
				for (map<int, bool>::iterator it = LocationMap.begin(); it != LocationMap.end(); it++){
					if (location == it->first){
						if (it->second){
							DeletePitch = false;
							it->second = false;
						}
					}
				}
			}
			// 是否是上一个位置的延音
			bool findInLast = find(lastPitchesInScore.begin(), lastPitchesInScore.end(), *it) != lastPitchesInScore.end();
			bool findInNext = find(nextPitchesInScore.begin(), nextPitchesInScore.end(), *it) != nextPitchesInScore.end();
			// 是否是当前位置的倍频错误
			int currOctave = *it;
			while (currOctave >= 12) currOctave -= 12;
			bool findInLastOctave = find(lastOctavesInScore.begin(), lastOctavesInScore.end(), currOctave) != lastOctavesInScore.end();
			bool findInOctave = find(currOctavesInScore.begin(), currOctavesInScore.end(), currOctave) != currOctavesInScore.end();
			bool findInNextOctave = find(nextOctavesInScore.begin(), nextOctavesInScore.end(), currOctave) != nextOctavesInScore.end();

			bool isMutifreq = false;
			for (vector<int>::iterator it2 = correctness->intersection.begin(); it2 != correctness->intersection.end(); it2++){
				for (map<int, vector<int>>::iterator it3 = MulitiFrq.begin(); it3 != MulitiFrq.end(); it3++){
					if (it3->first == *it2){
						for (int k = 0; k < it3->second.size(); k++){
							if (*it == it3->second[k]){
								isMutifreq = true;
								break;
							}
						}
					}
				}
			}
			int counts = 0;
			int index1 = 0;
			int distance = 0;
			for (vector<Correctness>::iterator it2 = totalCorrectness1.begin(); it2 != totalCorrectness1.end(); it2++){
				index1++;
				for (vector<int>::size_type j = 0; j < it2->excess.size(); j++){
					if (it2->excess[j] == *it)
						counts++;
				}
				for (vector<int>::size_type j = 0; j < it2->intersection.size(); j++){
					if (it2->intersection[j] == *it)
						counts++;
				}
			}
			bool Keep = true;
			for (int j = index1 - 1; Keep && j >= 0; j--) {
				distance++;
				if (j < sfResult[0].size()){
					for (int z = 1; z < sfResult[0][j].size(); z += 2){
						if (*it == sfResult[0][j][z]){
							Keep = false;
							break;
						}
					}
				}
			}
			double PrevHpeakerMax = 0.0;
			double NowHpeakerMax = 0.0;

			for (vector<int>::size_type j = 1; counts - 1 < timePitchesPair[*it - 1].size() && j < timePitchesPair[*it - 1][counts - 1].size(); j += 2){
				if (PrevHpeakerMax < timePitchesPair[*it - 1][counts - 1][j])
					PrevHpeakerMax = timePitchesPair[*it - 1][counts - 1][j];
			}
			for (vector<int>::size_type j = 1; counts < timePitchesPair[*it - 1].size() && j < timePitchesPair[*it - 1][counts].size(); j += 2){
				if (NowHpeakerMax < timePitchesPair[*it - 1][counts][j])
					NowHpeakerMax = timePitchesPair[*it - 1][counts][j];
			}
			bool MinusKeepPitch = ((NowHpeakerMax * 2) < PrevHpeakerMax && distance<=7);
			//cout << *it << "..." << PrevHpeakerMax << "..." << NowHpeakerMax << "...." << MinusKeepPitch << endl;
			// 是否是上一个位置的倍频错误
			if ((findInLast || findInLastOctave || findInOctave || findInNext || findInNextOctave || isMutifreq || MinusKeepPitch) && DeletePitch) {
				it = correctness->excess.erase(it);
			}
			else ++it;

		}
		for (vector<int>::iterator it = correctness1.excess.begin(); it != correctness1.excess.end();) {
			// 是否是上一个位置的延音
			bool findInLast = find(lastPitchesInScore.begin(), lastPitchesInScore.end(), *it) != lastPitchesInScore.end();
			bool findInNext = find(nextPitchesInScore.begin(), nextPitchesInScore.end(), *it) != nextPitchesInScore.end();
			// 是否是当前位置的倍频错误
			int currOctave = *it;
			while (currOctave >= 12) currOctave -= 12;
			bool findInLastOctave = find(lastOctavesInScore.begin(), lastOctavesInScore.end(), currOctave) != lastOctavesInScore.end();
			bool findInOctave = find(currOctavesInScore.begin(), currOctavesInScore.end(), currOctave) != currOctavesInScore.end();
			bool findInNextOctave = find(nextOctavesInScore.begin(), nextOctavesInScore.end(), currOctave) != nextOctavesInScore.end();

			bool isMutifreq = false;
			for (vector<int>::iterator it2 = correctness->intersection.begin(); it2 != correctness->intersection.end(); it2++){
				for (map<int, vector<int>>::iterator it3 = MulitiFrq.begin(); it3 != MulitiFrq.end(); it3++){
					if (it3->first == *it2){
						for (int k = 0; k < it3->second.size(); k++){
							if (*it == it3->second[k]){
								isMutifreq = true;
								break;
							}
						}
					}
				}
			}
			// 是否是上一个位置的倍频错误
			if ((findInLast || findInLastOctave || findInOctave || findInNext || findInNextOctave || isMutifreq) && DeletePitch) {
				it = correctness1.excess.erase(it);
			}
			else ++it;
		}
		totalCorrectness1.push_back(correctness1);
        totalCorrectness.push_back(*correctness);
    }
    return totalCorrectness;
}
vector<Correctness> EvaluateSfResult::EvaluateCorrectnessModify()
{
	// 存放演奏正确性评价表，每一行表示当前位置的正确性
	vector<Correctness> totalCorrectness;
	vector<Correctness> totalCorrectness1;
	vector<int> AllLocation;
	map<int, bool> LocationMap;
	for (int i = 0; i < sfResult[0].size(); i++){
		AllLocation.push_back(static_cast<int>(sfResultLocate[i]));
		LocationMap.insert(pair<int, int>(sfResultLocate[i], true));
	}
	// 遍历sfResult的每一个位置
	for (vector<int>::size_type i = 0; i < sfResult[2].size(); ++i) {
		// 评价乐谱中第iEventScore个位置的演奏正确性
		int location = static_cast<int>(sfResultLocate[i]); // 当前演奏的音符在乐谱中的位置
		// 遍历当前位置演奏的音符
		set<int> pitchesPerformance;
		for (vector<int>::size_type j = 1; j < sfResult[0][i].size(); j += 2) {
			pitchesPerformance.insert(static_cast<int>(sfResult[0][i][j]));
		}

		// 如果当前定位和上一个定位位置相同，我们将上个定位结果的正确音符也加到这个位置上
		if (i > 0 && sfResultLocate[i] == sfResultLocate[i - 1]) {
			// 上个定位结果的正确音符为 totalCorrectness.back().intersection
			for (vector<int>::size_type j = 0; j < totalCorrectness.back().intersection.size(); ++j) {
				pitchesPerformance.insert(totalCorrectness.back().intersection[j]);
			}
		}

		// 与乐谱中当前位置的标准音符对比，是否正确演奏，漏弹，多弹，错弹
		set<int> pitchesInScore;
		for (vector<int>::size_type j = 0; location <= scoreEvent.size() && j < scoreEvent[location - 1][g_Pitches].size(); ++j) {
			pitchesInScore.insert(static_cast<int>(scoreEvent[location - 1][g_Pitches][j]));
		}

		Correctness* correctness = new Correctness();
		// 判断是否是回弹，我们认为当前定位比上一次定位值小就出现了回弹
		if (i > 0 && sfResultLocate[i] - sfResultLocate[i - 1] < 0) {
			correctness->jumpback = 1;
		}
		else {
			correctness->jumpback = 0;
		}
		int intersection[20]; // 交集，预分配20大小，演奏的音符和乐谱中相应位置的音符交集不会超过20个音符
		int *end = set_intersection(pitchesInScore.begin(), pitchesInScore.end(), pitchesPerformance.begin(),
			pitchesPerformance.end(), intersection);
		for (int *p = intersection; p != end; ++p) {
			correctness->intersection.push_back(*p);
		}

		int difference1[10]; // 差集，预分配10大小
		end = set_difference(pitchesInScore.begin(), pitchesInScore.end(), pitchesPerformance.begin(),
			pitchesPerformance.end(), difference1);
		for (int *p = difference1; p != end; ++p) {
			correctness->omission.push_back(*p);
		}
		int difference2[10];
		end = set_difference(pitchesPerformance.begin(), pitchesPerformance.end(), pitchesInScore.begin(),
			pitchesInScore.end(), difference2);
		for (int *p = difference2; p != end; ++p) {
			correctness->excess.push_back(*p);
		}


		// 如果当前位置多弹了的音符在上一个位置的应该被演奏的音符中，我们不认为是多弹了
		// 比如当前位置为7，检测到了33,34,35，但是乐谱7位置的音符是34,35，乐谱6位置是32,33
		// 可能是由于划分位置不恰当导致我们误认为位置7多弹了33，实际上这个33是位置6的延音，我们将这个音从多检的数组中删除
		// 另外要考虑倍频错误，如果检测到33,45，如果认为45是多演奏的是不对的，45是倍频可能是多音调33的倍频错误，将这个值删除
		// 最后还有考虑上一个位置延音的倍频错误

		vector<int> lastPitchesInScore; // 上一个位置的音符
		vector<int> nextPitchesInScore; // 下一个位置的音符
		vector<int> lastOctavesInScore; // 上一个位置的octave
		vector<int> nextOctavesInScore;  // 下一个位置的octave
		vector<int> currOctavesInScore; // 当前位置的octave


		for (vector<int>::size_type z = 1; z <= 2; z++){
			for (vector<int>::size_type j = 0; location >2 && location - z - 1<scoreEvent.size() && j <scoreEvent[location - z - 1][g_Pitches].size(); j++) {
				int lastpitch = static_cast<int>(scoreEvent[location - z - 1][g_Pitches][j]);
				lastPitchesInScore.push_back(lastpitch);
				while (lastpitch >= 12) lastpitch -= 12;
				lastOctavesInScore.push_back(lastpitch);
			}
		}

		for (vector<int>::size_type z = 0; z <2; z++){
			for (vector<int>::size_type j = 0; location<scoreEvent.size() - 2 && j < scoreEvent[location + z][g_Pitches].size(); j++) {
				int nextpitch = static_cast<int>(scoreEvent[location + z][g_Pitches][j]);
				nextPitchesInScore.push_back(nextpitch);
				while (nextpitch >= 12) nextpitch -= 12;
				nextOctavesInScore.push_back(nextpitch);
			}
		}
		for (set<int>::iterator it = pitchesInScore.begin(); it != pitchesInScore.end(); it++){
			int nowpitch = *it;
			while (nowpitch >= 12) nowpitch -= 12;
			currOctavesInScore.push_back(nowpitch);
		}
		Correctness correctness1 = *correctness;
		bool DeletePitch = true;
		for (vector<int>::iterator it = correctness->excess.begin(); it != correctness->excess.end();) {
			int repeatLoc = count(AllLocation.begin(), AllLocation.end(), location);
			if (repeatLoc >= 4){
				for (map<int, bool>::iterator it = LocationMap.begin(); it != LocationMap.end(); it++){
					if (location == it->first){
						if (it->second){
							DeletePitch = false;
							it->second = false;
						}
					}
				}
			}
			// 是否是上一个位置的延音
			bool findInLast = find(lastPitchesInScore.begin(), lastPitchesInScore.end(), *it) != lastPitchesInScore.end();
			bool findInNext = find(nextPitchesInScore.begin(), nextPitchesInScore.end(), *it) != nextPitchesInScore.end();
			// 是否是当前位置的倍频错误
			int currOctave = *it;
			while (currOctave >= 12) currOctave -= 12;
			bool findInLastOctave = find(lastOctavesInScore.begin(), lastOctavesInScore.end(), currOctave) != lastOctavesInScore.end();
			bool findInOctave = find(currOctavesInScore.begin(), currOctavesInScore.end(), currOctave) != currOctavesInScore.end();
			bool findInNextOctave = find(nextOctavesInScore.begin(), nextOctavesInScore.end(), currOctave) != nextOctavesInScore.end();
			bool isSureFlag = (sfResult[1][i][0] == 0 && i>0 && sfResult[1][i-1][0]!=1);
			
			bool isMutifreq = false;
			for (vector<int>::iterator it2 = correctness->intersection.begin(); it2 !=correctness->intersection.end(); it2++){
				for (map<int, vector<int>>::iterator it3 = MulitiFrq.begin(); it3 != MulitiFrq.end(); it3++){
					if (it3->first == *it2){
						for (int k = 0; k < it3->second.size(); k++){
							if (*it == it3->second[k]){
								isMutifreq = true;
								break;
							}
						}
					}
				}
			}
			int counts = 0;  // 记录多弹音符出现的次数
			int index1 = 0;  // 当前音符在totalCorrecness的位置
			int distance = 0;  //用来记录当前多弹音符在上一次弹该音符之间的距离
			for (vector<Correctness>::iterator it2 = totalCorrectness1.begin(); it2!=totalCorrectness1.end(); it2++){
				index1++;
				for (vector<int>::size_type j = 0; j < it2->excess.size(); j++){
					if (it2->excess[j] == *it)
						counts++;
				}
				for (vector<int>::size_type j = 0; j < it2->intersection.size(); j++){
					if (it2->intersection[j] == *it)
						counts++;
				}
			}
			bool Keep = true;
			for (int j = index1-1;Keep && j>=0; j--) {
				distance++;
				if (j < sfResult[0].size()){
					for (int z = 1; z < sfResult[0][j].size(); z += 2){
						if (*it == sfResult[0][j][z]){
							Keep = false;
							break;
						}
					}
				}
			}
			double PrevHpeakerMax = 0.0;
			double NowHpeakerMax = 0.0;

			for (vector<int>::size_type j = 1; counts-1 < timePitchesPair[*it - 1].size() && j < timePitchesPair[*it - 1][counts-1].size(); j+=2){
				if (PrevHpeakerMax < timePitchesPair[*it - 1][counts-1][j])
					PrevHpeakerMax = timePitchesPair[*it - 1][counts-1][j];
			}
			for (vector<int>::size_type j = 1; counts < timePitchesPair[*it - 1].size() && j < timePitchesPair[*it - 1][counts].size(); j+=2){
				if (NowHpeakerMax < timePitchesPair[*it - 1][counts][j])
					NowHpeakerMax = timePitchesPair[*it - 1][counts][j];
			}
			bool MinusKeepPitch = ((NowHpeakerMax * 2) < PrevHpeakerMax && distance<=7);
			//cout << *it << "..." << PrevHpeakerMax << "..." << NowHpeakerMax << "...."<<MinusKeepPitch<<endl;
			// 是否是上一个位置的倍频错误
			if ((findInLast || findInLastOctave || findInOctave || findInNext || findInNextOctave || isSureFlag || isMutifreq||MinusKeepPitch) && DeletePitch) {
				it = correctness->excess.erase(it);
			}
			else ++it;
			
		}
		for (vector<int>::iterator it = correctness1.excess.begin(); it != correctness1.excess.end();) {
			// 是否是上一个位置的延音
			bool findInLast = find(lastPitchesInScore.begin(), lastPitchesInScore.end(), *it) != lastPitchesInScore.end();
			bool findInNext = find(nextPitchesInScore.begin(), nextPitchesInScore.end(), *it) != nextPitchesInScore.end();
			// 是否是当前位置的倍频错误
			int currOctave = *it;
			while (currOctave >= 12) currOctave -= 12;
			bool findInLastOctave = find(lastOctavesInScore.begin(), lastOctavesInScore.end(), currOctave) != lastOctavesInScore.end();
			bool findInOctave = find(currOctavesInScore.begin(), currOctavesInScore.end(), currOctave) != currOctavesInScore.end();
			bool findInNextOctave = find(nextOctavesInScore.begin(), nextOctavesInScore.end(), currOctave) != nextOctavesInScore.end();
			bool isSureFlag = (sfResult[1][i][0] == 0 && i>0 && sfResult[1][i - 1][0] != 1);  // 若当前多检音符是不确定定位并且上一次定位也为不确定定位，则删除多弹音符

			bool isMutifreq = false;   // 当前音符是否为正确弹奏音符的倍频，若是，则删除多弹音符
			for (vector<int>::iterator it2 = correctness->intersection.begin(); it2 != correctness->intersection.end(); it2++){
				for (map<int, vector<int>>::iterator it3 = MulitiFrq.begin(); it3 != MulitiFrq.end(); it3++){
					if (it3->first == *it2){
						for (int k = 0; k < it3->second.size(); k++){
							if (*it == it3->second[k]){
								isMutifreq = true;
								break;
							}
						}
					}
				}
			}
			// 是否是上一个位置的倍频错误
			if ((findInLast || findInLastOctave || findInOctave || findInNext || findInNextOctave || isSureFlag || isMutifreq) && DeletePitch) {
				it = correctness1.excess.erase(it);
			}
			else ++it;

		}
		totalCorrectness1.push_back(correctness1);
		totalCorrectness.push_back(*correctness);
	}
	return totalCorrectness;
}
vector<Correctness> EvaluateSfResult::EvaluateCorrectnessOrigin(int maxPitchesInFrame)
{
    // 存放演奏正确性评价表，每一行表示当前位置的正确性
    vector<Correctness> totalCorrectness;
    vector<int> isPlayed(maxPitchesInFrame, 0);
    // 遍历sfResult的每一个位置
    for (vector<int>::size_type i = 0; i < sfResult[2].size(); ++i) {
        // 评价乐谱中第iEventScore个位置的演奏正确性
        int location = static_cast<int>(sfResultLocate[i]); // 当前演奏的音符在乐谱中的位置

        // 遍历当前位置演奏的音符
        set<int> pitchesPerformance;
        for (vector<int>::size_type j = 1; j < sfResult[0][i].size(); j += 2) {
            pitchesPerformance.insert(static_cast<int>(sfResult[0][i][j]));
        }
        // 与乐谱中当前位置的标准音符对比，是否正确演奏，漏弹，多弹，错弹
        set<int> pitchesInScore;
		for (vector<int>::size_type j = 0; location <= scoreEvent.size() && j < scoreEvent[location - 1][g_Pitches].size(); ++j) {
            pitchesInScore.insert(static_cast<int>(scoreEvent[location - 1][g_Pitches][j]));
        }

        Correctness* correctness = new Correctness();
        // 判断是否是回弹，我们认为当前定位比上一次定位值小就出现了回弹
        if (i > 0 && sfResultLocate[i] - sfResultLocate[i-1] < 0) { // 回弹
            correctness->jumpback = -1;
        } else if (i > 0 && sfResultLocate[i] == sfResultLocate[i-1]) { // 重弹上一个位置
            correctness->jumpback = 0;
        } else if (i > 0 && sfResultLocate[i] ==sfResultLocate[i-1]+1) { // 顺着弹下一个位置
            correctness->jumpback = 1;
        } else if (i > 0 && sfResultLocate[i] > sfResultLocate[i-1] + 1) { // 跳弹后面的位置
            correctness->jumpback = 2;
        } else if (i == 0) {
            correctness->jumpback = 1;
        }

        // 存多弹的音符
        int difference2[10];
        int* end = set_difference(pitchesPerformance.begin(), pitchesPerformance.end(), pitchesInScore.begin(),
                             pitchesInScore.end(), difference2);
        for (int *p = difference2; p != end; ++p) {
            correctness->excess.push_back(*p);
        }

        int intersection[20]; // 交集，预分配20大小，演奏的音符和乐谱中相应位置的音符交集不会超过20个音符
        end = set_intersection(pitchesInScore.begin(), pitchesInScore.end(), pitchesPerformance.begin(),
                                    pitchesPerformance.end(), intersection);
        for (int *p = intersection; p != end; ++p) {
            correctness->intersection.push_back(*p);
        }

        // 存下弹了哪些音符
        for (vector<int>::size_type j = 0; j < correctness->intersection.size(); ++j) {
            set<int>::iterator it = pitchesInScore.find(correctness->intersection[j]);
            if (it != pitchesInScore.end()) {
                int index = distance(pitchesInScore.begin(), it);
                if (isPlayed[index] == 1) {
                    correctness->excess.push_back(correctness->intersection[j]);
                } else {
                    isPlayed[index] = 1;
                }
            }
        }

        // 存漏弹的音符
        if ((i < sfResult[2].size()-1 && sfResultLocate[i] != sfResultLocate[i+1]) || i == sfResult[2].size()-1) {
            for (vector<int>::size_type j = 0; j < isPlayed.size() && j < pitchesInScore.size(); ++j) {
                if (isPlayed[j] == 0) { // 漏弹
                    set<int>::iterator it = pitchesInScore.begin();
                    for (int k = 0; k < j; ++k) ++it;
                    correctness->omission.push_back(*it);
                }
            }

            // 清空isPlayed
            fill(isPlayed.begin(), isPlayed.end(), 0);
        }

//        int difference1[10]; // 差集，预分配10大小
//        end = set_difference(pitchesInScore.begin(), pitchesInScore.end(), pitchesPerformance.begin(),
//                             pitchesPerformance.end(), difference1);
//        for (int *p = difference1; p != end; ++p) {
//            correctness->omission.push_back(*p);
//        }


        totalCorrectness.push_back(*correctness);
    }
    return totalCorrectness;
}


bool EvaluateSfResult::CheckJumpBack(int i)
{
    // 如果当前位置的定位比上一个位置定位小，我们不能直接判断回弹了
    // 常见的回弹定位表现为：9 5 6 7 8...说明从5开始回弹
    // 我们的策略定位b < a && c=b+1 && d=c+1
    if (i > sfResult[2].size() - 3)  return false; // 后面音符不足两个了，没有足够信息认为回弹，所以判定为没有回弹

    if ((sfResult[2][i+2][0] == sfResult[2][i+1][0]+1) && (sfResult[2][i+1][0] == sfResult[2][i][0]+1)) {
        return true;
    } else {
        return false;
    }
}

vector<BeatRhythm> EvaluateSfResult::EvaluateBeatRhythm()
{
	vector<Correctness> correctness = EvaluateCorrectnessModify();
    // 保存每个小节演奏起止时间
    vector<BeatRhythm> beatRhythm(barFirst.size());
    for (vector<int>::size_type i = 0; i < sfResult[2].size(); ++i) {
        int beatIndex = 0; // 得到是第几小节
		while (beatIndex < barFirst.size() && !(sfResultLocate[i] >= barFirst[beatIndex] && sfResultLocate[i] <= barEnd[beatIndex])) {
            ++beatIndex;
        }

        int begin = i; // 保存sfResult中一个小节开始时的索引
        int end = i; // 更新遍历索引，得到sfResult中一个小节结束时的索引
        // 判断下一个位置是否还在这一小节内，如果在这一小节内一直往后走，直到下一个位置跳出了这一小节
		while (end < sfResult[2].size() - 1 && sfResultLocate[end] >= barFirst[beatIndex] && sfResultLocate[end] <= barEnd[beatIndex]) {
            ++end;
        }
        // 此时end已经指向了下一个小节的节首了，我们计算一个小节的时长就是下一个小节节首减去这个小节节首（默认认为下个小节节首就是这个小节结尾）

        // 通过小节开始和结束时的sfResult信息，计算出一个小节的时间
        // sfResult[0][i]保存的是第i个位置的开始时间，取最小值就是整个的开始时间，所以将i分别设置为begin和end就能知道开始结束时间了
        double startTime = sfResult[0][begin][0];
        for (vector<int>::size_type j = 0; j < sfResult[0][begin].size(); j += 2) {
            if (sfResult[0][begin][j] < startTime) {
                startTime = sfResult[0][begin][j];
            }
        }
        beatRhythm[beatIndex].start = startTime;
        double endTime = sfResult[0][end][0];
        for (vector<int>::size_type j = 0; j < sfResult[0][end].size(); j += 2) {
            if (sfResult[0][end][j] < endTime) {
                endTime = sfResult[0][end][j];
            }
        }
        beatRhythm[beatIndex].end = endTime;
        // 记录下小节持续时间，如果小节出现多次，记录小节平均时长
        if (beatRhythm[beatIndex].during == 0) { // 之前没有出现过这一小节
            beatRhythm[beatIndex].during = beatRhythm[beatIndex].end - beatRhythm[beatIndex].start;
        } else { // 之前计算过了这一小节的时间，说明回弹了，计算多次的平均时间
            beatRhythm[beatIndex].during = (beatRhythm[beatIndex].during + beatRhythm[beatIndex].end - beatRhythm[beatIndex].start) / 2;
        }
		map<int, int>::iterator iter = barPair.begin();
		for (; iter != barPair.end();++iter){
			if (iter->first == beatIndex){
				beatRhythm[beatIndex].beatnum = iter->second;
			}
		}
        // 更新遍历索引i
        i = static_cast<unsigned int>(end);
        if (end != sfResult[2].size()-1) --i; // 由于for循环会++i，所以这里要减去1，还是会指向下一个小节节首
    }

    double avgTime = 0;
    int validCount = 0; // 有效的小节计数，那些没有演奏的小节无效，小节时间与其他小节差距较大的也无效
    // 小节的平均时间计算方法修改一下，去掉哪些时长与大多数时长差距较大的（设置为差距超过2s的小节）
    vector<double> validTime;
    for (vector<int>::size_type i = 0; i < beatRhythm.size(); ++i) {
        // 去除掉那些没有被检测到演奏了的小节，只计算被检测到演奏了的小节的平均时间
        if (beatRhythm[i].during != 0) {
            validTime.push_back(beatRhythm[i].during);
        }
    }
    // 统计每个小节与其他几个小节的时间差距超过2s，如果有超过总小节数一半的数量，说明这个小节与其他小节时间有差距，不应该把他计算到平均时长中去
    vector<int> diffCount(validTime.size(), 0);
    for (vector<int>::size_type i = 0; i < validTime.size(); ++i) {
        for (vector<int>::size_type j = 0; j < validTime.size(); ++j) {
            if (fabs(validTime[i] - validTime[j]) > beatMaxDiffTime) { // 与其他小节差距超过1s，记下来，统计
                ++diffCount[i];
            }
        }
    }
    for (vector<int>::size_type i = 0; i < diffCount.size(); ++i) {
        if (diffCount[i] < diffCount.size() / 2) { // 这个小节没有与一半以上的小节差距比较大，说明有效
            avgTime += validTime[i];
            ++validCount;
        }
    }
    if (validCount == 0) avgTime = 0;
    else avgTime /= validCount;
    beatAvgTime = avgTime;

    double toleranceTime = avgTime * beatToleranceRate; // 容许误差时间为平均时间乘上一个系数
    beatToleranceTime = toleranceTime;
    // 保存每个小节演奏快慢
    for (vector<int>::size_type i = 0; i < beatRhythm.size(); ++i) {
        // 去除掉那些没有被检测到演奏了的小节，只计算被检测到演奏了的小节的平均时间
        if (beatRhythm[i].during != 0) {
            if (fabs(beatRhythm[i].during - avgTime) < toleranceTime) { // 在容许时间之内，认为节奏正确，值为0
                beatRhythm[i].progress = 0;
            } else if (beatRhythm[i].during - avgTime > toleranceTime) { // 在容许时间之外，演奏时间更长了，表示慢了，值为-1
                beatRhythm[i].progress = -1;
            } else if (avgTime - beatRhythm[i].during > toleranceTime) { // 在容许时间之外，演奏时间更短了，表示快了，值为1
                beatRhythm[i].progress = 1;
            }
        }
    }

    // 第一个小节和最后一个小节的节奏默认为正确
    // 由于录音数据可能很早就开始，导致第一个小节持续时间非常长，经常第一小节会被误判为弹慢了
    // 最后一个小节由于我们的策略不能正确的计算出小节的实际演奏时长，会被误判为弹快了
    if (!beatRhythm.empty()) {
        beatRhythm.front().progress = 0;
        beatRhythm.back().progress = 0;
    }
	//当有音符多弹或者回弹的情况，不考虑多弹回弹的小节和下一个小节的节奏评价
	vector<int> WrongBeat;  //记录多弹或者回弹的小节
	for (vector<int>::size_type i = 0; i < correctness.size(); ++i) {
		if (correctness[i].jumpback == 1) { // 回弹了
			int beatnum = 0;
			while (beatnum< barFirst.size() && !(sfResultLocate[i] >= barFirst[beatnum] && sfResultLocate[i] <= barEnd[beatnum])){
				++beatnum;
			}
			WrongBeat.push_back(beatnum);
		}
		else if (!correctness[i].excess.empty()) { // 多弹了音符或者没有弹
			int beatnum = 0;
			while (beatnum< barFirst.size() && !(sfResultLocate[i] >= barFirst[beatnum] && sfResultLocate[i] <= barEnd[beatnum])){
				++beatnum;
			}
			WrongBeat.push_back(beatnum);
		}
	}
	map<int,bool> AllLocationMap;     //下面操作是当出现两次相同定位时，且第一个位置的音符是上两次定位音符的倍频，则不考虑这个小节和下一小节的音符评价
	for (int i = 0; i < sfResult[0].size(); i++){
		AllLocationMap.insert(pair<int, bool>(sfResultLocate[i], true));
	}
	for (vector<int>::size_type i = 0; i <sfResultLocate.size(); i++){
		int repeatLoc = count(sfResultLocate.begin(), sfResultLocate.end(), sfResultLocate[i]);
		if (repeatLoc >= 2 && sfResult[0][i].size()==2){
			for (map<int,bool>::iterator it = AllLocationMap.begin(); it!=AllLocationMap.end(); it++){
				if (it->first == sfResultLocate[i] && it->second){
					vector<int> lastOctive;
					for (vector<int>::size_type j = i - 2; j >= 0 && j <= i - 1; j++){
						for (vector<int>::size_type k = 1; k < sfResult[0][j].size(); k += 2){
							lastOctive.push_back(static_cast<int>(sfResult[0][j][k]) % 12);
						}
					}
					int octive = static_cast<int>(sfResult[0][i][1]) % 12;
					bool findInLast = find(lastOctive.begin(), lastOctive.end(), octive) != lastOctive.end();
					if (findInLast){
						it->second = false;
						int beatnum = 0;
						while (beatnum< barFirst.size() && !(sfResultLocate[i] >= barFirst[beatnum] && sfResultLocate[i] <= barEnd[beatnum])){
							++beatnum;
						}
						WrongBeat.push_back(beatnum);
					}
				}
			}
		}
	}
	map<double, int> dataMap;
	for (vector<int>::size_type i = 0; i < RhyTime.size(); i++){
		dataMap[RhyTime[i]]++;
	}
	int counts = 0;
	double MaxTime;
	for (map<double,int>::iterator it = dataMap.begin(); it !=dataMap.end(); it++){
		if (it->second > counts){
			counts = it->second;
			MaxTime = it->first;
		}
	}
	for (int i = 0; i <RhyTime.size() ; i++){
		if (RhyTime[i] < MaxTime){
			WrongBeat.push_back(i);
		}
	}
	for (vector<int>::size_type i = 0; i < beatRhythm.size(); i++){
		vector<int>::iterator it = find(WrongBeat.begin(), WrongBeat.end(), i + 1);
		if (it != WrongBeat.end()){
			beatRhythm[i].progress = 0;
			if (i + 1 < beatRhythm.size())
				beatRhythm[i + 1].progress = 0;	// 将节奏评价快慢置为零
		}
	}
    return beatRhythm;
}

vector<BeatRhythm> EvaluateSfResult::EvaluateBeatRhythmRealtime(int newLocation, double onset, int& newBeatIndex, vector<vector<int>> &PitchesPair)
{
	
	int sfResultSize = 0;
	vector<vector<vector<double>>> sfResult1 = scoreFollowing.GetSfResult();
	vector<int> sfResultLocate1 = scoreFollowing.GetSfResultLocate();
	if (sfResult1[2].size() > 0)
		sfResultSize = sfResult1[2].size() - 1;
	vector<vector<int>> Pitchespair;
	for (vector<int>::size_type i = 0; i < PitchesPair.size(); i++){
		if (!PitchesPair[i].empty())
			Pitchespair.push_back(PitchesPair[i]);
	}
	set<int> pitchesPerformance;
	for (vector<int>::size_type j = 0; sfResultSize< Pitchespair.size() && j < Pitchespair[sfResultSize].size(); j++) {
		pitchesPerformance.insert(static_cast<int>(Pitchespair[sfResultSize][j]));
	}

	//if (sfResultSize > 0 && sfResult1[2][sfResultSize][0] == sfResult1[2][sfResultSize - 1][0]) {
	//	// 上个定位结果的正确音符为 totalCorrectness.back().intersection
	//	if (RealtotalCorrectness.size() > 0){
	//		for (vector<int>::size_type j = 0; j < RealtotalCorrectness.back().intersection.size(); ++j) {
	//			pitchesPerformance.insert(RealtotalCorrectness.back().intersection[j]);
	//		}
	//	}
	//}

	set<int> pitchesInScore;
	for (vector<int>::size_type j = 0; newLocation<=scoreEvent.size()&& j < scoreEvent[newLocation - 1][g_Pitches].size(); ++j) {
		pitchesInScore.insert(static_cast<int>(scoreEvent[newLocation - 1][g_Pitches][j]));
	}

	Correctness* correctness = new Correctness();
	// 判断是否是回弹，我们认为当前定位比上一次定位值小就出现了回弹
	if (sfResultSize>0 && sfResultLocate1[sfResultSize] - sfResultLocate1[sfResultSize - 1] < 0) {
		correctness->jumpback = 1;
	}
	else {
		correctness->jumpback = 0;
	}

	int intersection[20]; // 交集，预分配20大小，演奏的音符和乐谱中相应位置的音符交集不会超过20个音符
	int *end = set_intersection(pitchesInScore.begin(), pitchesInScore.end(), pitchesPerformance.begin(),
		pitchesPerformance.end(), intersection);
	for (int *p = intersection; p != end; ++p) {
		correctness->intersection.push_back(*p);
	}

	int difference1[10]; // 差集，预分配10大小
	end = set_difference(pitchesInScore.begin(), pitchesInScore.end(), pitchesPerformance.begin(),
		pitchesPerformance.end(), difference1);
	for (int *p = difference1; p != end; ++p) {
		correctness->omission.push_back(*p);
	}
	int difference2[10];
	end = set_difference(pitchesPerformance.begin(), pitchesPerformance.end(), pitchesInScore.begin(),
		pitchesInScore.end(), difference2);

	for (int *p = difference2; p != end; ++p) {
		correctness->excess.push_back(*p);
	}

	vector<int> lastPitchesInScore; // 上一个位置的音符
	vector<int> lastOctavesInScore; // 上一个位置的octave
	vector<int> currOctavesInScore; // 当前位置的octave


	for (vector<int>::size_type j = 0; newLocation>1 && newLocation-2<scoreEvent.size()&& j < scoreEvent[newLocation - 2][g_Pitches].size(); j++) {
		int lastpitch = static_cast<int>(scoreEvent[newLocation - 2][g_Pitches][j]);
		lastPitchesInScore.push_back(lastpitch);
		while (lastpitch >= 12) lastpitch -= 12;
		lastOctavesInScore.push_back(lastpitch);
	}

	for (set<int>::iterator it = pitchesInScore.begin(); it != pitchesInScore.end(); it++){
		int nowpitch = *it;
		while (nowpitch >= 12) nowpitch -= 12;
		currOctavesInScore.push_back(nowpitch);
	}

	for (vector<int>::iterator it = correctness->excess.begin(); it != correctness->excess.end();) {
		// 是否是上一个位置的延音
		bool findInLast = find(lastPitchesInScore.begin(), lastPitchesInScore.end(), *it) != lastPitchesInScore.end();
		// 是否是当前位置的倍频错误
		int currOctave = *it;
		while (currOctave >= 12) currOctave -= 12;
		bool findInLastOctave = find(lastOctavesInScore.begin(), lastOctavesInScore.end(), currOctave) != lastOctavesInScore.end();
		bool findInOctave = find(currOctavesInScore.begin(), currOctavesInScore.end(), currOctave) != currOctavesInScore.end();

		// 是否是上一个位置的倍频错误
		if (findInLast || findInLastOctave || findInOctave) {
			it = correctness->excess.erase(it);
		}
		else ++it;
	}
	RealtotalCorrectness.push_back(*correctness);


	// 得到是第几小节
	int beatIndex = 0;
	while (beatIndex < barFirst.size() && !(newLocation >= barFirst[beatIndex] && newLocation <= barEnd[beatIndex])) {
		++beatIndex;
	}
	newBeatIndex = -1;
	if (realtimeBeatIndex != beatIndex) {
		// 认为上一个小节结束
		newBeatIndex = realtimeBeatIndex; // 通知外部新演奏的小节的索引
		realtimeBeatRhythm[realtimeBeatIndex].end = onset; // 记录下新演奏的小节的结束时间

		// 求前面小节时长的平均值，大致评价节奏
		int validCount = 0;
		double avgTime = 0;
		for (vector<int>::size_type i = 0; i < realtimeBeatRhythm.size(); ++i) {
			if (realtimeBeatRhythm[i].during != 0) {
				++validCount;
				avgTime += realtimeBeatRhythm[i].during;
			}
		}
		if (validCount != 0) avgTime /= validCount;

		// 存下当前小节持续时间（先计算上面的平均时间，再计算这个持续时间，以免把当前小节的持续时间也算入了平均时间）
		realtimeBeatRhythm[realtimeBeatIndex].during = realtimeBeatRhythm[realtimeBeatIndex].end -
			realtimeBeatRhythm[realtimeBeatIndex].start;

		double toleranceTime = avgTime * beatToleranceRate; // 容许误差时间为平均时间乘上一个系数
		bool flag = false;
		for (int i = 0; i < RealtotalCorrectness.size(); i++){
			if (RealtotalCorrectness[i].jumpback == 1 || (!RealtotalCorrectness[i].excess.empty())){
				flag = true;
				break;
			}
		}
		if (flag){
			realtimeBeatRhythm[realtimeBeatIndex].progress = 2;    // 当出现回弹和多弹 ，progress=2，评价为bad
		}
		else if (fabs(realtimeBeatRhythm[realtimeBeatIndex].during - avgTime) > toleranceTime)
			realtimeBeatRhythm[realtimeBeatIndex].progress = 1;    //  当小节节奏不正确时，节奏性错误，评价为good
		else
			realtimeBeatRhythm[realtimeBeatIndex].progress = 0;   // 当前面两种情况都不满足时，评价为perfect

		// 持续时间与容许误差时间相比，判断小节演奏快慢
		//if (fabs(realtimeBeatRhythm[realtimeBeatIndex].during - avgTime) < toleranceTime) { // 在容许时间之内，认为节奏正确，值为0
		//    realtimeBeatRhythm[realtimeBeatIndex].progress = 0;
		//} else if (realtimeBeatRhythm[realtimeBeatIndex].during - avgTime > toleranceTime) { // 在容许时间之外，演奏时间更长了，表示慢了，值为-1
		//    realtimeBeatRhythm[realtimeBeatIndex].progress = -1;
		//} else if (avgTime - realtimeBeatRhythm[realtimeBeatIndex].during > toleranceTime) { // 在容许时间之外，演奏时间更短了，表示快了，值为1
		//    realtimeBeatRhythm[realtimeBeatIndex].progress = 1;
		//}


		// 更新所在小节的索引
		realtimeBeatIndex = beatIndex;
		// 下一小节的开始时间就是这里（这里是设置当前小节的结束时间，同时设置下一个小节的开始时间，所以第一个小节的开始时间没有设置过，默认为0）
		realtimeBeatRhythm[realtimeBeatIndex].start = onset;
		vector<Correctness>().swap(RealtotalCorrectness);
	}

	return realtimeBeatRhythm;
}

vector<int> EvaluateSfResult::EvaluateNoteRhythm()
{
    // 存放每个位置演奏节奏评价表，每一行表示当前位置演奏的时间与乐谱相比是快了还是慢了
    vector<int> noteRhythm;

    if (sfResult[0].empty()) return noteRhythm;
    // 存放乐谱中相邻两个位置之间的onset时间差
    vector<double> durInScore;
    for (vector<int>::size_type i = 0; i < scoreEvent.size(); ++i) {
        double diff = 0;
        if (i < scoreEvent.size() - 1) { // 做差分，最后一个位置没有时间差
            diff = scoreEvent[i+1][0][0] - scoreEvent[i][0][0];
        } else { // 最后一个位置默认设置为1
            diff = 1;
        }
        durInScore.push_back(diff);
    }

    vector<double> dur;
    vector<double> durRate;
    double currMinOnset = sfResult[0][0][0]; // 当前位置的onset时间，初始化为第一个onset值，然后通过比较找最小值
    // 找到第一个定位sfResult[0][0]中的onset最小值
    for (vector<int>::size_type i = 0; !sfResult[0].empty() && i < sfResult[0][0].size(); i += 2) {
        if (sfResult[0][0][i] < currMinOnset) {
            currMinOnset = sfResult[0][0][i];
        }
    }
    // 遍历sfResult的每一个位置，存放演奏的每个小节的耗时比例
    for (vector<int>::size_type i = 0; i < sfResult[2].size(); ++i) {
        if (i < sfResult[2].size() - 1) { // 还没到最后一个位置，还有下一个位置
            // 遍历下一个位置演奏的音符的onset时间，找到一个最小值
            double nextMinOnset = sfResult[0][i+1][0]; // 下一个位置的onset时间，初始化为第一个onset值，然后通过比较找最小值
            for (vector<int>::size_type j = 0; j < sfResult[0][i+1].size(); j += 2) {
                if (sfResult[0][i+1][j] < nextMinOnset) {
                    nextMinOnset = sfResult[0][i+1][j];
                }
            }

            if (sfResult[2][i][0] == sfResult[2][i+1][0]) { // 如果当前定位和下一个定位相同，累计到下一个不同定位再计算onset间隔
                dur.push_back(nextMinOnset - currMinOnset);
                durRate.push_back(0);
            } else {
                double currDur = nextMinOnset - currMinOnset; // 保存当前位置的onset时间和下一个位置onset时间差值
                currMinOnset = nextMinOnset; // 更新当前的最小onset值，为下一个位置做准备

                dur.push_back(currDur);

                // 当前的比例，计算方式是当前位置耗时比上对应位置的耗时
                double currRate = currDur / durInScore[static_cast<int>(sfResult[2][i][0]) - 1];
                durRate.push_back(currRate);
            }
        } else { // 最后一个位置默认设置比例符合
            dur.push_back(0);
            durRate.push_back(0);
        }
    }

    double avgDurRate = 0;
    double validCount = 0;
    for (vector<int>::size_type i = 0; i < durRate.size(); ++i) {
        if (durRate[i] != 0) { // 去掉无效的位置（当前位置和下一个位置相同，检测成了两个）
            avgDurRate += durRate[i];
            ++validCount;
        }
    }
    if (validCount != 0) avgDurRate /= validCount;

    double tolerance = noteToleranceRate * avgDurRate;
    for (vector<int>::size_type i = 0; i < durRate.size(); ++i) {
        if (durRate[i] == 0) noteRhythm.push_back(0); // 当前位置和下一个位置定位相同，看下一个
        else if (fabs(durRate[i] - avgDurRate) < tolerance) noteRhythm.push_back(0); // 当前位置耗时比例在容许误差内，节奏正确
        else if (durRate[i] - avgDurRate > tolerance) noteRhythm.push_back(-1); // 当前位置耗时比例比平均耗时比例大，说明耗时多，慢了
        else if (avgDurRate - durRate[i] > tolerance) noteRhythm.push_back(1); // 当前位置耗时比例比平均耗时比例小，说明耗时少，快了
    }
    return noteRhythm;
}

double EvaluateSfResult::CountScore(vector<Correctness> correctness, vector<BeatRhythm> beatRhythm)
{
	if (correctness.empty() || beatRhythm.empty()) return 0;

	// 对每个位置演奏音符的正确性评分
	vector<int> WrongBeat;
	double correctnessScore = 100;
	double scorePerEvent = correctnessScore / correctness.size(); // 前面进行了为空判断，可以直接除
	int jumpbackCount = 0;
	for (vector<int>::size_type i = 0; i < correctness.size(); ++i) {
		if (correctness[i].jumpback == 1) { // 回弹了
			int beatnum = 0;
			while (beatnum< barFirst.size() && !(sfResultLocate[i] >= barFirst[beatnum] && sfResultLocate[i] <= barEnd[beatnum])){
				++beatnum;
			}
			WrongBeat.push_back(beatnum);
			correctnessScore -= scorePerEvent;
			++jumpbackCount;
		}
		else if (!correctness[i].excess.empty() /*|| correctness[i].intersection.empty()*/) { // 多弹了音符或者没有弹
			int beatnum = 0;
			while (beatnum< barFirst.size() && !(sfResultLocate[i] >= barFirst[beatnum] && sfResultLocate[i] <= barEnd[beatnum])){
				++beatnum;
			}
			WrongBeat.push_back(beatnum);
			correctnessScore -= scorePerEvent;
		}
	}


	// 对节奏评分
	double beatRhythmScore = 100;
	double scorePerBeat = beatRhythmScore / beatRhythm.size(); // 前面进行了为空判断，可以直接除
	int validBeatCount = static_cast<int>(beatRhythm.size()); // 计算有效小节的数目，演奏了几个小节
	for (vector<int>::size_type i = 0; i < beatRhythm.size(); ++i) {
		vector<int>::iterator it = find(WrongBeat.begin(), WrongBeat.end(), i + 1);
		if (it != WrongBeat.end()){
			continue;
		}
		if (beatRhythm[i].during == 0) { // 如果这一小节没有弹
			beatRhythmScore -= scorePerBeat;
			--validBeatCount;
		}
		else if (beatRhythm[i].progress != 0) { // 如果这一小节节奏不正确
			beatRhythmScore -= scorePerBeat;
		}
	}

	

	double finalScore = 100;
	double beatcorrect = 100 - static_cast<double>(static_cast<double>(beatRhythmScore) / static_cast<double>(100)) * 100;
	double scorecorrect = 100 - static_cast<double>(static_cast<double>(correctnessScore) / static_cast<double>(100)) * 100;

	if (validBeatCount > beatRhythm.size() / 2){
		if (scorecorrect == 0){
			if (beatcorrect >= 0 && beatcorrect <= 10)
				finalScore = 5;
			else if (beatcorrect > 10 && beatcorrect <= 40)
				finalScore = 4;
			else
				finalScore = 3;
		}
		else if (scorecorrect > 0 && scorecorrect <= 10){
			if (beatcorrect >= 0 && beatcorrect <= 20)
				finalScore = 4;
			else if (beatcorrect > 20 && beatcorrect <= 50)
				finalScore = 3;
			else
				finalScore = 2;
		}
		else if (scorecorrect > 10 && scorecorrect <= 30){
			if (beatcorrect >= 0 && beatcorrect <= 40)
				finalScore = 3;
			else
				finalScore = 2;
		}
		else if (scorecorrect > 30 && scorecorrect <= 50){
			finalScore = 2;
		}
		else
			finalScore = 1;
	}
	else{
		finalScore = 1;
	}

	//   // 最后的打分是两个评分乘上一个权值
	//   double finalScore = beatRhythmScore*0.3 + correctnessScore*0.7;

	//   // 对流畅性打分，如果有多个回弹，多扣分
	//if (jumpbackCount > 0)
	//	finalScore = 80;
	//   if (jumpbackCount >= 8) { // 超过8个回弹最多得40分
	//       finalScore = 40;
	//   } else if (jumpbackCount >= 4) { // 4到8个回弹扣20分
	//       finalScore -= 20;
	//   } else if (jumpbackCount >= 2) { // 2到4个回弹扣10分
	//       finalScore -= 10;
	//   } else if (jumpbackCount > 0) { // 1个回弹扣5分
	//       finalScore -= 5;
	//   }

	//   if (validBeatCount < beatRhythm.size() / 2) { // 有一半以上的小节没弹，本次演奏无效，给0分
	//       finalScore = 0;
	//   } else if (finalScore < 40) { // 虽然完整的演奏了，但是最终得分不超过40分，给一个基准分40分
	//       finalScore = 40;
	//   }


	//if (correctness.empty() || beatRhythm.empty()) return 0;
	//int validBeatCount = static_cast<int>(beatRhythm.size()); // 计算有效小节的数目，演奏了几个小节
	//int correctBeatcount = static_cast<int>(beatRhythm.size());
	//for (vector<int>::size_type i = 0; i < beatRhythm.size; ++i){
	//	if (beatRhythm[i].during == 0)
	//		validBeatCount--;
	//	else if (beatRhythm[i].progress != 0)
	//		correctBeatcount--;
	//}
	//int jumpbackCount = 0;
	//int wrongCount = 0;
	//for (vector<int>::size_type i = 0; i < correctness.size(); ++i){
	//	if (correctness[i].jumpback == 1)
	//		jumpbackCount++;
	//	else if (!correctness[i].excess.empty() || correctness[i].intersection.empty())
	//		wrongCount++;
	//}

	return finalScore;
}

int EvaluateSfResult::GiveStars(double score)
{
	int starsNum = score;
	// 90分以上给5星，70到90给4星，40到70给3星，0到40分一下给两星，0分不给星
	/* if (score >= 90) {
	starsNum = 5;
	} else if (score >= 80) {
	starsNum = 4;
	} else if (score > 40) {
	starsNum = 3;
	} else if (score > 0) {
	starsNum = 2;
	} else {
	starsNum = 0;
	}*/
	return starsNum;
}

int EvaluateSfResult::SaveEvaluateResult(const string& filePath, vector<Correctness> correctness, vector<BeatRhythm> beatRhythm)
{
    ofstream fileStream(filePath);
    if (!fileStream.is_open()) {
        cout << "Error opening file evaluateResult" << endl;
        return -1;
    }

    // 写入数据到文件中
    if (correctness.empty() || beatRhythm.empty()) return 0;
    // 写入correctness正确性
    fileStream << "定位正确性" << endl;
	fileStream << "是否回弹 多弹音符1，多弹音符2 漏弹音符1，漏弹音符2 正确演奏音符1，正确演奏音符2" << endl;
    for (vector<int>::size_type i = 0; i < correctness.size(); ++i) {
        // 一行中依次写入回弹、多弹、漏弹、正确弹奏，用|隔开
        fileStream << correctness[i].jumpback << '\t';
        for (vector<int>::size_type j = 0; j < correctness[i].excess.size(); ++j) {
            fileStream << correctness[i].excess[j] << ',';
        }
        fileStream << '\t';
        for (vector<int>::size_type j = 0; j < correctness[i].omission.size(); ++j) {
            fileStream << correctness[i].omission[j] << ',';
        }
        fileStream << '\t';
        for (vector<int>::size_type j = 0; j < correctness[i].intersection.size(); ++j) {
            fileStream << correctness[i].intersection[j] << ',';
        }
        fileStream << endl;
    }

    // 写入beatRhythm小节节奏
    fileStream << endl << "小节级别节奏" << endl;
    fileStream << "节奏快慢（0表示正确、1表示快了、-1表示慢了） 小节开始时间  小节结束时间  小节时长" << endl;
    for (vector<int>::size_type i = 0; i < beatRhythm.size(); ++i) {
        // 一行中依次写入节奏快慢、小节开始时间、结束时间、时长
        fileStream << beatRhythm[i].progress << '\t';
        fileStream << beatRhythm[i].start << '\t';
        fileStream << beatRhythm[i].end << '\t';
        fileStream << beatRhythm[i].during << endl;
    }
    fileStream << "小节平均时长 avgTime = " << beatAvgTime << "s" << endl;
    fileStream << "容许误差 toleranceTime = " << beatToleranceTime << "s";

    fileStream.close();
    return 0;
}