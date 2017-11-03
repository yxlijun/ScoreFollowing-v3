//
// Created by zhangqianyi on 2017/3/16.
//

#ifndef READDATA_H
#define READDATA_H

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>

using namespace std;


/**
 * @author zhangqianyi
 * @date  2017/5/26
 * @param filePath              scoreEvent文件路径
 * @param scoreEvent            输入三维空数组scoreEvent，在函数中赋值
 * @return                      返回值表示执行是否成功，返回0表示正常执行，返回-1表示出错
 * @brief 从文件中读入scoreEvent
文件存放形式通过matlab转换得到
第一行有n个数字，matlab数据第一列，表示乐符的演奏时间，然后空一行
n行，matlab数据第二列，表示空一行
n行，matlab数据第三列，然后空一行
n行，matlab数据第四列，表示pitches，当前演奏音符，然后空一行
n行，matlab数据第五列，表示octave，当前演奏音符%12，然后空一行
n行，matlab数据第六列，表示节首，matlab中是空则在文件表示成为-1，结束
 */
int ReadScoreEvent(const string &filePath, vector<vector<vector<double> > >& scoreEvent);

/**
 * @author zhangqianyi
 * @date  2017/5/26
 * @param filePath              文件路径
 * @param H                     输入一个double型二维数组，行表示帧长，列表示模板长度，在函数中赋值
 * @return                      返回值表示执行是否成功，返回0表示正常执行，返回-1表示出错
 * @brief 从文件中读入多音调检测计算出的H
要求数据文件为 帧长*模板长度 (如5000*265)，每一行表示当前帧的H
与matlab相比是二维矩阵的转置，matlab中数据是按列存储，每一列表示当前帧的H
 */
int ReadH(const string& filePath, vector<vector<double> >& H);

/**
 * @author zhangqianyi
 * @date  2017/5/26
 * @param filePath              文件路径
 * @return scoreMidi            输入一个double型二维数组，在函数中赋值
 * @return                      返回值表示执行是否成功，返回0表示正常执行，返回-1表示出错
 * @brief 从文件中读取midi，将midi文件经过matlab解析之后，将数据直接存到文本文件中读取即可
 * scoreMidi第一列数据表示 pitch-20，第二列数据表示 onset时间，第三列数据表示offset时间，
 */
int ReadScoreMidi(const string& filePath, vector<vector<double> >& scoreMidi);

/**
 * @author zhangqianyi
 * @date  2017/5/26
 * @param filePath              文件路径
 * @param pianoRoll             输入一个int型二维数组，行表示帧长，列表示模板长度，在函数中赋值
 * @return                      返回值表示执行是否成功，返回0表示正常执行，返回-1表示出错
 * @brief 从文件中读入多音调检测计算出的pianoRoll（H通过阈值处理得到）
要求数据文件为 帧长*模板长度 (如5000*265)，每一行表示当前帧的pianoRoll
与matlab相比是二维矩阵的转置，matlab中数据是按列存储，每一列表示当前帧的pianoRoll
 */
int ReadPianoRoll(const string& filePath, vector<vector<int> >& pianoRoll);

/**
 * @author zhangqianyi
 * @date  2017/5/26
 * @param sfResult              通过乐谱跟踪算法计算出来的结果，第一行表示新演奏音符以及音符对应开始时间，第二行当前定位是否确定，第三行表示定位
 * @return
 * @brief 写入乐谱跟踪结果到文件中
文件存放形式为 前n行每一行存放新演奏音符以及音符对应开始时间，接着是一个"nan"，然后n行0或1表示当前定位是否确定，接着是n行定位值
 */
int SaveSfResult(const string& filePath, const vector<vector<vector<double> > >& sfResult);

/**
 * @author zhangqianyi
 * @date  2017/5/26
 * @param vec
 * @param filePath
 * @return
 * @brief 保存二维数组vector<vector<T>>到文件中
 */
int Save2DVector(const string& filePath, const vector<vector<double> >& vec);

/**
 * @author zhangqianyi
 * @date  2017/5/26
 * @param vec
 * @param filePath
 * @return
 * @brief 保存三维数组vector<vector<vector<T>>>到文件中
 */
int Save3DVector(const string& filePath, const vector<vector<vector<double> > >& vec);
#endif //READDATA_H
