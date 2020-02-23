/*******************************************************************************
 * ExplorationNakaMod
 *
 * 中浦らのアルゴリズムを改良した局所探索法
*******************************************************************************/
#ifndef EXPLORATIONNAKAMOD_H
#define EXPLORATIONNAKAMOD_H

#include "ExplorationBase.h"

class ExplorationNakaMod : public ExplorationBase {
   protected:
    std::vector<std::vector<long> > tabuList_;  // タブーリスト

   public:
    /* コンストラクタ */
    ExplorationNakaMod(QAP* qapInst, ExpCond* cond, std::string fileName);
    /* デストラクタ */
    ~ExplorationNakaMod();
    /* 探索の実行 */
    virtual void Explore();

   protected:
    /* 初期化 */
    void Init();
    /* タブーサーチ */
    void TabuSearch();
};

#endif /* EXPLORATIONNAKAMOD_H */
