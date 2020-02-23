/*******************************************************************************
 * ExplorationIGDVPN2optB
 *
 * ランダム選択・完全再生型(k-opt局所探索)
*******************************************************************************/
#ifndef EXPLORATIONIGDVPN2OPTB_H
#define EXPLORATIONIGDVPN2OPTB_H

#include "ExplorationBase.h"

class ExplorationIGDVPN2optB : public ExplorationBase
{
  public:
    /* コンストラクタ */
    ExplorationIGDVPN2optB(QAP *qapInst, ExpCond *cond, std::string fileName);
    /* デストラクタ */
    ~ExplorationIGDVPN2optB();
    /* 探索の実行 */
    void Explore();

  private:
    /* 初期化 */
    void Init();
    /* 解の再構成処理 */
    void Destruction(double ratio);
    /* 2optBサーチ */
    void LocalSearch();
};

#endif /* EXPLORATIONIGDVPN2OPTB_H */
