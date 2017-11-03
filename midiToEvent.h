//
// Created by zhangqianyi on 2017/5/2.
//

#ifndef MIDITOEVENT_H
#define MIDITOEVENT_H

/**
 *
 * 文件中存储midi变量，第1-3列分别表示：音符序号 - onset(s) - offset(s)
 * 生成的scoreEvent文件中存储乐谱信息，5列：
 * (1)onset(s)（用于计算演奏节奏）
 * (2)此刻演奏的音符在MIDI数据中的行号（用于音符结束后判断是否被演奏）
 * (3)此刻新演奏的音符在MIDI数据中的行号（用于乐谱跟踪中标记上一定位对应的音符是否被演奏）
 * (4)此刻新演奏的音符的音符序号
 * (5)忽略倍频时，此刻新演奏的音符的音符序号（[0,11]）
 */

#endif //MIDITOEVENT_H
