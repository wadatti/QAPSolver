/*******************************************************************************
 * struct
 *
 * 構造体や定数を設定
*******************************************************************************/
#ifndef STRUCT_H
#define STRUCT_H

#include <climits>
#include <vector>

/* 以下は実行環境(プログラムを置くディレクトリ)に応じて変更する */
const long INF = LONG_MAX;                                 // 評価値の最悪値として
const int NA = INT_MAX;                                    // インデックス用のNULLの代わり
const long EXC = LONG_MIN;                                 // 例外値として
const char* const RROBLEM_FILE_DIR = "../../problems/";    // 問題ファイル格納場所
const char* const RESULT_FILE_DIR = "../../results/";      // 結果ファイル格納場所
const char* const SOLUTION_FILE_DIR = "../../solutions/";  // 解のファイル格納場所
const char* const TEMP_FILE_DIR = "./temporaries/";        // テンポラリファイル格納場所
const char* const CONFIG_FILE_DIR = "./config/";           // 設定ファイル格納場所
/* 実行環境に応じて変更するのはここまで */

/* 実行結果ファイル等に書き込むタグ */
const char* const LBL_PROBLEM = "[PROBLEM]";
const char* const LBL_SIZE = "[SIZE]";
const char* const LBL_ALGORITHM = "[ALGORITHM]";
const char* const LBL_PARAMETER = "[PARAMETER]";
const char* const LBL_DELTA = "[DELTA]";
const char* const LBL_AVERAGE = "[AVERAGE]";

const char* const LBL_COST = "[COST]";
const char* const LBL_ITER = "[ITER]";
const char* const LBL_TRANS = "[TRANS]";
const char* const LBL_CD = "[CD]";
const char* const LBL_CDP = "[CDP]";
const char* const LBL_PARAM = "[PARAM]";
const char* const LBL_TIME = "[TIME]";

/* 探索アルゴリズムの種類 */
enum ExpType {
    ROTS,        // RoTS
    TWO_OPT_F,   // 即時移動戦略を用いた2-opt局所探索
    TWO_OPT_B,   // 最良移動戦略を用いた2-opt局所探索
    NAKAURA,     // 中浦らの手法
    NAKAMOD,     // 中浦らの手法をリファインした手法
    GDVPN,       // 星らの手法(卒論)
    IGDVPN,      // 星らの手法(修論)
    IGDVPNK,     // 可変部分近傍探索+k-opt(提案手法1)
    IGDVPN2optB  // 可変部分近傍探索+2optB(提案手法2)
};

/* 終了条件の種類 */
enum Terminator {
    ITER,  // 反復回数
    TIME   // 実行時間
};

/* 実験の条件 */
typedef struct ExplorationCondition {
    int examNum;                // 実験No.
    ExpType type;               // アルゴリズムの種類
    int numTrial;               // 試行回数
    Terminator endCond;         // 終了条件の種類
    long endCondVal;            // 終了条件のしきい値
    int interval;               // 途中経過を記録する間隔
    std::vector<double> param;  // パラメータ
} ExpCond;

/* 探索の状況 */
typedef struct ExplorationStatus {
    int currTrial;          // 現在の試行回数
    long currCost;          // 現在の評価値
    long currIter;          // 現在の反復回数
    long numIter;           // 反復回数
    long transCnt;          // 遷移回数
    long callCDCnt;         // ComputeDelta()の呼び出し回数
    long callCDPCnt;        // ComputeDeltaPart()の呼び出し回数
    long varParam;          // 可変パラメータ (アルゴリズムによって自由に)
    double startTime;       // 開始時間
    double endTime;         // 終了(予定)時間
    double calcTime;        // 計算時間
    long costDelta;         // デルタ完成時の評価値
    long iterDelta;         // デルタ完成時の反復回数
    long transDelta;        // デルタ完成時の遷移回数
    long CDDelta;           // デルタ完成時のComputeDelta()の呼び出し回数
    long CDPDelta;          // デルタ完成時のComputeDeltaPart()の呼び出し回数
    long paramDelta;        // デルタ完成時の可変パラメータ (アルゴリズムによって自由に)
    double timeDelta;       // デルタの完成時間
    bool isEndExploration;  // 探索の終了フラグ
} ExpStatus;

#endif /* STRUCT_H */
