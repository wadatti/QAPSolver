/*******************************************************************************
 * ExplorationGDVPN
 *
 * 星らのアルゴリズムを用いた局所探索
*******************************************************************************/
#ifndef EXPLORATIONGDVPN_H
#define EXPLORATIONGDVPN_H

#include "ExplorationBase.h"

class ExplorationGDVPN : public ExplorationBase {
   private:
    std::vector<std::vector<long> > tabuList_;  // タブーリスト

   public:
    /* コンストラクタ */
    ExplorationGDVPN(QAP* qapInst, ExpCond* cond, std::string fileName);
    /* デストラクタ */
    ~ExplorationGDVPN();
    /* 探索の実行 */
    void Explore();

   private:
    /* 初期化 */
    void Init();
    /* タブーサーチ */
    void TabuSearch();
};

#endif /* EXPLORATIONGDVPN_H */
