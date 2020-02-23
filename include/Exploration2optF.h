/*******************************************************************************
 * Exploration2-optF
 *
 * 即時移動戦略を用いた2-opt局所探索
*******************************************************************************/
#ifndef EXPLORATION2OPTF_H
#define EXPLORATION2OPTF_H

#include "ExplorationBase.h"

class Exploration2optF : public ExplorationBase {
   public:
    /* コンストラクタ */
    Exploration2optF(QAP* qapInst, ExpCond* cond, std::string fileName);
    /* デストラクタ */
    ~Exploration2optF();
    /* 探索の実行 */
    void Explore();

   private:
    /* 初期化 */
    void Init();
    /* 局所探索 */
    void LoacalSearch();
};

#endif /* EXPLORATION2OPTF_H */
