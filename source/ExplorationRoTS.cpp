#include "ExplorationRoTS.h"
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
ExplorationRoTS::ExplorationRoTS(QAP* qapInst, ExpCond* cond, std::string fileName) : ExplorationBase(qapInst, cond, fileName) {
    /* 配列を確保 */
    p_ = std::vector<int>(qapInstance_->size_);
    delta_ = std::vector<std::vector<long> >(qapInstance_->size_, std::vector<long>(qapInstance_->size_));
    tabuList_ = std::vector<std::vector<long> >(qapInstance_->size_, std::vector<long>(qapInstance_->size_));
}

/*******************************************************************************
 * デストラクタ
*******************************************************************************/
ExplorationRoTS::~ExplorationRoTS() {
}

/*******************************************************************************
 * 探索の実行
*******************************************************************************/
void ExplorationRoTS::Explore() {
    while (status_.currTrial <= condition_->numTrial) {
        /* 初期解を生成 */
        GenerateRandSol();
        /* アルゴリズムを適用 */
        TabuSearch();
        MsgExplore("RoTS");   // 探索開始メッセージ
        status_.currTrial++;  // 試行回数を進める
    }
}

/*******************************************************************************
 * 初期化
*******************************************************************************/
void ExplorationRoTS::Init() {
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

    /* タブーリスト初期化 */
    for (int i = 0; i < qapInstance_->size_; ++i) {
        for (int j = 0; j < qapInstance_->size_; ++j) {
            tabuList_[i][j] = -(qapInstance_->size_ * i + j);
        }
    }

    /* 差分リスト未登録数を初期化 */
    numRemnant_ = qapInstance_->size_ * (qapInstance_->size_ - 1) / 2;

    /* デルタを強制的に完成させてから探索を行う場合(検証用) */
    if (isForceDelta_ == true) {
        /* 差分リストを作る */
        for (int i = 0; i < qapInstance_->size_ - 1; ++i) {
            for (int j = i + 1; j < qapInstance_->size_; ++j) {
                delta_[i][j] = ComputeDelta(i, j);
                --numRemnant_;
            }
        }
    }
}

/*******************************************************************************
 * タブーサーチ
*******************************************************************************/
void ExplorationRoTS::TabuSearch() {
    long minDelta;    // 差分評価値の最小
    bool authorized;  // タブー状態のフラグ
    bool aspired;     // タブーの例外フラグ
    bool retained;    // 交換要素選択済みのフラグ
    long aspiration;  // 強制移動のパラメータ
    int minTabuDur;   // タブー期間の最小
    int maxTabuDur;   // タブー期間の最大
    int r, s;         // 交換する要素のインデックス

    Init();  // 解とリストの初期化

    /* パラメータの設定 */
    if (condition_->param.size() == 3) {
        aspiration = (long)condition_->param[0];
        minTabuDur = (int)condition_->param[1];
        maxTabuDur = (int)condition_->param[2];
    } else {
        aspiration = 2 * qapInstance_->size_ * qapInstance_->size_;
        minTabuDur = 9 / 10 * qapInstance_->size_;
        maxTabuDur = 11 / 10 * qapInstance_->size_;
    }

    StartMeas();  // 計測開始

    /*********************** タブーサーチのメインループ ***********************/
    /************************* 最良な(r, s)の組を探す *************************/
    while (IsEndExploration() == false) {
        status_.currIter++;  // 反復回数をインクリメント
        /* 初期化 */
        r = NA;            // 交換候補をリセット
        s = NA;            // すべての操作がタブーだとこれに引っかかる
        minDelta = INF;    // 差分評価値の最良をリセット
        retained = false;  // 交換候補を未選択にリセット

        /* 差分リストを作る部分 */
        if (numRemnant_ > 0) {
            for (int i = 0; i < qapInstance_->size_ - 1; ++i) {
                for (int j = i + 1; j < qapInstance_->size_; ++j) {
                    delta_[i][j] = ComputeDelta(i, j);
                    --numRemnant_;
                    if (IsEndExploration() == true) {
                        i = qapInstance_->size_ - 2;
                        j = qapInstance_->size_ - 1;
                    }
                }
            }
        }
        /* 差分リスト完成時の処理 */
        else if (numRemnant_ == 0) {
            --numRemnant_;
            /* 差分評価リスト完成時の状況を記憶 */
            SetCompDeltaStatus();
            status_.currIter--;  // 反復回数を修正
        }
        /* RoTS部分 */
        else if (numRemnant_ < 0) {
            /* 解順列の交換する要素を選ぶ */
            for (int i = 0; i < qapInstance_->size_ - 1; ++i) {
                for (int j = i + 1; j < qapInstance_->size_; ++j) {
                    /* タブー状態のフラグをセット */
                    authorized = (tabuList_[i][p_[j]] < status_.currIter) ||
                                 (tabuList_[j][p_[i]] < status_.currIter);
                    /* タブーの例外フラグをセット */
                    aspired = (tabuList_[i][p_[j]] < status_.currIter - aspiration) ||
                              (tabuList_[j][p_[i]] < status_.currIter - aspiration) ||
                              (status_.currCost + delta_[i][j] < qapInstance_->cost_);

                    /* 交換する要素を選択 */
                    if ((aspired && !retained) ||
                        (aspired && retained && (delta_[i][j] < minDelta)) ||
                        (!aspired && !retained && (delta_[i][j] < minDelta) && authorized)) {
                        r = i;
                        s = j;                    // 最良を指すインデックスを保持
                        minDelta = delta_[i][j];  // 最良の差分評価値を更新
                        if (aspired) {
                            retained = true;  // フラグを選択済みにセット
                        }
                    }
                }
            }

            /* 交換する要素がある => 交換・タブー設定・解と差分リスト更新 */
            if (r == NA) {
                std::cout << "All moves are tabu!" << std::endl;
            } else {
                status_.transCnt++;
                /* 解の要素を交換 */
                Swap(p_[r], p_[s]);
                /* 現在の評価値を更新 */
                status_.currCost = status_.currCost + delta_[r][s];
                /* 乱数を利用したタブーを設定 */
                tabuList_[r][p_[s]] = status_.currIter + (int)(Unif(minTabuDur, maxTabuDur));
                tabuList_[s][p_[r]] = status_.currIter + (int)(Unif(minTabuDur, maxTabuDur));

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
    }

    StopMeas();  // 計測終了
}
