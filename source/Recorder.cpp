#include "Recorder.h"
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "struct.h"

/*******************************************************************************
 * コンストラクタ
 *
 * @param date :		実験の実行日時
 * @param fileName :	出力ファイル名
*******************************************************************************/
Recorder::Recorder(std::string date, std::string fileName) {
    date_ = date;
    problemFileName_ = fileName;
}

/*******************************************************************************
 * デストラクタ
*******************************************************************************/
Recorder::~Recorder() {
}

/*******************************************************************************
 * 探索条件を設定
*******************************************************************************/
void Recorder::SetCondition(ExpCond *condition) {
    condition_ = condition;
}

/*******************************************************************************
 * 問題インスタンスを設定
 *
 * @param *qapInstance :	問題インスタンス
*******************************************************************************/
void Recorder::SetProblem(QAP *qapInstance) {
    qapInstance_ = qapInstance;
}

/*******************************************************************************
 * 結果を出力 (ダミー)
*******************************************************************************/
void Recorder::ShowResult() {
}

/*******************************************************************************
 * 実験結果をファイルに出力
*******************************************************************************/
void Recorder::RecResult() {
    int numTrial = condition_->numTrial;
    int numInterval = condition_->endCondVal / condition_->interval + 1;

    std::vector<std::vector<long> > cost(numTrial, std::vector<long>(numInterval));
    std::vector<std::vector<long> > iter(numTrial, std::vector<long>(numInterval));
    std::vector<std::vector<long> > trans(numTrial, std::vector<long>(numInterval));
    std::vector<std::vector<long> > callCD(numTrial, std::vector<long>(numInterval));
    std::vector<std::vector<long> > callCDP(numTrial, std::vector<long>(numInterval));
    std::vector<std::vector<long> > param(numTrial, std::vector<long>(numInterval));
    std::vector<std::vector<double> > time(numTrial, std::vector<double>(numInterval));
    std::vector<std::vector<double> > compDelta(numTrial, std::vector<double>(7));
    std::vector<std::vector<double> > aveResult(7, std::vector<double>(numInterval, 0));
    std::vector<double> aveCompDelta(7, 0);

    std::vector<long> sumCost(numInterval);

    /* テンポラリファイルの読み込み */
    IsOpenTempFiles(cost, iter, trans, callCD, callCDP, param, time, compDelta);

    /* 平均を求める */
    ComputeAve(aveResult[0], cost);
    ComputeAve(aveResult[1], iter);
    ComputeAve(aveResult[2], trans);
    ComputeAve(aveResult[3], callCD);
    ComputeAve(aveResult[4], callCDP);
    ComputeAve(aveResult[5], param);
    ComputeAve(aveResult[6], time);
    ComputeAve(aveCompDelta, compDelta);

    /* 問題情報を記録 */
    RecExamInfo();

    /* 差分リスト完成時の状況を書き込み */
    resultFileOStream_ << LBL_DELTA << ','
                       << LBL_COST << ','
                       << LBL_ITER << ','
                       << LBL_TRANS << ','
                       << LBL_CD << ','
                       << LBL_CDP << ','
                       << LBL_PARAM << ','
                       << LBL_TIME << ",\n";
    resultFileOStream_ << ",";
    for (int i = 0; i < 7; ++i) {
        if (aveCompDelta[i] == EXC) {
            resultFileOStream_ << "N/A,";
        } else if (aveCompDelta[i] == NA) {
            resultFileOStream_ << "N/A,";
        } else if (aveCompDelta[i] == INF) {
            resultFileOStream_ << "INF,";
        } else {
            resultFileOStream_ << aveCompDelta[i] << ",";
        }
    }
    resultFileOStream_ << "\n\n";

    /* 平均値を書き込み */
    resultFileOStream_ << LBL_AVERAGE << ','
                       << LBL_COST << ','
                       << LBL_ITER << ','
                       << LBL_TRANS << ','
                       << LBL_CD << ','
                       << LBL_CDP << ','
                       << LBL_PARAM << ','
                       << LBL_TIME << ",\n";
    RecMatrix(aveResult);
    resultFileOStream_ << "\n";

    /* 評価値を書き込み */
    RecMatrixLbl(LBL_COST, numTrial);
    RecMatrix(cost);
    resultFileOStream_ << "\n";

    /* 反復回数を書き込み */
    RecMatrixLbl(LBL_ITER, numTrial);
    RecMatrix(iter);
    resultFileOStream_ << "\n";

    /* 最良解更新回数を書き込み */
    RecMatrixLbl(LBL_TRANS, numTrial);
    RecMatrix(trans);
    resultFileOStream_ << "\n";

    /* ComputeDelta呼出回数を書き込み */
    RecMatrixLbl(LBL_CD, numTrial);
    RecMatrix(callCD);
    resultFileOStream_ << "\n";

    /* ComputeDeltaPart呼出回数を書き込み */
    RecMatrixLbl(LBL_CDP, numTrial);
    RecMatrix(callCDP);
    resultFileOStream_ << "\n";

    /* 可変パラメータの値を書き込み */
    RecMatrixLbl(LBL_PARAM, numTrial);
    RecMatrix(param);
    resultFileOStream_ << "\n";

    /* 計算時間を書き込み */
    RecMatrixLbl(LBL_TIME, numTrial);
    RecMatrix(time);

    /* 出力ファイルの設定 */
    std::string resultFilePath = RESULT_FILE_DIR;
    std::stringstream fns;
    fns << condition_->examNum;
    resultFilePath += "Result-Examination" + fns.str() + "_@" + date_ + ".csv";
    /* 出力ファイルに書き込み */
    std::ofstream resultFile(resultFilePath.c_str());  // 出力ストリーム
    resultFile << resultFileOStream_.str();

    MsgRecordResult();
}

/*******************************************************************************
 * 解をファイルに出力
*******************************************************************************/
void Recorder::RecSolution() {
    std::vector<long> cost(condition_->numTrial);
    std::vector<int> solution(qapInstance_->size_);
    std::string tempFilePath;  // テンポラリファイルの相対パス
    std::stringstream fns;

    /* 評価値を読み込む */
    for (int i = 1; i <= condition_->numTrial; ++i) {
        fns.str("");
        fns << condition_->examNum;
        tempFilePath = TEMP_FILE_DIR + (std::string) "sol" + date_ + "_e" + fns.str();
        fns.str("");
        fns << i;
        tempFilePath += "t" + fns.str() + ".dat";

        std::ifstream tempFile(tempFilePath.c_str());
        if (tempFile.fail()) {
            std::cout << "failed read cost from sol file @ Recoder.cpp" << std::endl;
        } else {
            tempFile >> cost[i - 1];
        }
    }
    /* 最良解を決める */
    long bestCost = INF;
    int bestTrial = 0;
    for (int i = 0; i < condition_->numTrial; ++i) {
        if (cost[i] < bestCost) {
            bestCost = cost[i];
            bestTrial = i + 1;
        }
    }
    /* 最良解の読み込み */
    fns.str("");
    fns << condition_->examNum;
    tempFilePath = TEMP_FILE_DIR + (std::string) "sol" + date_ + "_e" + fns.str();
    fns.str("");
    fns << bestTrial;
    tempFilePath += "t" + fns.str() + ".dat";

    std::ifstream tempFile(tempFilePath.c_str());
    if (tempFile.fail()) {
        std::cout << "failed read best solution form sol file @ Recoder.cpp" << std::endl;
    } else {
        tempFile >> bestCost;
        for (int i = 0; i < qapInstance_->size_; ++i) {
            tempFile >> solution[i];
        }
    }

    /* 書き込み処理 */
    std::string resultFilePath = SOLUTION_FILE_DIR;
    fns.str("");
    fns << condition_->examNum;
    resultFilePath += "Solution-Examination" + fns.str() + "_@" + date_ + ".csv";

    std::ofstream resultFile(resultFilePath.c_str());

    resultFile << LBL_PROBLEM << ',' << problemFileName_ << ",\n";
    resultFile << LBL_SIZE << ',' << qapInstance_->size_ << ",\n";
    resultFile << LBL_COST << ',' << bestCost << ",\n";

    resultFile << "Solution,\n";
    int cnt = 0;
    for (int i = 0; i < qapInstance_->size_; ++i) {
        ++cnt;
        resultFile << solution[i] << ",";
        if (cnt >= 20) {
            resultFile << "\n";
            cnt = 0;
        }
    }
    resultFile << "\n";

    MsgRecordSolution();
}

/*******************************************************************************
 * テンポラリファイルを読み込む
 *
 * @param &cost :		評価値
 * @param &iter :		反復回数
 * @param &trans :		交換回数
 * @param &callCD :		ComputeDelta()の呼び出し回数
 * @param &callCDP :	ComputeDeltaPart()の呼び出し回数
 * @param &param :		可変パラメータ (アルゴリズムによって自由に)
 * @param &time :		実行時間
 * @param &compDelta :	差分リスト完成時の状況
*******************************************************************************/
bool Recorder::IsOpenTempFiles(std::vector<std::vector<long> > &cost,
                               std::vector<std::vector<long> > &iter,
                               std::vector<std::vector<long> > &trans,
                               std::vector<std::vector<long> > &callCD,
                               std::vector<std::vector<long> > &callCDP,
                               std::vector<std::vector<long> > &param,
                               std::vector<std::vector<double> > &time,
                               std::vector<std::vector<double> > &compDelta) {
    std::string tempFilePath;  // テンポラリファイルの相対パス
    std::stringstream fns;
    int numInterval = condition_->endCondVal / condition_->interval + 1;

    /* テンポラリファイルの読み込み */
    for (int i = 1; i <= condition_->numTrial; ++i) {
        fns.str("");
        fns << condition_->examNum;
        tempFilePath = TEMP_FILE_DIR + (std::string) "rsl" + date_ + "_e" + fns.str();
        fns.str("");
        fns << i;
        tempFilePath += "t" + fns.str() + ".dat";

        std::ifstream tempFile(tempFilePath.c_str());
        if (tempFile.fail()) {
            std::cout << "failed read rsl file @ Recoder.cpp" << std::endl;
            return false;
        } else {
            for (int j = 0; j < numInterval; ++j) {
                tempFile >> cost[i - 1][j];
                tempFile >> iter[i - 1][j];
                tempFile >> trans[i - 1][j];
                tempFile >> callCD[i - 1][j];
                tempFile >> callCDP[i - 1][j];
                tempFile >> param[i - 1][j];
                tempFile >> time[i - 1][j];
            }
            for (int j = 0; j < 7; ++j) {
                tempFile >> compDelta[i - 1][j];
            }
        }
    }

    return true;
}

/*******************************************************************************
 * 個別の試行結果の設定を記録
*******************************************************************************/
void Recorder::RecExamInfo() {
    /* 問題名とアルゴリズム名を書き込み */
    resultFileOStream_ << LBL_PROBLEM << ',' << problemFileName_ << ",\n\n";

    resultFileOStream_ << LBL_ALGORITHM << ',';
    if (condition_->type == ROTS) {
        resultFileOStream_ << "RoTS,\n";
    } else if (condition_->type == TWO_OPT_F) {
        resultFileOStream_ << "2-opt_F,\n";
    } else if (condition_->type == TWO_OPT_B) {
        resultFileOStream_ << "2-opt_B,\n";
    } else if (condition_->type == NAKAURA) {
        resultFileOStream_ << "Nakaura,\n";
    } else if (condition_->type == NAKAMOD) {
        resultFileOStream_ << "NakaMod,\n";
    } else if (condition_->type == GDVPN) {
        resultFileOStream_ << "GDVPN,\n";
    } else if (condition_->type == IGDVPN) {
        resultFileOStream_ << "IGDVPN,\n";
    } else if (condition_->type == IGDVPNK) {
        resultFileOStream_ << "IGDVPNK,\n";
    } else if (condition_->type == IGDVPN2optB) {
        resultFileOStream_ << "IGDVPN2optB,\n";
    } else {
        resultFileOStream_ << "N/A,\n";
    }
    resultFileOStream_ << "\n";

    /* パラメータを書き込み */
    resultFileOStream_ << LBL_PARAM << ',';
    if (condition_->param.size() != 0) {
        for (unsigned int i = 0; i < condition_->param.size(); ++i) {
            resultFileOStream_ << condition_->param[i] << ",";
        }
        resultFileOStream_ << "\n\n";
    } else {
        resultFileOStream_ << "N/A,\n\n";
    }
}

/*******************************************************************************
 * 個別の試行結果のラベルを記録
 *
 * @param label :	ラベル名
 * @param n :		記録回数
*******************************************************************************/
void Recorder::RecMatrixLbl(std::string label, int n) {
    resultFileOStream_ << label << ",";
    for (int i = 1; i <= n; i++) {
        if (i % 10 == 1) {
            resultFileOStream_ << i << "st,";
        } else if (i % 10 == 2) {
            resultFileOStream_ << i << "nd,";
        } else if (i % 10 == 3) {
            resultFileOStream_ << i << "rd,";
        } else {
            resultFileOStream_ << i << "th,";
        }
    }
    resultFileOStream_ << "\n";
}
