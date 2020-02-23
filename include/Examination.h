/*******************************************************************************
 * Examination
 *
 * 実験の設定を元に、問題・探索・出力のインスタンスを作成して実行
*******************************************************************************/
#ifndef EXAMINATION_H
#define EXAMINATION_H

#include <iostream>
#include <string>
#include "struct.h"

class Examination {
   private:
    int argCnt_;                          // コマンドライン引数の数
    std::vector<std::string> argVector_;  // コマンドライン引数
    int examMode_;                        // 動作モード
    std::string date_;                    // 実行日時
    std::string problemFileName_;         // 問題ファイル名

   public:
    /* コンストラクタ */
    Examination(int argc, char *argv[]);
    /* デストラクタ */
    ~Examination();
    /* 実行 */
    void Run();

   private:
    /* 読込 => 探索 => 記録 */
    void Examine(int numCond, std::vector<std::string> cond, int num);
    /* Examine()を複数回実行 */
    void Automate();
    /* mode1用の設定ファイルを読む */
    void ReadConfFile(int &numExam, std::vector<std::vector<std::string> > &conf);
    /* 探索条件を取得 */
    ExpCond GetCondition(int argC, std::vector<std::string> argV, int num);
    /* ファイル名用の日時を取得 */
    std::string GetDateNTime();
    /***************************************************************************
	 * プログラム開始メッセージ
	***************************************************************************/
    void MsgStartExperiment() {
        std::cout << std::endl
                  << "======== Experiment @ " << date_
                  << " ========" << std::endl;
    }
    /***************************************************************************
	 * 実験開始メッセージ
	***************************************************************************/
    void MsgStartExamination(int n, int m) {
        std::cout << std::endl
                  << "-------- Examination " << n << " / "
                  << m << " --------" << std::endl;
    }
    /***************************************************************************
	 * ファイル読込メッセージ
	***************************************************************************/
    void MsgReadConfig() {
        std::cout << "Read the config file!" << std::endl;
    }
};
#endif /* EXAMINATION_H */
