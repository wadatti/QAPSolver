#include "Examination.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Exploration2optB.h"
#include "Exploration2optF.h"
#include "ExplorationGDVPN.h"
#include "ExplorationIGDVPN.h"
#include "ExplorationIGDVPN2optB.h"
#include "ExplorationIGDVPNK.h"
#include "ExplorationNakaMod.h"
#include "ExplorationNakaura.h"
#include "ExplorationRoTS.h"
#include "Problem.h"
#include "Recorder.h"

/*******************************************************************************
 * コンストラクタ
*******************************************************************************/
Examination::Examination(int argc, char *argv[]) {
    argCnt_ = argc;

    for (int i = 0; i < argCnt_; ++i) {
        argVector_.push_back(argv[i]);
    }

    date_ = GetDateNTime();

    std::istringstream modeIStream(argVector_[1]);
    modeIStream >> examMode_;

    /* 4_Debug
	std::cout << "argCnt = " << argCnt << std::endl;
	for (int i = 0; i < argCnt; i++) {
		std::cout << argVector[i] << " ";
	}
	std::cout << std::endl;
	*/
}

/*******************************************************************************
 * デストラクタ
*******************************************************************************/
Examination::~Examination() {
}

/*******************************************************************************
 * 実行モードごとに分岐して実行
*******************************************************************************/
void Examination::Run() {
    MsgStartExperiment();
    switch (examMode_) {
        case 0:
            /* 探索の実行 */
            Examine(argCnt_, argVector_, 0);
            break;
        case 1:
            /* 条件設定と探索を繰り返す */
            Automate();
            break;
        case 2:
            /* 条件設定と探索を繰り返す (差分リスト強制生成) */
            Automate();
            break;
        default:
            std::cout << "err about examMode @ Examination.cpp" << std::endl;
            break;
    }
}

/*******************************************************************************
 * 実験を実行
 *
 * 問題読み込み => 探索 => 結果の記録
 *
 * @param  numCond :	探索条件の要素数
 * @param  cond :		探索条件の配列
 * @param  num :		実験での試行No.
*******************************************************************************/
void Examination::Examine(int numCond, std::vector<std::string> cond, int num) {
    /* 探索条件の設定 */
    ExpCond condition;
    condition = GetCondition(numCond, cond, num);

    /* 問題インスタンスの作成 */
    QAP qapInstance(problemFileName_);
    if (qapInstance.IsOpenProblemFile() == false) {
        std::cout << "failed read problem file @ Examination.cpp" << std::endl;
    } else {
        /* アルゴリズム別にインスタンス作成 => 探索を実行 */
        switch (condition.type) {
            case ROTS: {
                ExplorationRoTS rots(&qapInstance, &condition, date_);
                if (examMode_ == 2) rots.SetForceDelta();
                rots.Explore();
                break;
            }
            case TWO_OPT_F: {
                Exploration2optF twoOptF(&qapInstance, &condition, date_);
                twoOptF.Explore();
                break;
            }
            case TWO_OPT_B: {
                Exploration2optB twoOptB(&qapInstance, &condition, date_);
                twoOptB.Explore();
                break;
            }
            case NAKAURA: {
                ExplorationNakaura nakaura(&qapInstance, &condition, date_);
                if (examMode_ == 2) nakaura.SetForceDelta();
                nakaura.Explore();
                break;
            }
            case NAKAMOD: {
                ExplorationNakaMod nakaMod(&qapInstance, &condition, date_);
                if (examMode_ == 2) nakaMod.SetForceDelta();
                nakaMod.Explore();
                break;
            }
            case GDVPN: {
                ExplorationGDVPN gdvpn(&qapInstance, &condition, date_);
                if (examMode_ == 2) gdvpn.SetForceDelta();
                gdvpn.Explore();
                break;
            }
            case IGDVPN: {
                ExplorationIGDVPN igdvpn(&qapInstance, &condition, date_);
                if (examMode_ == 2) igdvpn.SetForceDelta();
                igdvpn.Explore();
                break;
            }
            case IGDVPNK: {
                ExplorationIGDVPNK igdvpnk(&qapInstance, &condition, date_);
                if (examMode_ == 2)
                    igdvpnk.SetForceDelta();
                igdvpnk.Explore();
                break;
            }
            case IGDVPN2optB: {
                ExplorationIGDVPN2optB igdvpn2optb(&qapInstance, &condition, date_);
                if (examMode_ == 2)
                    igdvpn2optb.SetForceDelta();
                igdvpn2optb.Explore();
                break;
            }
            default:
                std::cout << "err about algorithm name @ Examination.cpp" << std::endl;
                break;
        }
        /* 出力処理用インスタンス作成 */
        Recorder result(date_, problemFileName_);
        /* 動作モード別に出力処理 */
        switch (examMode_) {
            case 0:
                result.SetCondition(&condition);
                result.ShowResult();
                break;
            case 1:
            case 2:
                result.SetCondition(&condition);
                result.SetProblem(&qapInstance);
                result.RecResult();
                result.RecSolution();
                break;
            default:
                std::cout << "failed output result @ Examination.cpp" << std::endl;
                break;
        }
    }
}

/*******************************************************************************
 * 設定ファイルを元に自動的に実行
 *
 * Examin()を繰り返し実行
*******************************************************************************/
void Examination::Automate() {
    int numExam;                                    // 実験回数
    int currExam;                                   // 現在の実験
    std::vector<std::vector<std::string> > config;  // 実験の設定

    /* 設定ファイルを読み込む */
    ReadConfFile(numExam, config);

    for (currExam = 0; currExam < numExam; ++currExam) {
        MsgStartExamination(currExam + 1, numExam);

        /* 探索の実行 */
        Examine(config[currExam].size(), config[currExam], currExam + 1);
    }
}

/*******************************************************************************
 * 設定ファイルの読み込み
 *
 * @param  &numExam :	実験回数
 * @param  &conf :		実験の設定
*******************************************************************************/
void Examination::ReadConfFile(int &numExam, std::vector<std::vector<std::string> > &conf) {
    std::ifstream configFile;  // 設定ファイルのストリーム
    std::string line;          // 設定ファイルの1行分

    std::string configFilePath = CONFIG_FILE_DIR + argVector_[2] + ".txt";
    configFile.open(configFilePath.c_str());
    if (configFile.fail()) {
        std::cout << "failed open config file @ Examination.cpp" << std::endl;
    } else {
        numExam = 0;
        if (argCnt_ == 3) {
            int i = 0;
            /* 設定ファイルをEOFまで読み込み */
            while (configFile.eof() == false) {
                std::istringstream lineIStream;  // 実験1回あたりの設定ストリーム
                std::getline(configFile, line);  // 実験1回あたりにあたる
                lineIStream.str(line);           // 1行をストリームに格納

                /* 要素数をカウント */
                int lineElemCnt = 1;
                for (int j = 0; j < (int)line.length() - 1; ++j) {
                    if (line[j] == ' ' && line[j + 1] != ' ') {
                        ++lineElemCnt;
                    }
                }
                /* 要素数が足らないならcontinue */
                if (lineElemCnt < 8) {
                    continue;
                }
                /* 動的配列に要素を追加 */
                ++numExam;
                conf.resize(numExam);
                conf[i].resize(lineElemCnt);
                for (int j = 0; j < lineElemCnt; ++j) {
                    lineIStream >> conf[i][j];
                }
                ++i;
            }
        } else {
            std::cout << "err about argc @ Examination.cpp" << std::endl;
        }
    }

    MsgReadConfig();

    /* 4_Debug 
	std::cout << "numExam = " << numExam << std::endl;
	std::cout << "config size = " << conf.size() << std::endl;
	for (int i = 0; i < numExam; i++) {
		std::cout << "config[" << i << "] size = "<< conf[i].size() << std::endl;
	}
	std::cout << std::endl;
	for (int i = 0; i < numExam; i++) {
		for (unsigned int j = 0; j < conf[i].size(); j++) {
			std::cout << "conf[" << i << "][" << j << "] = " << conf[i][j] << std::endl;
		}
	}
	*/
}

/*******************************************************************************
 * 探索条件を取得
 *
 * @param  argC	:		探索条件の要素数
 * @param  argV :		探索条件の配列
 * @param  num :		実験での探索No.
 * @return condition :	探索条件
*******************************************************************************/
ExpCond Examination::GetCondition(int argC, std::vector<std::string> argV, int num) {
    ExpCond condition;

    condition.examNum = num;

    /* アルゴリズムの種類を設定 */
    if (argV[2] == "rots" || argV[2] == "RoTS") {
        condition.type = ROTS;
    } else if (argV[2] == "2optf" || argV[2] == "2optF") {
        condition.type = TWO_OPT_F;
    } else if (argV[2] == "2optb" || argV[2] == "2optB") {
        condition.type = TWO_OPT_B;
    } else if (argV[2] == "nakaura" || argV[2] == "Nakaura") {
        condition.type = NAKAURA;
    } else if (argV[2] == "nakamod" || argV[2] == "NakaMod") {
        condition.type = NAKAMOD;
    } else if (argV[2] == "gdvpn" || argV[2] == "GDVPN") {
        condition.type = GDVPN;
    } else if (argV[2] == "igdvpn" || argV[2] == "IGDVPN") {
        condition.type = IGDVPN;
    } else if (argV[2] == "igdvpnk" || argV[2] == "IGDVPNK") {
        condition.type = IGDVPNK;
    } else if (argV[2] == "igdvpn2optb" || argV[2] == "IGDVPN2optB") {
        condition.type = IGDVPN2optB;
    } else {
        std::cout << "err about algorithm name @ Examination.cpp" << std::endl;
    }

    /* 問題ファイル名を設定 */
    problemFileName_ = argV[3];

    /* 試行回数を設定 */
    std::istringstream numTrialIStream(argV[4]);
    numTrialIStream >> condition.numTrial;

    /* 終了条件の種類を設定 */
    if (argV[5] == "iter" || argV[5] == "Iter" ||
        argV[5] == "iteration" || argV[5] == "Iteration") {
        condition.endCond = ITER;
    } else if (argV[5] == "time" || argV[5] == "Time") {
        condition.endCond = TIME;
    } else {
        std::cout << "err about end condition @ Examination.cpp" << std::endl;
    }

    /* 終了条件のしきい値を設定 */
    std::istringstream endCondValIStream(argV[6]);
    endCondValIStream >> condition.endCondVal;

    /* 途中経過を記録するタイミングを設定 */
    std::istringstream trPointIStream(argV[7]);
    trPointIStream >> condition.interval;

    /* パラメータを設定 */
    if (argC >= 9) {
        for (int i = 8; i < argC; ++i) {
            double temp;
            std::istringstream paramIStream(argV[i]);
            paramIStream >> temp;
            condition.param.push_back(temp);
        }
    }

    /* 4_Debug
	std::cout << "condition.type =       " << condition.type << std::endl;
	std::cout << "condition.numTrial =   " << condition.numTrial << std::endl;
	std::cout << "condition.endCond =    " << condition.endCond << std::endl;
	std::cout << "condition.endCondVal = " << condition.endCondVal << std::endl;
	std::cout << "condition.interval =   " << condition.interval << std::endl;
	if (argC >= 9) {
		for (int i = 0; i < argC - 8; i++) {
			std::cout << "condition.param[" << i << "] =   "
						<< condition.param[i] << std::endl;
		}
	}
	std::cout << std::endl;
	*/

    return condition;
}

/*******************************************************************************
 * 実行開始時間を取得 (結果ファイル名やテンポラリファイル名に用いる)
 *
 * @return date	:	実行時の年月日時分
*******************************************************************************/
std::string Examination::GetDateNTime() {
    std::string date;
    time_t now;
    struct tm *ltm;

    std::time(&now);
    ltm = localtime(&now);

    std::stringstream fns;
    fns << ltm->tm_year + 1900;
    date = fns.str() + ".";

    fns.str("");
    fns << ltm->tm_mon + 1;
    date = date + fns.str() + ".";

    fns.str("");
    fns << ltm->tm_mday;
    date = date + fns.str() + "_";

    fns.str("");
    fns << ltm->tm_hour;
    date = date + fns.str() + ";";

    fns.str("");
    fns << ltm->tm_min;
    date = date + fns.str();

    /* ファイル名重複機能を付与？
	FILE *fp;
	if ((fp = open(fileName, "r")) == NULL) {
		// 該当ファイルなし
	}
	fclose(fp);
	*/

    return date;
}
