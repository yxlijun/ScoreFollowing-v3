//
// Created by zhangqianyi on 2017/3/16.
//

#include "matchEvent.h"

int isIEvent(vector<int> pitch, const vector<vector<vector<double>>>& scoreEvent, int iEventPre, vector<int>& lia, vector<int>& locb) {
    int iEvent;
    // 乐谱当前位置演奏的音符
    vector<double> scoreEventPitches(scoreEvent[iEventPre][g_Pitches].begin(), scoreEvent[iEventPre][g_Pitches].end());
    // 比较新演奏的音符是否在乐谱当前位置演奏的音符中
    for (vector<int>::size_type i = 0; i < pitch.size(); ++i) {
        vector<double>::iterator it = find(scoreEventPitches.begin(), scoreEventPitches.end(), pitch[i]);
        if (it != scoreEventPitches.end()) {
            lia[i] = 1; // 表示当前音符存在
            locb[i] = it - scoreEventPitches.begin(); // 给出是第几个
        } else {
            lia[i] = 0;
            locb[i] = 0;
        }
    }
    bool allZeroLia = all_of(lia.begin(), lia.end(), [](int i) {return i != 0;}); // 音符全都在乐谱中，没多音符
    if (!allZeroLia) {
        iEvent = -1;
    } else {
        iEvent = iEventPre;
    }
    return iEvent;
}

int isIEventTotal(vector<int> pitch, const vector<vector<vector<double>>>& scoreEvent, int iEventPre, vector<int>& lia, vector<int>& locb){
	int iEvent;
	// 乐谱当前位置演奏的音符
	vector<double> scoreEventPitches(scoreEvent[iEventPre][g_Pitches].begin(), scoreEvent[iEventPre][g_Pitches].end());
	if (pitch.size() != scoreEventPitches.size()){
		iEvent = -1;
		fill(lia.begin(), lia.end(), 0);
		fill(locb.begin(), locb.end(), 0);
	}
	else{
		for (vector<int>::size_type i = 0; i < pitch.size(); ++i) {
			vector<double>::iterator it = find(scoreEventPitches.begin(), scoreEventPitches.end(), pitch[i]);
			if (it != scoreEventPitches.end()) {
				lia[i] = 1; // 表示当前音符存在
				locb[i] = it - scoreEventPitches.begin(); // 给出是第几个
			}
			else {
				lia[i] = 0;
				locb[i] = 0;
			}
		}
		bool allZeroLia = all_of(lia.begin(), lia.end(), [](int i) {return i != 0; }); // 音符全都在乐谱中，没多音符
		if (!allZeroLia) {
			iEvent = -1;
		}
		else {
			iEvent = iEventPre;
		}
	}
	// 比较新演奏的音符是否在乐谱当前位置演奏的音符中
	if (iEvent == -1){
		if ((pitch.size() == scoreEventPitches.size()) && (pitch.size() == 1)){
			int pitchoctive = pitch[0] % 12;
			int scorePitchOctive = static_cast<int>(scoreEventPitches[0]) % 12;
			if (pitchoctive == scorePitchOctive)
				iEvent = iEventPre;
		}
	}
	return iEvent;
}

double matchIEvent(vector<int> pitch, vector<vector<vector<double> > > scoreEvent, int iEvent) {
    if (iEvent < 0) return -1;
    unsigned int pitchSize = pitch.size();
    vector<int> octave(pitchSize);
    for (unsigned int i = 0; i < pitchSize; ++i) {
        octave[i] = pitch[i] % 12;
    }

    vector<int> distance;
    // 遍历每个音符，计算每个音符与iEvent位置的每个音符的差值，保存最小差值
    for (unsigned int i = 0; i < pitchSize; ++i) {
        vector<int> distanceTmp;
        for (unsigned int j = 0; j < scoreEvent[iEvent][g_Pitches].size(); ++j) {
            int temp = static_cast<int>(fabs(pitch[i] - scoreEvent[iEvent][g_Pitches][j]));
            distanceTmp.push_back(temp);
        }
        if (!distanceTmp.empty()) {
            int minElement = *min_element(distanceTmp.begin(), distanceTmp.end());
            distance.push_back(minElement);
        }
    }
    // 将音符差值转换为匹配程度
    double matching1 = 0;
    for (unsigned int i = 0; i < distance.size(); ++i) {
        matching1 += distanceToMatching(distance[i]);
    }
    // 计算倍频匹配程度
    double matching2 = 0;
    for (unsigned int i = 0; i < pitchSize; ++i) {
        vector<double>::iterator it = find(scoreEvent[iEvent][g_PitchesOctave].begin(), scoreEvent[iEvent][g_PitchesOctave].end(), octave[i]);
        if (it != scoreEvent[iEvent][g_PitchesOctave].end()) {
            ++matching2;
        }
    }
    double matching = 0;
    if (pitchSize != 0) {
		//matching = (matching1 * 0.6 + matching2 * 0.4) / octave.size();
		if (matching1 / octave.size() == 1) matching = 1;
		else if (matching2 / octave.size() == 1) matching = 0.95;
		else matching = (matching1 * 0.6 + matching2 * 0.4) / octave.size();
    }

    return matching;
}

double distanceToMatching(int distance) {
    double matching = 0;
    switch (distance) {
        case 0: matching = 1; break;
        case 1: matching = 0.9; break;
        case 2: matching = 0.8; break;
        case 3: matching = 0.6; break;
        default: break;
    }
    return matching;
}


void matchAllCandidate(vector<int> pitch, vector<vector<vector<double> > > scoreEvent, int beginIEvent,
                        vector<vector<double> >& matching, vector<vector<int> >& path) {
    unsigned int pitchSize = pitch.size();
    vector<int> octave(pitchSize);
    for (unsigned int i = 0; i < pitchSize; ++i) {
        octave[i] = pitch[i] % 12;
    }
    // 上次的位置候选+其后2个位置
    int len;
    if (matching.size() == 0) len = beginIEvent + 1;
    else len = (beginIEvent + matching[matching.size()-1].size() + 2) < scoreEvent.size() ? (beginIEvent + matching[matching.size()-1].size() + 2) : scoreEvent.size();
    vector<int> iEventCandidate;
    for (int i = beginIEvent; i < len; ++i) {
        iEventCandidate.push_back(i);
    }
    vector<double> matchingTmp(iEventCandidate.size());
    for (vector<int>::size_type i = 0; i < iEventCandidate.size(); ++i) {
        matchingTmp[i] = matchIEvent(pitch, scoreEvent, iEventCandidate[i]);
    }
    matching.push_back(matchingTmp);

    vector<int> iEventMatched;
    for (vector<int>::size_type i = 0; i < matching[matching.size()-1].size(); ++i) {
        if (matching[matching.size()-1][i] == 1) {
            iEventMatched.push_back(static_cast<int>(i));
        }
    }
    if (iEventMatched.empty()) {
        vector<vector<int> >().swap(path); // 清空path
    } else {
        if (path.empty()) {
            for (vector<int>::size_type i = 0; i < iEventMatched.size(); ++i) {
                vector<int> pathTmp(1); pathTmp[0] = iEventMatched[i];
                path.push_back(pathTmp);
            }
        } else {
            int nPath = path.size();
            for (int i = 0; i < nPath; ++i) {
                int pathEnd = path[i][path[i].size()-1];
                vector<int> memberVector(3);
                memberVector[0] = pathEnd; memberVector[1] = pathEnd + 1; memberVector[2] = pathEnd + 2;
                vector<int> isContinue(3);
                vector<int> locb(3);
                for (vector<int>::size_type j = 0; j < memberVector.size(); ++j) {
                    vector<int>::iterator it;
                    it = find(iEventMatched.begin(), iEventMatched.end(), memberVector[j]);
                    if (it != iEventMatched.end()) {
                        isContinue[j] = 1;
                        locb[j] = it - iEventMatched.begin();
                    } else {
                        isContinue[j] = 0;
                        locb[j] = 0;
                    }
                }
                if (any_of(isContinue.begin(), isContinue.end(), [](int x) {return x != 0;})) {
                    vector<int> iEventContinue;
                    for (vector<int>::size_type j = 0; j < isContinue.size(); ++j) {
                        if (isContinue[j] != 0) {
                            iEventContinue.push_back(iEventMatched[locb[j]]);
                        }
                    }
                    int sumIsContinue = accumulate(isContinue.begin(), isContinue.end(), 0);
                    for (int j = 0; j < sumIsContinue; ++j) {
                        vector<int> pathTemp(path[i]);
                        pathTemp.push_back(iEventContinue[j]);
                        path.push_back(pathTemp);
                    }
                }
            }
            for (int i = 0; i < nPath; ++i) {
                path.erase(path.begin()); // 删掉前面的节点
            }
        }
    }
}


void matchScore(vector<vector<int> > pitches, vector<vector<vector<double> > > scoreEvent,
                vector<vector<double> >& matching, vector<vector<int> >& path) {
    matching.resize(pitches.size());
    for (vector<int>::size_type i = 0; i < pitches.size(); ++i) {
        vector<double>().swap(matching[i]);
        for (vector<int>::size_type j = 0; j < scoreEvent.size(); ++j) {
            double matchingTemp = matchIEvent(pitches[i], scoreEvent, j);
            matching[i].push_back(matchingTemp);
        }
    }

    vector<vector<int> >().swap(path); // 清空path
    vector<int> pathEnd;
    for (vector<int>::size_type i = 0; i < matching[matching.size()-1].size(); ++i) {
        if (matching[matching.size()-1][i] == 1) {
            pathEnd.push_back(static_cast<int>(i));
        }
    }
    for (unsigned int i = 0; i < pathEnd.size(); ++i) {
        traceback(pathEnd[i], matching, matching.size() - 1, path);
    }
}

void traceback(int pathEnd, vector<vector<double> > matching, int iCol, vector<vector<int> >& path) {
    // 第iCol个演奏event对应第pathEnd个乐谱event
    if (iCol == 0) {
        path.push_back(vector<int>{pathEnd});
        return ;
    }

    vector<int> iEventCandidate;
    int from = pathEnd;
    int to = (pathEnd-2) > 0 ? (pathEnd-2) : 0;
    for (int i = from; i >= to; --i) {
        iEventCandidate.push_back(i);
    }
    vector<int> thisPathEnd;
    for (unsigned int i = 0; i < iEventCandidate.size(); ++i) {
        if (matching[iCol-1][iEventCandidate[i]] == 1) {
            thisPathEnd.push_back(iEventCandidate[i]);
        }
    }

    if (thisPathEnd.empty()) {
        path.push_back(vector<int>{pathEnd});
    } else {
        for (unsigned int i = 0; i < thisPathEnd.size(); ++i) {
            int nPathPre = path.size();
            traceback(thisPathEnd[i], matching, iCol-1, path);
            int nPathNext = path.size();
            for (int j = nPathPre; j < nPathNext; ++j) {
                path[j].push_back(pathEnd);
            }
        }
    }
}
