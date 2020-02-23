#include <iostream>
#include <string>
#include <cmath>
#include "ExplorationIGDVPNK.h"
#include "ExplorationBase.h"
#include "struct.h"

/*******************************************************************************
 * コンストラクタ
 *
 * @param  qapInst :	問題インスタンス
 * @param  cond :		探索条件
 * @param  fileName :	出力ファイル名
*******************************************************************************/
ExplorationIGDVPNK::ExplorationIGDVPNK(QAP *qapInst, ExpCond *cond, std::string fileName) : ExplorationBase(qapInst, cond, fileName)
{
    /* 配列を確保 */
    p_ = std::vector<int>(qapInstance_->size_);
    flag_ = std::vector<bool>(qapInstance_->size_);
    delta_ = std::vector<std::vector<long> >(qapInstance_->size_, std::vector<long>(qapInstance_->size_));
    remnant_ = std::vector<int>(qapInstance_->size_ * (qapInstance_->size_ - 1) / 2);
}

/*******************************************************************************
 * デストラクタ
*******************************************************************************/
ExplorationIGDVPNK::~ExplorationIGDVPNK()
{
}

/*******************************************************************************
 * 探索の実行
*******************************************************************************/
void ExplorationIGDVPNK::Explore()
{
    while (status_.currTrial <= condition_->numTrial)
    {
        /* 初期解を生成 */
        GenerateRandSol();
        /* アルゴリズムを適用 */
        KoptSearch();
        MsgExplore("IGDVPNK"); // 探索開始メッセージ
        status_.currTrial++;   // 試行回数を進める
    }
}

/*******************************************************************************
 * 初期化
*******************************************************************************/
void ExplorationIGDVPNK::Init()
{
    /* 初期解をセット */
    for (int i = 0; i < qapInstance_->size_; ++i)
    {
        p_[i] = qapInstance_->pi_[i];
    }

    /* 現在のコストと差分リスト初期化 */
    status_.currCost = 0;
    for (int i = 0; i < qapInstance_->size_; ++i)
    {
        for (int j = 0; j < qapInstance_->size_; ++j)
        {
            status_.currCost += qapInstance_->flow_[i][j] * qapInstance_->distance_[p_[i]][p_[j]];
            if (i < j)
            {
                delta_[i][j] = INF;
            }
        }
    }
    qapInstance_->cost_ = status_.currCost;

    /* 差分リスト未登録数を初期化 */
    numRemnant_ = qapInstance_->size_ * (qapInstance_->size_ - 1) / 2;
    for (int i = 0; i < numRemnant_; ++i)
    {
        remnant_[i] = i;
    }

    /* デルタを強制的に完成させてから探索を行う場合(検証用) */
    if (isForceDelta_ == true)
    {
        /* 差分リストを作る */
        for (int i = 0; i < qapInstance_->size_ - 1; ++i)
        {
            for (int j = i + 1; j < qapInstance_->size_; ++j)
            {
                delta_[i][j] = ComputeDelta(i, j);
                --numRemnant_;
            }
        }
    }
}

/*******************************************************************************
 * 解の再構成処理
 *
 * @parami  ratio :	解を破壊する割合
*******************************************************************************/
void ExplorationIGDVPNK::Destruction(double ratio)
{
    /* 解の破壊数を設定 */
    int numDestruct = qapInstance_->size_ * ratio;

    /* 破壊目標を決定する配列を宣言 */
    std::vector<int> objDestruct(qapInstance_->size_);

    /* 要素がインデックスに対応するように初期化 */
    for (int i = 0; i < qapInstance_->size_; i++)
    {
        objDestruct[i] = i;
    }

    /* ランダムに入れ換える要素を決定 */
    for (int i = 0; i < qapInstance_->size_ - 1; i++)
    {
        Swap(objDestruct[i], objDestruct[Unif(i, qapInstance_->size_ - 1)]);
    }

    /* 破壊目標の配列をもとに解を破壊数までループを回して入れ変え */
    for (int i = 0; i < numDestruct - 1; i++)
    {
        Swap(p_[objDestruct[i]], p_[objDestruct[i + 1]]);
    }

    /* 現在のコストと差分リスト初期化 */
    status_.currCost = 0;
    for (int i = 0; i < qapInstance_->size_; ++i)
    {
        for (int j = 0; j < qapInstance_->size_; ++j)
        {
            status_.currCost += qapInstance_->flow_[i][j] * qapInstance_->distance_[p_[i]][p_[j]];
            if (i < j)
            {
                delta_[i][j] = INF;
            }
        }
    }
    /* 最良解が改善された時 */
    if (status_.currCost < qapInstance_->cost_)
    {
        qapInstance_->cost_ = status_.currCost;
        for (int k = 0; k < qapInstance_->size_; ++k)
        {
            qapInstance_->pi_[k] = p_[k];
        }
    }

    /* 差分リスト未登録数を初期化 */
    numRemnant_ = qapInstance_->size_ * (qapInstance_->size_ - 1) / 2;
    for (int i = 0; i < numRemnant_; ++i)
    {
        remnant_[i] = i;
    }
}

/*******************************************************************************
 * 探索
*******************************************************************************/
void ExplorationIGDVPNK::KoptSearch()
{
    long minDelta; // 差分評価値の最小
    int r, s;    // 交換する要素のインデックス
    int randNum; // 擬似乱数

    double m;          // 部分近傍のサイズ
    int prevR;         // 差分リスト更新処理直後の交換する要素のインデックス
    int prevS;         // 同上
    long prevMinDelta; // 差分リスト更新処理直後の差分評価値の最良
    double incVal;     // 部分近傍のパラメータ (卒論の実験では 1 で固定)
    double growSpeed;  // 同上					(卒論の実験では 0.1 から 0.5 を指定)
    double growAccela; // 同上					(卒論の実験では 0 で固定)
    int growSpeedCnt;  // 部分近傍管理用のカウンタ

    int stagnation;         //停滞許容回数
    int stagnationCnt;      //停滞回数カウンタ
    double destructRatio;   //再構成範囲

    /* k-opt局所探索　変数宣言 */
    long gain_max;
    long cost_prev;
    long cost_best;
    long gain;
    std::vector<int> p_best;
    std::vector<int> p_prev;
    std::vector<std::vector<long> > delta_best;
    std::vector<std::vector<long> > delta_prev;

    /* パラメータの設定(GDVPNK処理用) */
    incVal = 1;
    growSpeed = 0.3 * qapInstance_->size_;
    growAccela = 0 * qapInstance_->size_;
    m = 1;
    growSpeedCnt = 0;

    /* パラメータの設定 */
    if (condition_->param.size() == 2)
    {
        stagnation = (int)condition_->param[0] + 1;
        destructRatio = (double)condition_->param[1];
    }
    else
    {
        stagnation = 100;
        destructRatio = 0.5;
    }
    stagnationCnt = 0;
    status_.varParam = 0;

    prevR = NA;
    prevS = NA;
    prevMinDelta = INF;

    Init();
    StartMeas(); // 計測開始

    /*********************** メインループ ***********************/
    while (IsEndExploration() == false)
    {
        status_.currIter++; // 反復回数をインクリメント
        /* 初期化 */
        r = NA;         // 交換候補のリセット
        s = NA;
        minDelta = INF; // 差分評価値の最良値をリセット
        // retained = false; // 交換候補を未選択にセット

        /* 差分評価値リストを作りながら探索する部分 */
        if (numRemnant_ > 0)
        {
            int k = 0;
            while (k < m && numRemnant_ > 0)
            {
                /* ランダムな値を選ぶ */
                randNum = (int)(Rand() * numRemnant_);

                /* 交換候補をランダムに選ぶ */
                int j = (int)(1 + sqrt(8 * (double)remnant_[randNum] + 1)) / 2;
                int i = remnant_[randNum] - (j - 1) * j / 2;

                /* 計算済み差分評価値の残りを更新 */
                remnant_[randNum] = remnant_[numRemnant_ - 1];
                --numRemnant_;

                /* 部分近傍のサイズを変更 */
                ++growSpeedCnt;
                if (growSpeedCnt >= growSpeed)
                {
                    m += incVal;
                    growSpeed -= growAccela;
                    if (growSpeed < 1)
                    {
                        growSpeed = 1;
                    }
                    growSpeedCnt = 0;
                }

                /* 差分評価値を計算 */
                delta_[i][j] = ComputeDelta(i, j);

                /* 交換する要素を選択 */
                if (delta_[i][j] < minDelta)
                {
                    r = i;
                    s = j;                   // 最良を指すインデックスを保持
                    minDelta = delta_[i][j]; // 最良の差分評価値を更新
                }

                /* デルタが完成したら終了 */
                if (numRemnant_ <= 0)
                {
                    break;
                }
                if (IsEndExploration() == true)
                {
                    break;
                }

                ++k;
            }
            /* 交換要素の選択範囲に計算済みの差分評価値を含める */
            if (prevMinDelta < minDelta)
            {
                r = prevR;
                s = prevS;
                minDelta = prevMinDelta;
            }

            /* 交換する要素がある => 交換・タブー設定・解と差分リスト更新 */
            if (r == NA)
            {
                std::cout << "All moves are tabu!" << std::endl;
            }
            else
            {
                status_.transCnt++;
                /* 解の要素を交換 */
                Swap(p_[r], p_[s]);

                /* 現在の評価値を更新 */
                status_.currCost = status_.currCost + minDelta;

                /* 最良解が改善された時 */
                if (status_.currCost < qapInstance_->cost_)
                {
                    qapInstance_->cost_ = status_.currCost;
                    for (int i = 0; i < qapInstance_->size_; ++i)
                    {
                        qapInstance_->pi_[i] = p_[i];
                    }
                }

                /* 次回の反復のための選択済みを初期化 */
                prevR = NA;
                prevS = NA;
                prevMinDelta = INF;

                /* 差分リストを更新 */
                for (int i = 0; i < qapInstance_->size_ - 1; ++i)
                {
                    for (int j = i + 1; j < qapInstance_->size_; ++j)
                    {
                        if (delta_[i][j] != INF)
                        {
                            if (i != r && i != s && j != r && j != s)
                            {
                                delta_[i][j] = ComputeDeltaPart(i, j, r, s);
                                if (delta_[i][j] < prevMinDelta)
                                {
                                    prevR = i;
                                    prevS = j;
                                    prevMinDelta = delta_[i][j];
                                }
                            }
                            else
                            {
                                delta_[i][j] = ComputeDelta(i, j);
                                if (delta_[i][j] < prevMinDelta)
                                {
                                    prevR = i;
                                    prevS = j;
                                    prevMinDelta = delta_[i][j];
                                }
                            }
                        }
                    }
                }
            }
        }

        /* 差分リスト完成時の処理 */
        else if (numRemnant_ == 0)
        {
            --numRemnant_;
            /* 差分評価リスト完成時の状況を記憶 */
            SetCompDeltaStatus();
            status_.currIter--; // 反復回数を修正
        }

        /* k-opt局所探索部分 */
        else if (numRemnant_ < 0)
        {         
            /* k-opt局所探索 */
            /* 外ループ */
            while (IsEndExploration() == false)
            {
                p_prev = p_;
                delta_prev = delta_;
                cost_prev = status_.currCost;
                gain = 0;
                gain_max = 0;
                stagnationCnt = 0;
                /* フラグの初期化 */
                for (int i = 0; i < qapInstance_->size_; i++)
                {
                    flag_[i] = true;
                }
                
                /* 内ループ */
                while (IsEndExploration() == false)
                {
                    r = NA;
                    s = NA;
                    minDelta = INF;
                    /* 最良の交換要素を探索 */
                    for (int i = 0; i < qapInstance_->size_ - 1; ++i)
                    {
                        if (!flag_[i])
                        {
                            continue;
                        }
                        for (int j = i + 1; j < qapInstance_->size_; ++j)
                        {
                            if (!flag_[j])
                            {
                                continue;
                            }
                            /* 最良の差分評価値であるか評価 */
                            if (delta_[i][j] < minDelta)
                            {
                                r = i; // 最良を指すインデックスを保持
                                s = j;
                                minDelta = delta_[r][s]; // 最良の差分評価値を更新
                            }
                        }
                    }

                    /* 交換できる要素があったかどうか判定 */
                    if (r == NA || s == NA)
                    {
                        std::cout << "P=0" << std::endl;
                        break;
                    }

                    /* 要素を交換 */
                    Swap(p_[r], p_[s]);
                    status_.currCost += minDelta;
                    status_.transCnt++;

                    /* 差分リストを更新 */
                    for (int i = 0; i < qapInstance_->size_ - 1; ++i)
                    {
                        for (int j = i + 1; j < qapInstance_->size_; ++j)
                        {
                            if (i != r && i != s && j != r && j != s)
                            {
                                delta_[i][j] = ComputeDeltaPart(i, j, r, s);
                            }
                            else
                            {
                                delta_[i][j] = ComputeDelta(i, j);
                            }
                        }
                    }

                    /* 差分評価値を更新 */
                    gain = gain + minDelta;

                    /* 改悪仮移動を行う場合 */
                    if(minDelta >= 0){
                        stagnationCnt++;
                        std::cout << "stagnationCnt:" << stagnationCnt << std::endl;
                        if(stagnationCnt == stagnation)
                        {
                            break;
                            std::cout << "stagnationCnt==stagnation" << std::endl;
                        }
                    }
                    else{
                        stagnationCnt = 0;
                    }

                    /* 差分評価値が改善された場合 */
                    if (gain < gain_max)
                    {
                        gain_max = gain;
                        p_best = p_;
                        cost_best = status_.currCost;
                        delta_best = delta_;
                        if (stagnationCnt > 0)
                        {
                            status_.varParam++;
                        }
                    }

                    /* 交換禁止 */
                    flag_[r] = false;
                    flag_[s] = false;
                }

                /* 差分評価値が改善されれば更新 */
                if (gain_max < 0)
                {
                    p_ = p_best;
                    delta_ = delta_best;
                    status_.currCost = cost_best;
                }
                else
                {
                    p_ = p_prev;
                    delta_ = delta_prev;
                    status_.currCost = cost_prev;
                    break;
                }
            }

            /* 最良解が改善された時 */
            if (status_.currCost < qapInstance_->cost_)
            {
                qapInstance_->cost_ = status_.currCost;
                for (int k = 0; k < qapInstance_->size_; ++k)
                {
                    qapInstance_->pi_[k] = p_[k];
                }
            }
            else /* 改善されなければ解の再構成処理 */
            {
                Destruction(destructRatio);
                std::cout << "Destruction:" << status_.varParam << std::endl;
                prevR = NA;
                prevS = NA;
                prevMinDelta = INF;
                m = 1;
                growSpeedCnt = 0;
            }
        }
    }
    StopMeas(); // 計測終了
}


