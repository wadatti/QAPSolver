/*******************************************************************************
 * ExplorationRoTS
 *
 * Robust Tabu Searchを用いた局所探索
*******************************************************************************/
#ifndef EXPLORATIONROTS_H
#define EXPLORATIONROTS_H

#include "ExplorationBase.h"

class ExplorationRoTS : public ExplorationBase {
   private:
    std::vector<std::vector<long> > tabuList_;  // タブーリスト

   public:
    /* コンストラクタ */
    ExplorationRoTS(QAP* qapInst, ExpCond* cond, std::string fileName);
    /* デストラクタ */
    ~ExplorationRoTS();
    /* 探索の実行 */
    void Explore();

   private:
    /* 初期化 */
    void Init();
    /* タブーサーチ */
    void TabuSearch();
};

#endif /* EXPLORATIONROTS_H */
