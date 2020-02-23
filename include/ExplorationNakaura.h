/*******************************************************************************
 * ExplorationNakaura
 *
 * 中浦らのアルゴリズムを用いた局所探索
*******************************************************************************/
#ifndef EXPLORATIONNAKAURA_H
#define EXPLORATIONNAKAURA_H

#include "ExplorationBase.h"

class ExplorationNakaura : public ExplorationBase {
   protected:
    std::vector<std::vector<long> > tabuList_;  // タブーリスト

   public:
    /* コンストラクタ */
    ExplorationNakaura(QAP* qapInst, ExpCond* cond, std::string fileName);
    /* デストラクタ */
    ~ExplorationNakaura();
    /* 探索の実行 */
    virtual void Explore();

   protected:
    /* 初期化 */
    void Init();
    /* タブーサーチ */
    void TabuSearch();
};

#endif /* EXPLORATIONNAKAURA_H */
