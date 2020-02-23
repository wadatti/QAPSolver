/*******************************************************************************
 * ExplorationIGDVPNK
 *
 * ランダム選択・完全再生型(k-opt局所探索)
*******************************************************************************/
#ifndef EXPLORATIONIGDVPNK_H
#define EXPLORATIONIGDVPNK_H

#include "ExplorationBase.h"

class ExplorationIGDVPNK : public ExplorationBase {
private:
  std::vector<bool> flag_; //k-opt局所探索（交換要素フラグ）

public:
  /* コンストラクタ */
  ExplorationIGDVPNK(QAP *qapInst, ExpCond *cond, std::string fileName);
  /* デストラクタ */
  ~ExplorationIGDVPNK();
  /* 探索の実行 */
  void Explore();
private:
	/* 初期化 */
	void Init();
	/* 解の再構成処理 */
	void Destruction(double ratio);
	/* k-optサーチ */
	void KoptSearch();
};

#endif /* EXPLORATIONIGDVPNK_H */
