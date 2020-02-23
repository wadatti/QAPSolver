#include "Exploration2optB.h"
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
Exploration2optB::Exploration2optB(QAP* qapInst, ExpCond* cond, std::string fileName) : ExplorationBase(qapInst, cond, fileName) {
    /* 配列を確保 */
    p_ = std::vector<int>(qapInstance_->size_);
    delta_ = std::vector<std::vector<long> >(qapInstance_->size_, std::vector<long>(qapInstance_->size_));
    remnant_ = std::vector<int>(qapInstance_->size_ * (qapInstance_->size_ - 1) / 2);
}

/*******************************************************************************
 * デストラクタ
*******************************************************************************/
Exploration2optB::~Exploration2optB() {
}

/*******************************************************************************
 * 探索の実行
*******************************************************************************/
void Exploration2optB::Explore() {
    while (status_.currTrial <= condition_->numTrial) {
        /* 初期解を生成 */
        GenerateRandSol();
        /* アルゴリズムを適用 */
        LoacalSearch();
        MsgExplore("2-optB");  // 探索開始メッセージ
        status_.currTrial++;   // 試行回数を進める
    }
}

/*******************************************************************************
 * 初期化
*******************************************************************************/
void Exploration2optB::Init() {
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

    for (int i = 0; i < qapInstance_->size_ - 1; ++i) {
        for (int j = i + 1; j < qapInstance_->size_; ++j) {
            delta_[i][j] = ComputeDelta(i, j);
        }
    }
}

/*******************************************************************************
 * 局所探索
*******************************************************************************/
void Exploration2optB::LoacalSearch() {
    long minDelta;  // 差分評価値の最小
    int r, s;       // 交換候補用のインデックス
    bool isRecord = false;

    Init();       // 解とリストの初期化
    StartMeas();  // 計測開始

    /*********************** 2-opt局所探索のメインループ ***********************/
    while (IsEndExploration() == false) {
        status_.currIter++;  // 反復回数をインクリメント
        /* 初期化 */
        r = NA;          // 交換候補をリセット
        s = NA;          // すべての操作がタブーだとこれに引っかかる
        minDelta = INF;  // 差分評価値の最良をリセット

        /* 解順列の交換する要素を選ぶ */
        for (int i = 0; i < qapInstance_->size_ - 1; ++i) {
            for (int j = i + 1; j < qapInstance_->size_; ++j) {
                if (delta_[i][j] < minDelta && delta_[i][j] < 0) {
                    r = i;
                    s = j;                    // 最良を指すインデックスを保持
                    minDelta = delta_[i][j];  // 最良の差分評価値を更新
                }
            }
        }

        /* 交換する要素がある => 交換・タブー設定・解と差分リスト更新 */
        if (r == NA) {
            /* 局所解に到達した状況を記憶 */
            if (isRecord == false) {
                SetCompDeltaStatus();
                isRecord = true;
            }
        } else {
            status_.transCnt++;
            /* 解の要素を交換 */
            Swap(p_[r], p_[s]);
            /* 現在の評価値を更新 */
            status_.currCost = status_.currCost + delta_[r][s];

            /* 最良解が改善された時 */
            if (status_.currCost < qapInstance_->cost_) {
                qapInstance_->cost_ = status_.currCost;
                for (int k = 0; k < qapInstance_->size_; ++k) {
                    qapInstance_->pi_[k] = p_[k];
                }
            }

            /* 差分リストを更新 */
            for (int i = 0; i < qapInstance_->size_ - 1; ++i) {
                for (int j = i + 1; j < qapInstance_->size_; ++j) {
                    if (i != r && i != s && j != r && j != s) {
                        delta_[i][j] = ComputeDeltaPart(i, j, r, s);
                    } else {
                        delta_[i][j] = ComputeDelta(i, j);
                    }
                }
            }
        }
    }

    StopMeas();  // 計測終了
}
