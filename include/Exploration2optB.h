/*******************************************************************************
 * Exploration2-optB
 *
 * 最良移動戦略を用いた2-opt局所探索
*******************************************************************************/
#ifndef EXPLORATION2OPTB_H
#define EXPLORATION2OPTB_H

#include "ExplorationBase.h"

class Exploration2optB : public ExplorationBase {
   public:
    /* コンストラクタ */
    Exploration2optB(QAP* qapInst, ExpCond* cond, std::string fileName);
    /* デストラクタ */
    ~Exploration2optB();
    /* 探索の実行 */
    void Explore();

   private:
    /* 初期化 */
    void Init();
    /* 局所探索 */
    void LoacalSearch();
};

#endif /* EXPLORATION2OPTB_H */
