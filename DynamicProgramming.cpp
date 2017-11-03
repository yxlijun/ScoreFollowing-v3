//
// Created by zhangqianyi on 2017/3/16.
//

#include "DynamicProgramming.h"

vector<vector<int> > dynamicProgramming(vector<vector<double> > matching, int pathEnd, double& maxD) {
    vector<vector<int> > iEvent;
    vector<vector<double> > degree;
    if (matching.empty()) return iEvent;
    unsigned int nCol = matching.size(); // 匹配路径长度
    if (nCol == 1) { // 如果长度为1的话就取这个位置匹配程度最高的，如果匹配程度有相同的取不同位置最多的
        degree.push_back(matching[0]);
        maxD = *max_element(degree[0].begin(), degree[0].end());
        for (int i = 0; i < degree[0].size(); ++i) { // 找出匹配程度最高的位置
            if (degree[0][i] == maxD) {
                iEvent.push_back(vector<int>{i});
            }
        }
    }
    // degree(i,j)：第i个乐谱event 与 倒数第j个演奏event 的匹配程度
    // degree是一个matching.size()列，matching[matching.size()-1].size()+2行的数组
    // matching[matching.size()-1].size()表示matching最后一个数组的长度，肯定是最长的
    // 然后考虑到了乐谱尾，倒数第二列和倒数第一列的长度一样，但是考虑倒数第二列的方向
    // 由于我们设置了三个方向，最远的方向是+2，所以行数要设置成最大加2
    int nRow = matching[matching.size()-1].size(); // 最后一个位置的匹配个数
    if (pathEnd == -1) { // 如果没有确定的尾，比如说录音结束时或者超过动态规划长度，就会执行这里
        for (int i = 0; i < nRow + 2; ++i) {
            degree.push_back(vector<double>(nCol, -1));
        }
        for (int i = 0; i < nCol; ++i) {
            for (int j = 0; j < matching[matching.size()-1 - i].size() && j < nRow; ++j) {
                degree[j][i] = matching[matching.size()-1 - i][j];
            }
        }
    } else {
        int sizeTmp = matching[matching.size()-2].size();
        nRow = (pathEnd+1) < sizeTmp ? (pathEnd+1): sizeTmp;
        for (int i = 0; i < nRow + 2; i++) {
            degree.push_back(vector<double>(nCol, -1));
        }
        degree[pathEnd][0] = 1;
        for (int i = 1; i < nCol; ++i) {
            sizeTmp = matching[matching.size()-1 - i].size(); // 每个matching[i]数组的长度
            int nCandidate = nRow < sizeTmp ? nRow : sizeTmp;
            for (int j = 0; j < nCandidate; ++j) {
                degree[j][i] = matching[matching.size()-1 - i][j];
            }
        }
    }
    // 计算各路径的匹配程度
    // degree(i,j)：从pathEnd到(i,j)的最大累积匹配程度
    // phi(i,j)：从pathEnd到degree(i,j+1)的方向
    vector<vector<vector<double> > > phi(nRow);
    for (int i = 0; i < nRow; i++) {
        phi[i].resize(nCol - 1);
    }
    for (int iCol = 0; iCol < nCol - 1; ++iCol) { // 从最后一列开始累加，所以不用考虑nCol，遍历结尾为nCol-1
        for (int iRow = 0; iRow < nRow; ++iRow) {
            // 设置路径方向为三个方向（所以degree的行数要+2）
			//vector<double> dNeighbor{ degree[iRow][iCol], degree[iRow + 1][iCol], degree[iRow + 2][iCol] };
			vector<double> dNeighbor{0.9*degree[iRow][iCol], degree[iRow+1][iCol], 0.8*degree[iRow + 2][iCol]};
            double dMax = *max_element(dNeighbor.begin(), dNeighbor.end()); // 找到各个方向的匹配程度最大值
            if (dMax != -1) { // degree默认是-1，如果整列degree中不是全部都是-1的话
                vector<double> tb; // tb记录下匹配程度最大值在三个方向中的位置，0表示同行，1表示上一行，2表示上两行
                for (unsigned int i = 0; i < dNeighbor.size(); ++i) {
                    if (dNeighbor[i] == dMax) {
                        double tbTemp = static_cast<double>(i);
                        tb.push_back(tbTemp);
                    }
                }
                if (degree[iRow][iCol + 1] != -1) { // 把当前列的最大匹配程度累加到下一列上
                    degree[iRow][iCol + 1] += dMax;
                }
                phi[iRow][iCol].resize(tb.size()); // 存下走过的方向
                for (unsigned int i = 0; i < tb.size(); i++) {
                    phi[iRow][iCol][i] = tb[i]; // 通过tb就能知道是那个方向了
                }
            } else {
                degree[iRow][iCol + 1] = -1;
            }
        }
    }
    // Traceback
    // 有了最大的degree，从这个点出发，倒着走phi的方向，保存每个节点的值就是路径了
    vector<double> nColDegree;
    for (int i = 0; i < degree.size(); i++) {
        nColDegree.push_back(degree[i][nCol-1]);
    }
    maxD = *max_element(nColDegree.begin(), nColDegree.end()); // 最大的degree
    vector<int> pathStart; // 倒着走的起点
    if (maxD != -1) { // 当maxD为-1的时候说明全是-1，此时pathStart设为空
        for (int i = 0; i < nColDegree.size(); i++) {
            if (nColDegree[i] == maxD) {
                pathStart.push_back(i);
            }
        }
    }
    vector<vector<int> > emptyIEvent(nCol);
    if (pathStart.size() == 1) { // 倒着走的起点只有一个
        iEvent = traceback(pathStart[0], phi, nCol - 1, nCol, emptyIEvent);
        // 得到的矩阵先转置再找到最大不同的
        vector<vector<int> > iEventTranspose(iEvent[0].size());
        for (int i = 0; i < iEvent[0].size(); i++) {
            iEventTranspose[i].resize(iEvent.size());
            for (int j = 0; j < iEvent.size(); j++) {
                iEventTranspose[i][j] = iEvent[j][i];
            }
        }
        iEvent = iEventTranspose;
    } else { // 倒着走的起点有多个
        vector<vector<vector<int> > > iEvent3D;
        for (int i = 0; i < pathStart.size(); i++) {
            vector<vector<int> > iEventTmp = traceback(pathStart[i], phi, nCol - 1, nCol, emptyIEvent);
            vector<vector<int> > iEventTmpTranspose(iEventTmp[0].size());
            for (int j = 0; j < iEventTmp[0].size(); j++) {
                iEventTmpTranspose[j].resize(iEventTmp.size());
                for (int k = 0; k < iEventTmp.size(); k++) {
                    iEventTmpTranspose[j][k] = iEventTmp[k][j];
                }
            }
            for (int j = 0; j < iEventTmpTranspose.size(); j++)
                iEvent.push_back(iEventTmpTranspose[j]);
        }
    }
    return iEvent;
}

vector<vector<int> > traceback(int pathStart, vector<vector<vector<double> > > phi, int iCol, int nCol, vector<vector<int> > iEvent) {
    // 第（nCol+1-iCol）个演奏event对应第pathStart个乐谱event
    // 递归调用
    if (iCol == 0) { // 结束条件，递归到最后一列
        iEvent[nCol - 1].push_back(pathStart);
        for (int i = 0; i < nCol-1; i++)  { // 其他的置-1
            iEvent[i].push_back(-1);
        }
        return iEvent;
    }
    vector<double> direction = phi[pathStart][iCol-1]; // 下一步往哪里走
    for (int i = 0; i < direction.size(); i++) {
        int thisPathStart = pathStart + static_cast<int>(direction[i]);  // 这一列的节点位置索引，就是上一列的节点位置索引加上方向值
        int nColPre = iEvent[0].size(); // 保存这一列定位的个数
        iEvent = traceback(thisPathStart, phi, iCol-1, nCol, iEvent);
        int nColNext = iEvent[0].size();
        for (int j = nColPre; j < nColNext; j++) {
            iEvent[nCol-1-iCol][j] = pathStart;
        }
    }
    return iEvent;
}
