/*******************************************************************************
 * ExplorationBase
 *
 * 探索アルゴリズムの基底クラス
*******************************************************************************/
#ifndef EXPLORATIONBASE_H
#define EXPLORATIONBASE_H

#include <sstream>
#include "Problem.h"

class ExplorationBase {
   protected:
    QAP *qapInstance_;                       // QAPインスタンスのポインタ
    ExpCond *condition_;                     // 実験条件のポインタ
    ExpStatus status_;                       // 探索の状況
    std::string date_;                       // 実行日時 (ファイル名に使用)
    std::vector<int> p_;                     // 現在の解 (currCostと同義)
    std::vector<std::vector<long> > delta_;  // 差分リスト
    std::vector<int> remnant_;               // 差分リストの未登録リスト
    int numRemnant_;                         // 差分リストの未登録数
    std::ostringstream tempFileOStream_;     // テンポラリファイル出力用ストリーム
    bool isForceDelta_;                      // 探索前に強制的にデルタを完成させる
   public:
    /* コンストラクタ */
    ExplorationBase(QAP *qapInst, ExpCond *cond, std::string fileName);
    /* デストラクタ */
    virtual ~ExplorationBase();
    /* 探索の実行 */
    virtual void Explore() = 0;
    /* 探索前にデルタを強制的に完成させる */
    void SetForceDelta();

   protected:
    virtual void Init() = 0;
    /* 擬似乱数生成器 */
    double Rand();
    /* 初期解を生成 */
    void GenerateRandSol();
    /* 探索の終了判定 */
    bool IsEndExploration();
    /* 計測開始 */
    void StartMeas();
    /* デルタ完成時の探索状況を記憶 */
    void SetCompDeltaStatus();
    /* 計測終了 */
    void StopMeas();
    /* 現在時刻を取得 */
    double GetRusageSec();
    /* 探索の状況を記録 */
    void RecProgress();
    /* デルタ完成時の探索状況を記録 */
    void RecCompDeltaStatus();
    /* テンポラリファイルに書き込み */
    void WriteTempFile();

    /***************************************************************************
	 * 2つの値の間を返す
	 *
	 * @param  low :	戻り値の下限
	 * @param  high :	戻り値の上限
	 * @return		: low ≦ return ≦ high
	***************************************************************************/
    long Unif(int low, int high) {
        return (low + long(double(high - low + 1) * Rand()));
    }

    /***************************************************************************
	 * 要素を入れ替える
	 *
	 * @param  &a :	入れ替える要素
	 * @param  &b :	同上
	***************************************************************************/
    void Swap(int &a, int &b) {
        int temp = a;
        a = b;
        b = temp;
    }

    /***************************************************************************
	 * 差分評価値を計算
	 *
	 * @param  i :	入れ替える要素
	 * @param  j :	同上
	***************************************************************************/
    long ComputeDelta(int i, int j) {
        status_.callCDCnt++;

        long gain = (qapInstance_->flow_[i][i] - qapInstance_->flow_[j][j]) *
                        (qapInstance_->distance_[p_[j]][p_[j]] - qapInstance_->distance_[p_[i]][p_[i]]) +
                    (qapInstance_->flow_[i][j] - qapInstance_->flow_[j][i]) *
                        (qapInstance_->distance_[p_[j]][p_[i]] - qapInstance_->distance_[p_[i]][p_[j]]);

        for (int k = 0; k < qapInstance_->size_; ++k) {
            if (k != i && k != j) {
                gain += (qapInstance_->flow_[k][i] - qapInstance_->flow_[k][j]) *
                            (qapInstance_->distance_[p_[k]][p_[j]] - qapInstance_->distance_[p_[k]][p_[i]]) +
                        (qapInstance_->flow_[i][k] - qapInstance_->flow_[j][k]) *
                            (qapInstance_->distance_[p_[j]][p_[k]] - qapInstance_->distance_[p_[i]][p_[k]]);
            }
        }
        return gain;
    }

    /***************************************************************************
	 * 差分評価値の更新
	 *
	 * @param  i :	入れ替える要素
	 * @param  j :	同上
	 * @param  r :	入れ替えた要素
	 * @param  s :	同上
	***************************************************************************/
    long ComputeDeltaPart(int i, int j, int r, int s) {
        status_.callCDPCnt++;

        return (delta_[i][j] +
                (qapInstance_->flow_[r][i] - qapInstance_->flow_[r][j] +
                 qapInstance_->flow_[s][j] - qapInstance_->flow_[s][i]) *
                    (qapInstance_->distance_[p_[s]][p_[i]] - qapInstance_->distance_[p_[s]][p_[j]] +
                     qapInstance_->distance_[p_[r]][p_[j]] - qapInstance_->distance_[p_[r]][p_[i]]) +
                (qapInstance_->flow_[i][r] - qapInstance_->flow_[j][r] + qapInstance_->flow_[j][s] -
                 qapInstance_->flow_[i][s]) *
                    (qapInstance_->distance_[p_[i]][p_[s]] - qapInstance_->distance_[p_[j]][p_[s]] +
                     qapInstance_->distance_[p_[j]][p_[r]] - qapInstance_->distance_[p_[i]][p_[r]]));
    }

    /***************************************************************************
	 * 探索メッセージ
	 *
	 * @param  name :	アルゴリズム名
	***************************************************************************/
    void MsgExplore(std::string name) {
        std::cout << "Explored by " << name << " @ trial " << status_.currTrial
                  << " / " << condition_->numTrial << std::endl;
    }
};

#endif /* EXPLORATIONBASE_H */
