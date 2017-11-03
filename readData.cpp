//
// Created by zhangqianyi on 2017/3/16.
//

#include "readData.h"

int ReadScoreEvent(const string &filePath, vector<vector<vector<double> > >& scoreEvent) {
	ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        cout << "Error opening file scoreEvent" << endl;
        return -1; // 出错返回-1
    }
	int linenums = 0;
	while (!fileStream.eof()){
		string strLine = "";
		getline(fileStream, strLine);
		if (strLine.empty()){
			linenums++;
		}
	}
	fileStream.close();

	ifstream scoreStream(filePath);
	if (!scoreStream.is_open()){
		cout << "Error opening file scoreEvent" << endl;
		return -1; // 出错返回-1
	}
	if (!scoreEvent.empty()) {
		vector<vector<vector<double> > >().swap(scoreEvent);
	}
    // 从文件中读取数据
    // 按scoreEvent中每一列来读取文件中的数字，文件存放方式是文件的第一行存放scoreEvent的第一列，从而得到scoreEvent的列数column
    // 然后一行空格，之后column行数字，每行中数字不定，作为下一列
    string strLine = "";
	getline(scoreStream, strLine); // 读入文件第一行，将数据存为string
    // 在string中解析出来数字
    istringstream iss(strLine);
    vector<double> numLine; // 存入scoreEvent第一行数值
    for (double d; iss >> d; numLine.push_back(d)) {}
    // 将读取完的数字存入scoreEvent返回
    vector<vector<double> > firstColumn(numLine.size());
    for (vector<int>::size_type i = 0; i < numLine.size(); ++i) {
        firstColumn[i].resize(1); // 第一列数字元素只有一个，所以二维数组退化为一维
        firstColumn[i][0] = numLine[i];
    }
    scoreEvent.push_back(firstColumn);

    // 通过同样的方式读入scoreEvent的后面五列
    for (int i = 0; i < linenums-1; ++i) {
		getline(scoreStream, strLine); // 读取一行空行
        vector<vector<double> > nColumn;
        for (vector<int>::size_type j = 0; j < numLine.size(); ++j) {
            vector<double> element; // 一列中每个小元素都是一个数组
			getline(scoreStream, strLine);
            istringstream iss2(strLine);
            for (double d; iss2 >> d; element.push_back(d)) {}
            nColumn.push_back(element); // 这些小元素都放入一列这个二维数组中
        }
        scoreEvent.push_back(nColumn);
    }

	scoreStream.close();

    // 矩阵转置
    vector<vector<vector<double> > > scoreEventTranspose(scoreEvent[0].size());
    for (vector<int>::size_type i = 0; i < scoreEvent[0].size(); ++i) {
        scoreEventTranspose[i].resize(scoreEvent.size());
        for (vector<int>::size_type j = 0; j < scoreEvent.size(); ++j) {
//            scoreEventTranspose[i][j].resize(scoreEvent[j][i].size());
            scoreEventTranspose[i][j] = scoreEvent[j][i];
        }
    }

    scoreEvent = scoreEventTranspose;

    // 如果有数值为空，返回错误
    if (scoreEvent[0][6].empty()) {
        return -1;
    }
    // 正常返回值
    return 0;
}

int ReadH(const string& filePath, vector<vector<double> >& H) {
    ifstream HStream(filePath.c_str());
    if (!HStream.is_open()) {
        cout << "Error opening file H" << endl;
        return -1;
    }

    //从文件中读取数据
    vector<double> temp;
    while (HStream) {
        double iData;
        HStream >> iData;
        temp.push_back(iData);
    }
    //将数据转存为二维vector，行为temp.size/88 ，列为88
    //int rows = temp.size() / 176; //TODO : 测试修改了H的帧数，实际使用修改回来
	int rows = temp.size() / 88;
    int columns = 88;
    H.resize(rows);
    for (int i = 0; i < rows; i++) {
        H[i].resize(columns);
    }
	
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            H[i][j] = temp[i * columns + j];
        }
    }
	
	/*for (int i = 0; i < rows; i++){
		for (int j = 0; j < columns; j++){
				H[i][j] = temp[i*columns*2 + j];
		}
	}*/
    HStream.close();
    return 0;
}

int ReadScoreMidi(const string& filePath, vector<vector<double> >& scoreMidi) {
    ifstream scoreMidiStream(filePath.c_str());
    if (!scoreMidiStream.is_open()) {
        cout << "Error opening file scoreMidi" << endl;
        return -1;
    }
    //从文件中读取数据
    vector<double> temp;
    while (scoreMidiStream) {
        double iData;
        scoreMidiStream >> iData;
        temp.push_back(iData);
    }
    //将数据转存为二维vector，行为temp.size/88 ，列为88
    int rows = temp.size() / 3;
    int columns = 3;
    scoreMidi.resize(rows);
    for (int i = 0; i < rows; i++) {
        scoreMidi[i].resize(columns);
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            scoreMidi[i][j] = temp[i * columns + j];
        }
    }

    scoreMidiStream.close();
    return 0;
}

int ReadPianoRoll(const string& filePath, vector<vector<int> >& pianoRoll) {
    ifstream pianoRollStream(filePath.c_str());
    if (!pianoRollStream.is_open()) {
        cout << "Error opening file pianoRoll" << endl;
        return -1;
    }

    //从文件中读取数据
    vector<int> temp;
    while (pianoRollStream) {
        int iData;
        pianoRollStream >> iData;
        temp.push_back(iData);
    }
    //将数据转存为二维vector，行为temp.size/88 ，列为88
    int rows = temp.size() / 88;
    int columns = 88;
    pianoRoll.resize(rows);
    for (int i = 0; i < rows; i++)
        pianoRoll[i].resize(columns);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            pianoRoll[i][j] = temp[i * columns + j];
        }
    }

    pianoRollStream.close();
    return 0;
}

int SaveSfResult(const string& filePath, const vector<vector<vector<double> > >& sfResult) {
    ofstream fileStream(filePath);
    if (!fileStream.is_open()) {
        cout << "Error opening file scoreFollowingResult" << endl;
        return -1;
    }

    // 写入数据到文件中
    if (sfResult.empty()) return 0;
    // 写入sfResult的第一行时间
    fileStream << "onset时间" << endl;
    for (vector<int>::size_type i = 0; i < sfResult[0].size(); ++i) {
        for (vector<int>::size_type j = 0; j < sfResult[0][i].size(); j += 2) {
            fileStream << sfResult[0][i][j] << '\t';
        }
        fileStream << endl;
    }
    // 写入sfResult的第一行音符
    fileStream << "音符" << endl;
    for (vector<int>::size_type i = 0; i < sfResult[0].size(); ++i) {
        for (vector<int>::size_type j = 1; j < sfResult[0][i].size(); j += 2) {
            fileStream << sfResult[0][i][j] << '\t';
        }
        fileStream << endl;
    }
    // 写入sfResult的是否确定定位
    fileStream << "是否确定定位" << endl;
    for (vector<int>::size_type j = 0; j < sfResult[1].size(); ++j) {
        fileStream << sfResult[1][j][0] << endl;
    }
    // 下一列写一个nan来分隔一行写下一行数据
    fileStream << "定位" << endl;
    for (vector<int>::size_type j = 0; j < sfResult[2].size(); ++j) {
        fileStream << sfResult[2][j][0] << endl;
    }
    fileStream.close();
    return 0;
}

int Save2DVector(const string& filePath, const vector<vector<double> >& vec) {
    ofstream fileStream(filePath);
    if (!fileStream.is_open()) {
        cout << "Error opening file scoreFollowingResult" << endl;
        return -1;
    }

    // 写入数据到文件中
    if (vec.empty()) return 0;
    // 写入sfResult的第一行
    for (vector<int>::size_type i = 0; i < vec.size(); ++i) {
        for (vector<int>::size_type j = 0; j < vec[i].size(); ++j) {
            fileStream << vec[i][j] << '\t';
        }
        fileStream << endl;
    }
    fileStream.close();
    return 0;
}

int Save3DVector(const string& filePath, const vector<vector<vector<double> > >& vec) {
    ofstream fileStream(filePath);
    if (!fileStream.is_open()) {
        cout << "Error opening file scoreFollowingResult" << endl;
        return -1;
    }

    // 写入数据到文件中
    if (vec.empty()) return 0;
    // 写入sfResult的第一行
    for (vector<int>::size_type i = 0; i < vec.size(); ++i) {
        for (vector<int>::size_type j = 0; j < vec[i].size(); ++j) {
            for (vector<int>::size_type k = 0; k < vec[i][j].size(); ++k) {
                fileStream << vec[i][j][k] << '\t';
            }
            fileStream << "|\t";
        }
        fileStream << endl;
    }
    fileStream.close();
    return 0;
}