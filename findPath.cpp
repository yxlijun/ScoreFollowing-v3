//
// Created by zhangqianyi on 2017/3/16.
//

#include "findPath.h"

const int g_Pitches = 3;
const int g_PitchesOctave = 4;
const int g_BarFirst = 5;

// 当前演奏确定定位为pathEnd，对于此前未确定定位的演奏，计算匹配程度最高的路径
vector<vector<int> > findPath(const Candidate& candidate, int pathEnd, int barFirst) {
    vector<vector<int> > iEvent;

    // 是否有所有event都匹配的路径
    // 如果有完全匹配的路径，直接输出，不用动态规划
    vector<int> isFullMatch;
    for (vector<int>::size_type i = 0; i < candidate.path.size(); ++i) {
        isFullMatch.push_back(candidate.path[i].size() == candidate.pitches.size());
    }

    if (any_of(isFullMatch.begin(), isFullMatch.end(), [](int x) {return x != 0;})) {
        for (vector<int>::size_type i = 0; i < isFullMatch.size(); ++i) {
            if (isFullMatch[i] == 1 && candidate.path[i][candidate.path[i].size()-1] == pathEnd) {
                iEvent.push_back(candidate.path[i]);
            }
        }
    } else { // 不是完全匹配，进入dtw
        double maxD = 0;
        iEvent = dynamicProgramming(candidate.matching, pathEnd, maxD);
    }

//    if (!candidate.matching.empty() && candidate.matching[0].size() <= 5) { // 5 = nEventBack*2+1
//        int temp = candidate.iEventLastSure-2;
//        for (vector<int>::size_type i = 0; i < iEvent.size(); ++i) {
//            for (vector<int>::size_type j = 0; j < iEvent[i].size(); ++j) {
//                iEvent[i][j] = iEvent[i][j] + temp;
//            }
//        }
//    }

    // 节首的相对值，加上节首位置
    for (vector<int>::size_type i = 0; i < iEvent.size(); ++i) {
        for (vector<int>::size_type j = 0; j < iEvent[i].size(); ++j) {
            iEvent[i][j] = iEvent[i][j] + barFirst;
        }
    }

    postProcessingIEvent(iEvent, candidate, barFirst);
    return iEvent;
}

void updatePath(vector<vector<int> >& path, vector<int> newNode) {
    if (newNode.empty()) {
        vector<vector<int> >().swap(path); // 清空path
    } else {
        vector<int> notMatched(newNode.size(), 1);
        int nPath = path.size(); // 保存之前的path的长度
        for (int i = 0; i < nPath; ++i) {
            int pathEnd = path[i][path[i].size()-1];
            vector<int> memberVector{pathEnd, pathEnd+1, pathEnd+2};
            vector<int> isContinue(memberVector.size());
            vector<int> locb(memberVector.size());
            for (vector<int>::size_type j = 0; j < memberVector.size(); ++j) {
                vector<int>::iterator it;
                it = find(newNode.begin(), newNode.end(), memberVector[j]);
                if (it != newNode.end()) {
                    isContinue[j] = 1;
                    locb[j] = it - newNode.begin();
                } else {
                    isContinue[j] = 0;
                    locb[j] = 0;
                }
            }
            if (any_of(isContinue.begin(), isContinue.end(), [](int x) {return x != 0;})) {
                vector<int> iEventContinue;
                for (vector<int>::size_type j = 0; j < isContinue.size(); ++j) {
                    if (isContinue[j] != 0) {
                        iEventContinue.push_back(newNode[locb[j]]);
                        notMatched[locb[j]] = 0;
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
        // 删掉前面的节点
        path.erase(path.begin(), path.begin() + nPath);
        for (vector<int>::size_type i = 0; i < notMatched.size(); ++i) {
            if (notMatched[i] == 1) {
                path.push_back(vector<int>{newNode[i]});
            }
        }
    }
}

vector<vector<int> > isSure(const Candidate& candidate, int minNMatch, int& isSureFlag, int iEventPre, int barFirst,vector<int> &sfResultOrigin) {
    vector<vector<int> > iEvent;
    // 路径长度>=minNMatch
    vector<int> iPath;
	vector<int> pathsize;
	set<int> allpitch;
    int pathSize = candidate.path.size();
    for (int i = 0; i < pathSize; ++i) {
        if (candidate.path[i].size() >= minNMatch) {
            iPath.push_back(i);
			pathsize.push_back(candidate.path[i].size());
			for (int j = 0; j < candidate.path[i].size(); j++){
				allpitch.insert(candidate.path[i][j]);
			}
        }
    }

    vector<int> pathEnd;
    for (unsigned int i = 0; i < iPath.size(); ++i) {
        int end = candidate.path[iPath[i]].size()-1;
        int pathEndTmp = candidate.path[iPath[i]][end];
        pathEnd.push_back(pathEndTmp);
    }
    vector<int>::iterator it = unique(pathEnd.begin(), pathEnd.end());
    pathEnd.erase(it, pathEnd.end()); // 终点去重

	bool issuremark = false;
	if (!pathsize.empty()){
		bool equals = count(pathsize.begin(), pathsize.end(), pathsize[0]) == pathsize.size();
		vector<int> temp;
		for (set<int>::iterator it = allpitch.begin(); it != allpitch.end(); it++){
			temp.push_back(*it);
		}
		if (temp.size()>=4){
			sort(temp.begin(), temp.end());
			bool equals2 = temp.back() - temp[0] == temp.size() - 1;
			bool equal3 = (pathsize.size() == pathSize);
			if (equals && equals2 && equal3)
				issuremark = true;
		}
	}


    // 若满足要求的路径的终点只有一个，计算最佳匹配路径
    if (!iPath.empty() && pathEnd.size() == 1 || issuremark) {
        isSureFlag = 1;
//        // 计算路径终点
//        vector<int> pathEnd;
//        for (unsigned int i = 0; i < iPath.size(); ++i) {
//            int end = candidate.path[iPath[i]].size()-1;
//            int pathEndTmp = candidate.path[iPath[i]][end];
//            pathEnd.push_back(pathEndTmp);
//        }
//        vector<int>::iterator it = unique(pathEnd.begin(), pathEnd.end());
//        pathEnd.erase(it, pathEnd.end()); // 终点去重
//        // 若终点有多个，取距iEventPre最近、在iEventPre之后的位置
//        if (pathEnd.size() > 1)  {
//            vector<int> distance;
//            for (unsigned int i = 0; i < pathEnd.size(); ++i) {
//                distance.push_back(abs(pathEnd[i]-iEventPre));
//            }
//            // 如果最靠近的位置有多个，表示左边一个右边一个，取右边的
//            int minDistance = *min_element(distance.begin(), distance.end());
//            vector<int> distanceTemp;
//            for (int i = 0; i < distance.size(); ++i) {
//                if (distance[i] == minDistance) {
//                    distanceTemp.push_back(i);
//                }
//            }
//            if (distanceTemp.size() == 1) {
//                iEvent.push_back(vector<int>{pathEnd[distanceTemp[0]]});
//            } else if (distanceTemp.size() == 2) { // 有两个取右边的
//                iEvent.push_back(vector<int>{pathEnd[distanceTemp[1]]});
//            }
//        } else {
//            iEvent.push_back(pathEnd);
//        }
        iEvent.push_back(pathEnd);
    } else {
        // 不确定定位时，取匹配程度最高、距iEventLastSure最近、在iEventLastSure之后的位置
        isSureFlag = 0;

//        // 不确定定位时改成直接向后走，iEventPre+1
//        iEvent.push_back(vector<int>{iEventPre+1});

//        // 取匹配程度最高的位置
//        vector<int> iEventTemp;
//        double maxMatching = 0;
//        if (!candidate.matching.empty()) {
//            maxMatching = *max_element(candidate.matching[candidate.matching.size() - 1].begin(),
//                         candidate.matching[candidate.matching.size() - 1].end());
//            for (int i = 0; i < candidate.matching[candidate.matching.size() - 1].size(); ++i) {
//                if (candidate.matching[candidate.matching.size() - 1][i] == maxMatching) {
//                    iEventTemp.push_back(i);
//                }
//            }
//        }
//        if (iEventTemp.size() > 1) { // 匹配程度最大的位置有多个，取最靠近当前位置的
//            int moveForward = 0; // 如果当前位置和下一个位置的匹配程度都是最大的，moveForward为2，我们默认向后走
//            vector<int> distance(iEventTemp);
//            for (int i = 0; i < distance.size(); ++i) {
//                distance[i] = iEventTemp[i] - iEventPre;
//                if (distance[i] == 0 || distance[i] == 1) ++moveForward; // 距离为0表示当前位置，距离为1表示下一个位置
//				distance[i] = abs(distance[i]);
//            }
//
//            // 不确定定位时，取匹配程度最高、距iEventPre最近(若与iEventPre、iEventPre+1匹配程度相同，取iEventPre+1)、在iEventPre之后的位置
//            if (moveForward >= 2) { // 当前位置和下一个位置匹配程度都是最大的，向后走
//                iEvent.push_back(vector<int>{iEventPre + 1});
//            } else {
//                // 如果最靠近的位置有多个，表示左边一个右边一个，取右边的
//                int minDistance = *min_element(distance.begin(), distance.end());
//                vector<int> distanceTemp;
//                for (int i = 0; i < distance.size(); ++i) {
//                    if (distance[i] == minDistance) {
//                        distanceTemp.push_back(i);
//                    }
//                }
//                if (distanceTemp.size() == 1) { // 位置最近的地方只有一个，就取这个位置
//                    iEvent.push_back(vector<int>{iEventTemp[distanceTemp[0]]});
//                } else if (distanceTemp.size() == 2) { // 位置最近的地方有两个，说明中心对称，默认取向后走，有两个取右边的
//                    iEvent.push_back(vector<int>{iEventTemp[distanceTemp[1]]});
//                }
//            }
//        } else {
//            iEvent.push_back(iEventTemp);
//        }

        // 取当前位置和下一个位置，这两个位置匹配程度最高的位置
		if (sfResultOrigin.size() > 0){
			double lastlocation = sfResultOrigin.back();
			vector<double> pitch;
			for (int i = 0; i <sfResultOrigin.size(); i++){
				pitch.push_back(sfResultOrigin[i]);
			}
			int repeatnum = count(pitch.begin(), pitch.end(), lastlocation);
			if (repeatnum <= 1){
				if (candidate.matching.back().size() - 1 > iEventPre) {
					double currentMatching = candidate.matching.back()[iEventPre]; // 与当前位置匹配程度
					double nextMatching = candidate.matching.back()[iEventPre + 1]; // 与下一个位置匹配程度
					if (nextMatching >= currentMatching) iEvent.push_back(vector<int>{iEventPre + 1});
					else iEvent.push_back(vector<int>{iEventPre});
				}
				else {
					iEvent.push_back(vector<int>{iEventPre});
				}
			}
			else{
				int  num = candidate.matching.back().size() - iEventPre;
				double maxMatching = candidate.matching.back()[iEventPre];
				int location = iEventPre;
				if (num > repeatnum){
					for (int i = 0; i <= repeatnum; i++){
						if (iEventPre + i<candidate.matching.back().size()){
							if (i == 1){
								if (candidate.matching.back()[iEventPre + i] >= maxMatching){
									location = iEventPre + i;
								}
							}
							else{
								if (candidate.matching.back()[iEventPre + i] > maxMatching){
									location = iEventPre + i;
								}
							}
							maxMatching = candidate.matching.back()[location];
						}
					}
				}
				else{
					for (int i = 0; i <= num; i++){
						if (iEventPre + i<candidate.matching.back().size()){
							if (i == 1){
								if (candidate.matching.back()[iEventPre + i] >= maxMatching){
									location = iEventPre + i;
								}
							}
							else{
								if (candidate.matching.back()[iEventPre + i] > maxMatching){
									location = iEventPre + i;
								}
							}
							maxMatching = candidate.matching.back()[location];
						}
					}
				}
				iEvent.push_back(vector<int>{location});
			}
		}
		else{
			if (candidate.matching.back().size() - 1 > iEventPre) {
				double currentMatching = candidate.matching.back()[iEventPre]; // 与当前位置匹配程度
				double nextMatching = candidate.matching.back()[iEventPre + 1]; // 与下一个位置匹配程度
				if (nextMatching > currentMatching) iEvent.push_back(vector<int>{iEventPre + 1});
				else iEvent.push_back(vector<int>{iEventPre});
			}
			else {
				iEvent.push_back(vector<int>{iEventPre});
			}
		}
		// 由于不确定定位时是从节首开始计算匹配程度，所以iEvent是节首的相对值，最后输出定位要加上节首
		for (vector<int>::size_type i = 0; i < iEvent.size(); ++i) {
			for (vector<int>::size_type j = 0; j < iEvent[i].size(); ++j) {
				iEvent[i][j] += barFirst;
			}
		}
	}
    return iEvent;
}

void postProcessingIEvent(vector<vector<int> >& iEvent, const Candidate& candidate, int barFirst) {
	int iEventPre = candidate.iEventLastSure;
    // 若最佳匹配路径有多个
	if (iEvent.size() > 1) {
		vector<vector<int>> iEventNotBack;
		for (vector<int>::size_type i = 0; i <iEvent.size(); ++i){
			if (iEvent[i][0] >= iEventPre){
				iEventNotBack.push_back(iEvent[i]);    //选取第一个定位大于上一个确定定位的情况
			}
		}
		if (!iEventNotBack.empty()){
			// 取unique event数最多的
			vector<int> uniqueEvent(iEventNotBack.size(), 0);
			for (vector<int>::size_type i = 0; i < iEventNotBack.size(); ++i) {
				vector<int> temp(iEventNotBack[i]);
				temp.insert(temp.begin(), iEventPre);
				sort(temp.begin(), temp.end());
				vector<int>::iterator it = unique(temp.begin(), temp.end());
				temp.resize(distance(temp.begin(), it));
				uniqueEvent[i] = temp.size();
			}
			vector<int>::iterator it1 = max_element(uniqueEvent.begin(), uniqueEvent.end());
			int iIEvent = distance(uniqueEvent.begin(), it1);
			vector<vector<int>> MaxiEvent;
			for (vector<int>::size_type i = 0; i < uniqueEvent.size(); i++){
				if (uniqueEvent[i] == uniqueEvent[iIEvent]){
					if (iEventNotBack[i][0] >= iEventPre)
						MaxiEvent.push_back(iEventNotBack[i]);
				}
			}
			if (MaxiEvent.size() > 1){     // 如果event数最多的不值一条，则选取与理想路径差距最小的一条
				vector<int> IdealEvent(MaxiEvent[0].size());
				for (vector<int>::size_type i = 0; i<MaxiEvent[0].size(); i++){
					IdealEvent[i] = iEventPre + i + 1;
				}
				vector<int> allsum(MaxiEvent.size());
				for (vector<int>::size_type i = 0; i < MaxiEvent.size(); i++){
					int sum = 0;
					for (vector<int>::size_type j = 0; j < MaxiEvent[0].size(); j++){
						sum += (abs(MaxiEvent[i][j] - IdealEvent[j]));
					}
					allsum[i] = sum;
				}
				vector<int>::iterator it = min_element(allsum.begin(), allsum.end());
				int loc = distance(allsum.begin(), it);
				vector<int> iEventTmp(MaxiEvent[loc]);
				vector<vector<int> >().swap(iEvent);
				iEvent.push_back(iEventTmp);
			}
			else{
				vector<vector<int> >().swap(iEvent);   // 如果只有一条，则直接将次路径给iEvent
				iEvent = MaxiEvent;
			}
		}
		else {                                        // 如果所有路径的第一个值都小于上一个确定定位，选取一条离上一次确定定位最近的路径
			vector<int> uniqueEvent(iEvent.size(), 0);
			for (vector<int>::size_type i = 0; i < iEvent.size(); ++i) {
				vector<int> temp(iEvent[i]);
				temp.insert(temp.begin(), iEventPre);
				sort(temp.begin(), temp.end());
				vector<int>::iterator it = unique(temp.begin(), temp.end());
				temp.resize(distance(temp.begin(), it));
				uniqueEvent[i] = temp.size();
			}
			vector<int>::iterator it1 = max_element(uniqueEvent.begin(), uniqueEvent.end());
			int iIEvent = distance(uniqueEvent.begin(), it1);
			int firstLoc = iEvent[iIEvent][0];
			int maxloc = 0;
			for (vector<int>::size_type i = 0; i < uniqueEvent.size(); i++){
				if (uniqueEvent[i] == uniqueEvent[iIEvent]){
					if (iEvent[i][0] > firstLoc){
						firstLoc = iEvent[i][0];
						maxloc = i;
					}
				}
			}
			if (iEvent[maxloc][0] == iEventPre-1 &&
			        (!candidate.matching.empty() && candidate.matching[0].size() > iEventPre-barFirst) &&
			        (candidate.matching[0][iEvent[iIEvent][0]-barFirst] == candidate.matching[0][iEventPre-barFirst])) { // 出现回弹
			    iEvent[iIEvent][0] = iEventPre;
			}
			vector<int> iEventTmp(iEvent[iIEvent]);
			vector<vector<int> >().swap(iEvent); // 清空iEvent
			iEvent.push_back(iEventTmp);
		}
		
       //if (iEvent.size() > 1) {
       // // 取unique event数最多的
       // vector<int> uniqueEvent(iEvent.size(), 0);
       // for (vector<int>::size_type i = 0; i < iEvent.size(); ++i) {
       //     vector<int> temp(iEvent[i]);
       //     temp.insert(temp.begin(), iEventPre);
       //     sort(temp.begin(), temp.end());
       //     vector<int>::iterator it = unique(temp.begin(), temp.end());
       //     temp.resize(distance(temp.begin(), it));
       //     uniqueEvent[i] = temp.size();
       // }
       // vector<int>::iterator it1 = max_element(uniqueEvent.begin(), uniqueEvent.end());
       // int iIEvent = distance(uniqueEvent.begin(), it1);

       // // 上面操作将多条路径选择了一条位置最多的路径，比如有路径0 2 2 2，1 2 2 2,2 2 2 2，那么我们会选择0,2,2,2
       // // 但是从位置1开始不确定定位，所以最后输出结果变成了1 0 2 2 2，出现了回弹，这样不对
       // // 其实在路径中有一条1 2 2 2，也是正确的路径，只是我们选取了索引为0的位置的定位，没选到他，现在为了去掉回弹情况要选择这条路径

       // // 如果定位结果的第一个iEvent[iIEvent][0]比上一次定位小，iEvent[iIEvent][0] == iEventPre-1，说明这种情况回弹了
       // // 同时如果0,2,2,2和1,2,2,2这两条路径的开头0和2的匹配程度相同，就可以选1
       // if (iEvent[iIEvent][0] == iEventPre-1 &&
       //         (!candidate.matching.empty() && candidate.matching[0].size() > iEventPre-barFirst) &&
       //         (candidate.matching[0][iEvent[iIEvent][0]-barFirst] == candidate.matching[0][iEventPre-barFirst])) { // 出现回弹
       //     iEvent[iIEvent][0] = iEventPre;
       // }

       // vector<int> iEventTmp(iEvent[iIEvent]);
       // vector<vector<int> >().swap(iEvent); // 清空iEvent
       // iEvent.push_back(iEventTmp);
	}
}

vector<int> matlabUnique(vector<vector<int> >& arr) {
    // 先排序，再用unique函数，最后用distance和resize
    vector<vector<int> > sortArr(arr);
    sort(sortArr.begin(), sortArr.end());
    vector<vector<int> >::iterator it = unique(sortArr.begin(), sortArr.end());
    sortArr.resize(distance(sortArr.begin(), it));

    vector<int> indexArr; // vector中元素第一次出现的index
    for (vector<int>::size_type i = 0; i < sortArr.size(); ++i) {
        it = find(arr.begin(), arr.end(), sortArr[i]);
        indexArr.push_back(distance(arr.begin(), it));
    }
    sort(indexArr.begin(), indexArr.end());

    vector<vector<int> > resultArr;
    for (vector<int>::size_type i = 0; i < sortArr.size(); i++) {
        resultArr.push_back(arr[indexArr[i]]);
    }

    arr = resultArr;
    return indexArr;
}

void ProcessEvent(vector<vector<int>> &IEvent, Candidate& candidate, vector<int> &sfResultOrigin, int IEventPre, int barFirst, int iEventLastSure){
	int iEventPreTemp = IEventPre - barFirst;
	if (IEvent[0][0] < iEventLastSure){
		if (candidate.matching.back()[iEventPreTemp] == 1 || (iEventPreTemp + 1<candidate.matching.back().size() && candidate.matching.back()[iEventPreTemp + 1] == 1)){
			for (int i = 0; i<IEvent[0].size(); i++){
				if (i == IEvent[0].size() - 1){
					if (candidate.matching.back().size() - 1 > iEventPreTemp){
						if (candidate.matching.back()[iEventPreTemp] < candidate.matching.back()[iEventPreTemp + 1])
							IEvent[0].back() = IEventPre + 1;
						else
							IEvent[0].back() = IEventPre;
					}
					else
						IEvent[0].back() = IEventPre;
				}
				else{
					int length = IEvent[0].size() - 1;
					int sfLength = sfResultOrigin.size();
					if (sfLength - length + i >= 0 && sfLength - length + i<sfLength){
						IEvent[0][i] = static_cast<int>(sfResultOrigin[sfLength - length + i] - 1);
					}
				}
			}
		}
	}
}