#include "Problem.h"
#include <fstream>
#include <iostream>
#include <string>

/*******************************************************************************
 * コンストラクタ
*******************************************************************************/
QAP::QAP(std::string &fileName) {
    problemFileName_ = fileName;
}

/*******************************************************************************
 * デストラクタ
*******************************************************************************/
QAP::~QAP() {
}

/*******************************************************************************
 * 問題ファイルの読み込み
 *
 * @return boolean :	ファイルを読み込めたかどうか
*******************************************************************************/
bool QAP::IsOpenProblemFile() {
    std::string problemFilePath;  // 問題ファイルの相対パス
    problemFilePath = RROBLEM_FILE_DIR + problemFileName_ + ".dat";
    std::ifstream dataFile(problemFilePath.c_str());

    if (dataFile.fail()) {
        std::cout << "failed read data file @ Problem.cpp" << std::endl;
    } else {
        /* 問題サイズを読み込み */
        dataFile >> size_;

        /* フロー行列と距離行列を確保 */
        flow_ = std::vector<std::vector<long> >(size_, std::vector<long>(size_));
        distance_ = std::vector<std::vector<long> >(size_, std::vector<long>(size_));

        /* フロー行列と距離行列を読み込み */
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                dataFile >> flow_[i][j];
            }
        }
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                dataFile >> distance_[i][j];
            }
        }

        /* 解行列を確保 */
        pi_ = std::vector<int>(size_);

        MsgReadProblem();

        return true;
    }

    return false;
}
