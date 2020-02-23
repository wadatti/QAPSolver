/*******************************************************************************
 * Problem
 *
 * QAPのクラス
*******************************************************************************/
#ifndef PROBLEM_H
#define PROBLEM_H

#include "struct.h"

class QAP {
   private:
    std::string problemFileName_;  // 問題ファイル名
   public:
    int size_;                                  // 問題サイズ
    std::vector<std::vector<long> > flow_;      // フロー行列
    std::vector<std::vector<long> > distance_;  // 距離行列
    std::vector<int> pi_;                       // 解行列 (事実上の bestSolution)
    long cost_;                                 // 評価値 (事実上の bestCost)

   public:
    /* コンストラクタ */
    QAP(std::string &fileName);
    /* デストラクタ */
    ~QAP();
    /* 問題ファイルの読み込み */
    bool IsOpenProblemFile();

   private:
    /***************************************************************************
	 * ファイル読込メッセージ
	***************************************************************************/
    void MsgReadProblem() {
        std::cout << "Read the problem file!" << std::endl;
    }
};

#endif /* PROBLEM_H */
