#include "Exploration2optF.h"
#include <cmath>
#include <iostream>
#include <string>
#include "struct.h"

/*******************************************************************************
 * コンストラクタ
 *
 * @param  qapInst :	問題インスタンス
 * @param  cond :		探索条件
 * @param  fileName :	出力ファイル名
*******************************************************************************/
Exploration2optF::Exploration2optF(QAP* qapInst, ExpCond* cond, std::string fileName) : ExplorationBase(qapInst, cond, fileName) {
    /* 配列を確保 */
    p_ = std::vector<int>(qapInstance_->size_);
    delta_ = std::vector<std::vector<long> >(qapInstance_->size_, std::vector<long>(qapInstance_->size_));
    remnant_ = std::vector<int>(qapInstance_->size_ * (qapInstance_->size_ - 1) / 2);
}

/*******************************************************************************
 * デストラクタ
*******************************************************************************/
Exploration2optF::~Exploration2optF() {
}

/*******************************************************************************
 * 探索の実行
*******************************************************************************/
void Exploration2optF::Explore() {
    while (status_.currTrial <= condition_->numTrial) {
        /* 初期解を生成 */
        GenerateRandSol();
        /* アルゴリズムを適用 */
        LoacalSearch();
        MsgExplore("2-optF");  // 探索開始メッセージ
        status_.currTrial++;   // 試行回数を進める
    }
}

/*******************************************************************************
 * 初期化
*******************************************************************************/
void Exploration2optF::Init() {
    /* 初期解をセット */
    for (int i = 0; i < qapInstance_->size_; ++i) {
        p_[i] = qapInstance_->pi_[i];
    }

    /* 現在のコストと差分リスト初期化 */
    status_.currCost = 0;
    for (int i = 0; i < qapInstance_->size_; ++i) {
        for (int j = 0; j < qapInstance_->size_; ++j) {
            status_.currCost += qapInstance_->flow_[i][j] * qapInstance_->distance_[p_[i]][p_[j]];
            if (i < j) {
                delta_[i][j] = INF;
            }
        }
    }
    qapInstance_->cost_ = status_.currCost;

    /* 差分リスト未登録数を初期化 */
    numRemnant_ = (qapInstance_->size_) * (qapInstance_->size_ - 1) / 2;
    for (int i = 0; i < numRemnant_; ++i) {
        remnant_[i] = i;
    }
}

/*******************************************************************************
 * 局所探索
*******************************************************************************/
void Exploration2optF::LoacalSearch() {
    int r, s;     // 交換候補用のインデックス
    int randNum;  // ランダムな数
    bool isUpdate = false;
    bool isRecord = false;

    Init();       // 解とリストの初期化
    StartMeas();  // 計測開始

    /*********************** 2-opt局所探索のメインループ ***********************/
    while (IsEndExploration() == false) {
        status_.currIter++;  // 反復回数をインクリメント
        r = NA;              // 交換候補をリセット
        s = NA;              // すべての操作がタブーだとこれに引っかかる

        if (numRemnant_ > 0) {
            /* ランダムな値を選ぶ */
            randNum = (int)(Rand() * numRemnant_);

            /* 交換候補をランダムに選ぶ */
            s = (int)(1 + sqrt(8 * (double)remnant_[randNum] + 1)) / 2;
            r = remnant_[randNum] - (s - 1) * s / 2;

            /* 計算済み差分評価値の残りを更新 */
            remnant_[randNum] = remnant_[numRemnant_ - 1];
            --numRemnant_;

            /* 差分評価値を計算 */
            delta_[r][s] = ComputeDelta(r, s);

            /* 差分評価値が改良された時 */
            if (delta_[r][s] < 0) {
                isUpdate = true;
                status_.transCnt++;
                /* 解の要素を交換 */
                Swap(p_[r], p_[s]);
                /* 現在の評価値を更新 */
                status_.currCost += delta_[r][s];

                /* 最良解が改善された時 */
                if (status_.currCost < qapInstance_->cost_) {
                    qapInstance_->cost_ = status_.currCost;
                    for (int i = 0; i < qapInstance_->size_; ++i) {
                        qapInstance_->pi_[i] = p_[i];
                    }
                }
            }
        } else {
            if (isUpdate == false) {
                if (isRecord == false) {
                    SetCompDeltaStatus();
                    isRecord = true;
                }
            } else {
                isUpdate = false;
            }
            /* 差分リスト未登録数を初期化 */
            numRemnant_ = qapInstance_->size_ * (qapInstance_->size_ - 1) / 2;
            for (int i = 0; i < numRemnant_; ++i) {
                remnant_[i] = i;
            }
            status_.currIter--;  // 反復回数を修正
        }
    }

    StopMeas();  // 計測終了
}
