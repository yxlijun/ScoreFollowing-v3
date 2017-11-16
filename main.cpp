#include <iostream>
#include "ScoreFollowing.h"
#include "EvaluateSfResult.h"
#include <io.h>

using namespace std;

vector<string> scorefiles;
vector<string> Hfiles;
vector<string> paths;
void GetAllFileFromPath(string folderPath)
{
	_finddata_t FileInfo;
	string strfind = folderPath + "/*";
	long Handle = _findfirst(strfind.c_str(), &FileInfo);

	if (Handle == -1L)
	{
		_findclose(Handle);
		return;
	}
	do{
		if (FileInfo.attrib & _A_SUBDIR)
		{
			if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
			{
				string newPath = folderPath + "/" + FileInfo.name;
				GetAllFileFromPath(newPath);
			}
		}
		else{
			string newPath = folderPath + "/" + FileInfo.name;
			string path = FileInfo.name;
			int index = path.find(".dat", 0);
			int index2 = path.find("xH88", 0);
			if (index != -1){
				scorefiles.push_back(newPath);
				paths.push_back(folderPath);
			}
			if (index2 != -1){
				Hfiles.push_back(newPath);
			}
		}
	} while (_findnext(Handle, &FileInfo) == 0);

	_findclose(Handle);
}

void GetFilePath(string folderPath, vector<string> &scorepath, vector<string> &scoreDat)
{
	_finddata_t FileInfo;
	string strfind = folderPath + "/*";
	long Handle = _findfirst(strfind.c_str(), &FileInfo);

	if (Handle == -1L)
	{
		_findclose(Handle);
		return;
	}
	do{
		if (FileInfo.attrib & _A_SUBDIR)
		{
			if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
			{
				string newPath = folderPath + "/" + FileInfo.name;
				GetFilePath(newPath, scorepath, scoreDat);
			}
		}
		else{
			string newPath = folderPath + "/" + FileInfo.name;
			string path = FileInfo.name;
			int index = path.find(".txt", 0);
			int index1 = path.find(".dat", 0);
			int index2 = path.find(".zip", 0);
			if (index == -1 && index1 == -1 && index2 == -1){
				scorepath.push_back(newPath);
				string DatPath = newPath + ".dat";
				scoreDat.push_back(DatPath);
			}

		}
	} while (_findnext(Handle, &FileInfo) == 0);
	_findclose(Handle);
}

void Rename(string path){            //文件重命名
	vector<string> scorepath, scoreDat;
	GetFilePath(path, scorepath, scoreDat);
	for (vector<int>::size_type i = 0; i < scoreDat.size(); i++){
		if (!_access(scorepath[i].c_str(), 0)){
			if (!rename(scorepath[i].c_str(), scoreDat[i].c_str())){
				cout << scorepath[i] << " rename: " << scoreDat[i] << endl;
			}
		}
	}
}


int main()
{
	//string dirroot = "../Data/Pad/opern";
	//GetAllFileFromPath(dirroot);
	//string savefile[4] = {"sfResult.txt","sfResultOrigin.txt","evaluateResult.txt","evaluateResultOrigin.txt"};

	//for (int i = 0; i < scorefiles.size(); i++){
	//	const string scoreEventPath =scorefiles[i];
	//	const string HPath = Hfiles[i];
	//	vector<string> ResultFile(4);
	//	for (size_t j = 0; j < 4; j++){
	//		ResultFile[j] = paths[i] +"/"+ savefile[j];
	//	}
	//	ScoreFollowing scoreFollowing;
	//	cout << scoreEventPath << endl;
	//	cout << HPath << endl;

	//	if (scoreFollowing.Init(scoreEventPath) == -1) {
	//		cout << "Read scoreEvent error!" << endl;
	//		exit(-1);
	//	}
	//	scoreFollowing.processNMF.SetThreshParams(0.2, 240);
	//	//  scoreFollowing.processNMF.SetSplitFrameCountParams(0.225, 0.2, 0.13);
	//	//double timeResolution = static_cast<double>(512*2) / 44100;
	//	double timeResolution = static_cast<double>(512) / 44100;

	//	vector<int> notesInScore = scoreFollowing.GetNoteInScore();
	//	int maxPitchesInEvent = scoreFollowing.GetMaxPitchesInFrame();
	//	vector<vector<double> > H;
	//	if (ReadH(HPath, H) == -1) {
	//		cout << "Read H error!" << endl;
	//		exit(-1);
	//	}
	//	vector<double> error;
	//	scoreFollowing.ScoreFollowingOffline(H, error,timeResolution, maxPitchesInEvent);

	//	SaveSfResult(ResultFile[0], scoreFollowing.GetSfResult());
	//	SaveSfResult(ResultFile[1], scoreFollowing.GetSfResultOrigin());

	//	// 乐谱结果评价

	//	EvaluateSfResult evaluateResult(scoreFollowing);
	//	evaluateResult.Init();
	//	vector<Correctness> correctness = evaluateResult.EvaluateCorrectness();
	//	vector<Correctness> correctness1 = evaluateResult.EvaluateCorrectnessModify();

	//	vector<BeatRhythm> beatRhythm = evaluateResult.EvaluateBeatRhythm();
	//	//vector<int> noteRhythm = evaluateResult.EvaluateNoteRhythm();
	//	double score = evaluateResult.CountScore(correctness1, beatRhythm);
	//	int starsNum = evaluateResult.GiveStars(score);
	//	cout << "score = " << score << " stars = " << starsNum << endl;

	//	evaluateResult.SaveEvaluateResult(ResultFile[2], correctness1, beatRhythm);

	//	vector<Correctness> correctnessOrigin = evaluateResult.EvaluateCorrectnessOrigin(maxPitchesInEvent);
	//	evaluateResult.SaveEvaluateResult(ResultFile[3], correctnessOrigin, beatRhythm);
	//}
 //   


	const string scoreEventPath = "../Data/Pad/11.1/HuaShengDun/HuaShengDun.dat";
	ScoreFollowing scoreFollowing;
	if (scoreFollowing.Init(scoreEventPath) == -1) {
		cout << "Read scoreEvent error!" << endl;
		exit(-1);
	}
	scoreFollowing.processNMF.SetThreshParams(0.2, 240);
	//  scoreFollowing.processNMF.SetSplitFrameCountParams(0.225, 0.2, 0.13);
	//double timeResolution = static_cast<double>(512*2) / 44100;
	double timeResolution = static_cast<double>(512) / 44100;

	vector<int> notesInScore = scoreFollowing.GetNoteInScore();
	int maxPitchesInEvent = scoreFollowing.GetMaxPitchesInFrame();
	vector<vector<double> > H;
	if (ReadH("../Data/Pad/11.1/HuaShengDun/xH88.txt", H) == -1) {
		cout << "Read H error!" << endl;
		exit(-1);
	}
	vector<double> error;
	scoreFollowing.ScoreFollowingOffline(H,error, timeResolution, maxPitchesInEvent);

	SaveSfResult("../Data/Pad/11.1/HuaShengDun/sfResult.txt", scoreFollowing.GetSfResult());
	SaveSfResult("../Data/Pad/11.1/HuaShengDun/sfResultOrigin.txt", scoreFollowing.GetSfResultOrigin());




	//    // 第二遍调用
	//    scoreFollowing.Reset();
	//    vector<vector<double> > H2;
	//    if (ReadH("../Data/Pad/test/Santa_Lucia2_xH88.txt", H2) == -1) {
	//        cout << "Read H error!" << endl;
	//        exit(-1);
	//    }
	//    scoreFollowing.ScoreFollowingOffline(H2, timeResolution);
	//
	//    SaveSfResult("../Data/Pad/result/Santa_Lucia2_sfResult.txt", scoreFollowing.GetSfResult());
	//

	// 乐谱结果评价

	EvaluateSfResult evaluateResult(scoreFollowing);
	evaluateResult.Init();
	vector<Correctness> correctness = evaluateResult.EvaluateCorrectness();
	vector<Correctness> correctness1 = evaluateResult.EvaluateCorrectnessModify();

	vector<BeatRhythm> beatRhythm = evaluateResult.EvaluateBeatRhythm();
	//vector<int> noteRhythm = evaluateResult.EvaluateNoteRhythm();
	double score = evaluateResult.CountScore(correctness1, beatRhythm);
	int starsNum = evaluateResult.GiveStars(score);
	cout << "score = " << score << " stars = " << starsNum << endl;
	evaluateResult.SaveEvaluateResult("../Data/Pad/11.1/HuaShengDun/evaluateResult.txt", correctness1, beatRhythm);

	vector<Correctness> correctnessOrigin = evaluateResult.EvaluateCorrectnessOrigin(maxPitchesInEvent);
	evaluateResult.SaveEvaluateResult("../Data/Pad/11.1/HuaShengDun/evaluateResultOrigin.txt", correctnessOrigin, beatRhythm);
    cout << "test" << endl;
    return 0;
}




