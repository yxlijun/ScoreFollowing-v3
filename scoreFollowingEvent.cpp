//
// Created by zhangqianyi on 2017/3/17.
//

#include "scoreFollowingEvent.h"

vector<vector<int> > scoreFollowingEvent(vector<int>& pitch, vector<vector<vector<double> > >& scoreEvent, vector<int> &sfResultOriginLocate,
                                         int iEventPre, Candidate& candidate, vector<int>& isPlayed, int& isSureFlag) {
    vector<vector<int> > iEvent;
    int isSureFlagPre = isSureFlag;
    vector<int> lia(pitch.size());
    vector<int> locb(pitch.size());

    int nEventBack = 2;         // 局部匹配时，最前的可能的位置为iEventLastSure-nEventBack
    int nEventScoreSearch = 3;  // 不确定定位的演奏次数达到nEventScoreSearch时，全谱匹配
    int maxNotSure = 10;        // 最长不确定定位长度，超过这个就直接开始动态规划
    int scoreEventSize = scoreEvent.size(); // 乐谱行数

    // 若上一演奏的音符有确定定位，对应于乐谱中第iEventPre个位置
    // 和当前位置匹配 & 未被演奏过                    确定为当前位置
    // 和当前位置匹配 &  被演奏过 & 和下一位置不匹配    确定为当前位置
    // 和当前位置匹配 &  被演奏过 & 和下一位置匹配      确定为下一位置
    // 和当前位置不匹配 & 和下一位置匹配               确定为下一位置
    // 和当前位置不匹配 & 和下一位置不匹配             不确定定位

    while (isSureFlagPre == 1) {
        int isCurr = isIEvent(pitch, scoreEvent, iEventPre, lia, locb);
		vector<int> lia2(pitch.size());
		vector<int> locb2(pitch.size());
		int isCurr1 = isIEventTotal(pitch, scoreEvent, iEventPre, lia2, locb2);
        if (isCurr != -1) {
            iEvent.push_back(vector<int>{isCurr});
        }

        vector<int> tmp(locb.size()); // 判断是否被演奏过
        for (vector<int>::size_type i = 0; i < locb.size(); ++i) {
            tmp[i] = isPlayed[locb[i]];
        }
        bool anyIsPlayed = any_of(tmp.begin(), tmp.end(), [](int i) {return i != 0;}); // 是否都未被演奏过
        if (isCurr != -1 && !anyIsPlayed) { // 和当前位置匹配，且都未被演奏过，确定为当前位置
			if (iEventPre < scoreEventSize - 1) {
				vector<int> lia2(pitch.size());
				vector<int> locb2(pitch.size());
				int isNext = isIEventTotal(pitch, scoreEvent, iEventPre + 1, lia2, locb2);
				if (isNext != -1 && isCurr1 == -1){
					isCurr = iEventPre + 1;
					vector<vector<int> >().swap(iEvent);
					iEvent.push_back(vector<int>{isCurr}); // 将iEvent换成下一个位置
					lia = lia2;
					locb = locb2;
					break;
				}
			}
            break;
        }

        // 下面的就是anyIsPlayed为真，表示有音符被演奏过
        if (iEventPre < scoreEventSize - 1) {
			if (sfResultOriginLocate.size() > 0){
				double lastlocation = sfResultOriginLocate.back();
				vector<double> pitchs;
				for (int i = 0; i <sfResultOriginLocate.size(); i++){
					pitchs.push_back(sfResultOriginLocate[i]);
				}
				int repeatnum = count(pitchs.begin(), pitchs.end(), lastlocation);
				if (repeatnum >= 2){
					bool boolmatch = false;
					for (vector<int>::size_type i = 1; i <= repeatnum; i++){
						int ieventnow = iEventPre + i;
						if (ieventnow < scoreEventSize){
							vector<int> lia2(pitch.size());
							vector<int> locb2(pitch.size());
							int isNext = isIEvent(pitch, scoreEvent, ieventnow, lia2, locb2);
							if (isNext != -1 && isCurr1 == -1){
								isCurr = ieventnow;
								vector<vector<int> >().swap(iEvent);
								iEvent.push_back(vector<int>{isCurr}); // 将iEvent换成下一个位置
								lia = lia2;
								locb = locb2;
								boolmatch = true;
								break;
							}
						}
					}
					if (boolmatch)
						break;
				}
			}
			vector<int> lia2(pitch.size());
			vector<int> locb2(pitch.size());
			int isNext = isIEvent(pitch, scoreEvent, iEventPre + 1, lia2, locb2);
			int isNext1 = isIEventTotal(pitch, scoreEvent, iEventPre + 1, lia2, locb2);

			// 分别和iEventPre、iEventPre+1是否完全匹配
			if (isNext != -1 || (isNext==-1 && isNext1!=-1)) { // 和当前位置匹配 &  被演奏过 & 和下一位置匹配      确定为下一位置
				isCurr = iEventPre + 1;
				vector<vector<int> >().swap(iEvent);
				iEvent.push_back(vector<int>{isCurr}); // 将iEvent换成下一个位置
				lia = lia2;
				locb = locb2;
				break;
			}
			else if (isCurr != -1) { // 和当前位置匹配 &  被演奏过 & 和下一位置不匹配    确定为当前位置
				break;
			}
        }

        // 都不满足，说明新演奏音符与上一个位置和当前位置都不匹配
        isSureFlag = 0;
        candidate.iEventLastSure = iEventPre;
        break;
    }

    // 若上一个位置不确定定位
    if (isSureFlag == 0) {
        candidate.pitches.push_back(pitch);
        int nNotSure = candidate.pitches.size();
//        int notSureFlag = candidate.pitches.size() - nEventScoreSearch;
        int iEventLastSure = candidate.iEventLastSure;
        int barFirst = static_cast<int>(scoreEvent[iEventLastSure][g_BarFirst][0]) - 1;
//        if (notSureFlag < 0) {
//            int from = iEventLastSure-nEventBack;
//            int to = (iEventLastSure+nEventBack*nNotSure) < (scoreEventSize-1) ? (iEventLastSure+nEventBack*nNotSure) : (scoreEventSize-1);
//            vector<double> matchingTemp;
//            for (int i = from; i <= to; ++i) {
//                matchingTemp.push_back(matchIEvent(pitch, scoreEvent, i));
//            }
//            candidate.matching.push_back(matchingTemp);
//
//            vector<int> newNode;
//            for (vector<int>::size_type i = 0; !candidate.matching.empty() && i < candidate.matching[candidate.matching.size()-1].size(); ++i) {
//                if (candidate.matching[candidate.matching.size()-1][i] == 1) {
//                    newNode.push_back(static_cast<int>(i));
//                }
//            }
//            updatePath(candidate.path, newNode);
//            // 不确定定位的长度小于5，所以不用进行全谱匹配，isMatchingAll赋为0，当确定定位有两个时可以确定整个定位了minNMatch赋为2
//            int iEventPreTemp = iEventPre - iEventLastSure + nEventBack;
//            iEvent = isSure(candidate, 2, isSureFlag, iEventPreTemp);
//        } else if (notSureFlag == 0) {
//            matchScore(candidate.pitches, scoreEvent, candidate.matching, candidate.path);
//            // 不确定定位的长度等于5，进行全谱匹配，isMatchingAll赋为1，当确定定位有三个时可以确定整个定位了minNMatch赋为2
//            iEvent = isSure(candidate, 3, isSureFlag, iEventPre);
//        } else {
//            vector<double> matchingTemp;
//            for (int i = 0; i < scoreEventSize; ++i) {
//                matchingTemp.push_back(matchIEvent(pitch, scoreEvent, i));
//            }
//            candidate.matching.push_back(matchingTemp);
//
//            vector<int> newNode;
//            for (vector<int>::size_type i = 0; !candidate.matching.empty() && i < candidate.matching[candidate.matching.size()-1].size(); ++i) {
//                if (candidate.matching[candidate.matching.size()-1][i] == 1) {
//                    newNode.push_back(static_cast<int>(i));
//                }
//            }
//            updatePath(candidate.path, newNode);
//            // 不确定定位的长度大于5，进行全谱匹配，isMatchingAll赋为1，当确定定位有三个时可以确定整个定位了minNMatch赋为2
//            iEvent = isSure(candidate, 3, isSureFlag, iEventPre);
//        }


        int from = barFirst;
		int	to = (iEventLastSure + nEventBack*nNotSure) < (scoreEventSize - 1) ? (iEventLastSure + nEventBack*nNotSure) : (scoreEventSize - 1);
        vector<double> matchingTemp;
        for (int i = from; i <= to; ++i) {
            matchingTemp.push_back(matchIEvent(pitch, scoreEvent, i));
        }
        candidate.matching.push_back(matchingTemp);

        vector<int> newNode;
        for (vector<int>::size_type i = 0; !candidate.matching.empty() && i < candidate.matching[candidate.matching.size()-1].size(); ++i) {
            if (candidate.matching[candidate.matching.size()-1][i] ==1) {
                newNode.push_back(static_cast<int>(i));
            }
        }
        updatePath(candidate.path, newNode);
        // 不确定定位的长度小于5，所以不用进行全谱匹配，isMatchingAll赋为0，当确定定位有两个时可以确定整个定位了minNMatch赋为2
        int iEventPreTemp = iEventPre - barFirst;
		iEvent = isSure(candidate, 2, isSureFlag, iEventPreTemp, barFirst, sfResultOriginLocate);


        // 当确定定位之后，更改前面的输出
        // 实时定位注释掉下面，实时给出输出
        if (isSureFlag == 1) {
            iEvent = findPath(candidate, iEvent[0][0], barFirst);
        }
        else if (nNotSure == maxNotSure) {
            iEvent = findPath(candidate, -1, barFirst);
            isSureFlag = 1;
        }

        if (!iEvent.empty() && iEvent[0][0] > scoreEventSize-1) {
            iEvent[0][0] = scoreEventSize-1; // 防止越界，带来影响是当检测到最后一个定位定位会一直在最后一个，向前拖不会跳到新的位置
        }
		if (iEvent.empty()){
			iEvent.push_back(vector<int>{iEventPre});
		}
		if (!iEvent.empty() && iEvent[0].size()>1){
			ProcessEvent(iEvent, candidate, sfResultOriginLocate, iEventPre, barFirst, iEventLastSure);
		}
//        if (iEvent[0][0] < 0) {
//            iEvent[0][0] = 0;
//        }
    }

    // 后处理
    if (isSureFlag == 1) {
        // 清空结构体
        candidate.iEventLastSure = 0;
        vector<vector<double> >().swap(candidate.matching);
        vector<vector<int> >().swap(candidate.path);
        vector<vector<int> >().swap(candidate.pitches);

        // 更新isPlayed数组
        if (isSureFlagPre == 1) { // 上一个位置确定定位，当前定位不是上一个定位
            if (iEvent.size() != 1 || iEvent[0].size() != 1 || iEvent[0][0] != iEventPre) {
                fill(isPlayed.begin(), isPlayed.end(), 0);
            }
        } else { // 上一个位置不确定定位，
            for (vector<int>::size_type i = 0; i < pitch.size(); ++i) {
                if (!iEvent.empty()) {
                    int iEventEnd = iEvent[0][iEvent[0].size() - 1];
                    vector<double>::iterator it;
                    it = find(scoreEvent[iEventEnd][g_Pitches].begin(), scoreEvent[iEventEnd][g_Pitches].end(), pitch[i]);
                    if (it != scoreEvent[iEventEnd][g_Pitches].end()) {
                        lia[i] = 1;
                        locb[i] = it - scoreEvent[iEventEnd][g_Pitches].begin();
                    } else {
                        lia[i] = 0;
                        locb[i] = 0;
                    }
                }
            }
            fill(isPlayed.begin(), isPlayed.end(), 0);
        }
        for (vector<int>::size_type i = 0; i < lia.size(); ++i) {
            if (lia[i] == 1) {
                isPlayed[locb[i]] = 1;
            }
        }
    }
    return iEvent;
}
