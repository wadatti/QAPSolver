/*******************************************************************************
 * ExplorationIGDVPN
 *
 * ランダム選択・完全再生型
*******************************************************************************/
#ifndef EXPLORATIONIGDVPN_H
#define EXPLORATIONIGDVPN_H

#include "ExplorationBase.h"

class ExplorationIGDVPN : public ExplorationBase {
   private:
    std::vector<std::vector<long> > tabuList_;  // タブーリスト

   public:
    /* コンストラクタ */
    ExplorationIGDVPN(QAP* qapInst, ExpCond* cond, std::string fileName);
    /* デストラクタ */
    ~ExplorationIGDVPN();
    /* 探索の実行 */
    void Explore();

   private:
    /* 初期化 */
    void Init();
    /* 解の再構成処理 */
    void Destruction(double ratio);
    /* タブーサーチ */
    void TabuSearch();
};

#endif /* EXPLORATIONIGDVPN_H */
