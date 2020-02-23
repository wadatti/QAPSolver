#ifdef WIN32
#include <windows.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#endif
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "ExplorationBase.h"
#include "MT.h"
#include "struct.h"

/*******************************************************************************
 * コンストラクタ
 *
 * @param  qapInst :	問題インスタンス
 * @param  cond :		探索条件
 * @param  fileName :	出力ファイル名
*******************************************************************************/
ExplorationBase::ExplorationBase(QAP* qapInst, ExpCond* cond, std::string fileName) {
    qapInstance_ = qapInst;
    condition_ = cond;
    date_ = fileName;

    /* 探索の状況を初期化 */
    status_.currTrial = 1;   // 現在の試行回数
    status_.currCost = INF;  // 現在の評価値
    status_.currIter = 0;    // 現在の反復回数
    status_.numIter = 0;     // 探索終了時点(予定)での反復回数
    status_.transCnt = 0;    // 最良解の更新回数
    status_.callCDCnt = 0;   // ComputeDelta()の呼び出し回数
    status_.callCDPCnt = 0;  // ComputeDeltaPart()の呼び出し回数
    status_.varParam = EXC;
    status_.startTime = 0;             // 探索開始時間
    status_.endTime = 0;               // 探索終了(予定)時間
    status_.calcTime = 0;              // 探索時間
    status_.costDelta = INF;           // デルタ完成時の評価値
    status_.iterDelta = 0;             // デルタ完成時の反復回数
    status_.transDelta = 0;            // デルタ完成時の遷移回数
    status_.timeDelta = 0;             // デルタの完成時間
    status_.isEndExploration = false;  // 探索の終了フラグ

    /* 乱数シードを設定 */
    mt::init_genrand((unsigned)time(NULL));

    isForceDelta_ = false;
}
/*******************************************************************************
 * デストラクタ
*******************************************************************************/
ExplorationBase::~ExplorationBase() {
}

/*******************************************************************************
 * 探索前にデルタを強制的に完成させる
*******************************************************************************/
void ExplorationBase::SetForceDelta() {
    isForceDelta_ = true;
}

/*******************************************************************************
 * 擬似乱数発生器
 *
 * @return rnd : (0, 1]
*******************************************************************************/
double ExplorationBase::Rand() {
    double rnd = mt::genrand_real2();

    if (rnd == 0) {
        return 1.0;
    } else {
        return rnd;
    }
}

/*******************************************************************************
 * 初期解を生成する
 *
 * ランダムな解を生成する
*******************************************************************************/
void ExplorationBase::GenerateRandSol() {
    for (int i = 0; i < qapInstance_->size_; ++i) {
        qapInstance_->pi_[i] = i;
    }

    for (int i = 0; i < qapInstance_->size_ - 1; ++i) {
        Swap(qapInstance_->pi_[i], qapInstance_->pi_[Unif(i, qapInstance_->size_ - 1)]);
    }
}

/*******************************************************************************
 * 探索の終了を判定
 *
 * @return boolean :	探索の終了条件を満たしたか
*******************************************************************************/
bool ExplorationBase::IsEndExploration() {
    /* 探索の終了条件が反復回数 */
    if (condition_->endCond == ITER) {
        if (status_.currIter >= status_.numIter) {
            /* 探索終了の場合 */
            if (status_.isEndExploration == true) {
                return true;
            }
            if (status_.numIter >= condition_->endCondVal) {
                RecProgress();
                RecCompDeltaStatus();
                status_.isEndExploration = true;
                return true;
            }
            /* 探索の経過を記録する場合 */
            else {
                RecProgress();
                status_.numIter += condition_->interval;
            }
        }
        return false;
    }
    /* 探索の終了条件が計算時間 */
    else if (condition_->endCond == TIME) {
        if (GetRusageSec() >= status_.endTime) {
            /* 探索終了の場合 */
            if (status_.isEndExploration == true) {
                return true;
            } else if (GetRusageSec() >= (condition_->endCondVal + status_.startTime)) {
                RecProgress();
                RecCompDeltaStatus();
                status_.isEndExploration = true;
                return true;
            }
            /* 探索の経過を記録する場合 */
            else {
                RecProgress();
                status_.endTime += condition_->interval;
            }
        }
        return false;
    } else {
        std::cout << "err about terminal condition of loop @ ExplorationBase.cpp" << std::endl;
        return false;
    }
}

/*******************************************************************************
 * 計測開始時の処理
*******************************************************************************/
void ExplorationBase::StartMeas() {
    status_.currIter = 0;
    status_.transCnt = 0;
    status_.callCDCnt = 0;
    status_.callCDPCnt = 0;

    status_.costDelta = INF;           // デルタ完成時の評価値
    status_.iterDelta = EXC;           // デルタ完成時の反復回数
    status_.transDelta = EXC;          // デルタ完成時の遷移回数
    status_.timeDelta = EXC;           // デルタの完成時間
    status_.isEndExploration = false;  // 探索の終了フラグ

    status_.startTime = GetRusageSec();  // 計測開始時刻を取得

    /* 探索の終了条件が反復回数 */
    if (condition_->endCond == ITER) {
        status_.numIter = condition_->interval;
    }
    /* 探索の終了条件が計算時間 */
    else if (condition_->endCond == TIME) {
        status_.endTime = condition_->interval + status_.startTime;
    } else {
        std::cout << "err about condition @ ExplorationBase.cpp" << std::endl;
    }

    /* 計測開始時のデータを記録 */
    RecProgress();
}

/*******************************************************************************
 * 差分リスト完成時の探索状況を記憶
*******************************************************************************/
void ExplorationBase::SetCompDeltaStatus() {
    status_.costDelta = qapInstance_->cost_;
    status_.iterDelta = status_.currIter;
    status_.transDelta = status_.transCnt;
    status_.CDDelta = status_.callCDCnt;
    status_.CDPDelta = status_.callCDPCnt;
    status_.paramDelta = status_.varParam;
    status_.timeDelta = GetRusageSec() - status_.startTime;
}

/*******************************************************************************
 * 計測終了時の処理
*******************************************************************************/
void ExplorationBase::StopMeas() {
    status_.calcTime = GetRusageSec() - status_.startTime;
    status_.numIter = status_.currIter;

    /* ストリームに溜めた記録をファイル出力 */
    WriteTempFile();
}

/*******************************************************************************
 * 現在時刻を取得
 *
 * @return :	現在のCPU時刻
*******************************************************************************/
#ifdef WIN32
double ExplorationBase::GetRusageSec() {
    LARGE_INTEGER time, freq;

    QueryPerformanceCounter(&time);
    QueryPerformanceFrequency(&freq);

    return static_cast<double>(time.QuadPart) / freq.QuadPart;
}
#else
double ExplorationBase::GetRusageSec() {
    struct rusage t;
    struct timeval s;

    getrusage(RUSAGE_SELF, &t);
    s = t.ru_utime;

    return s.tv_sec + (double)s.tv_usec * 1e-6;
}
#endif

/*******************************************************************************
 * 探索の状況をストリームに記録
*******************************************************************************/
void ExplorationBase::RecProgress() {
    /* 以下の現在の探索状況を記録 */
    /* "評価値" "反復回数" "解の遷移回数" "計算時間" */
    tempFileOStream_ << qapInstance_->cost_ << " ";
    tempFileOStream_ << status_.currIter << " ";
    tempFileOStream_ << status_.transCnt << " ";
    tempFileOStream_ << status_.callCDCnt << " ";
    tempFileOStream_ << status_.callCDPCnt << " ";
    tempFileOStream_ << status_.varParam << " ";
    tempFileOStream_ << GetRusageSec() - status_.startTime << "\n";
}

/*******************************************************************************
 * 差分リスト完成時の状況をストリームに記録
*******************************************************************************/
void ExplorationBase::RecCompDeltaStatus() {
    tempFileOStream_ << status_.costDelta << " ";
    tempFileOStream_ << status_.iterDelta << " ";
    tempFileOStream_ << status_.transDelta << " ";
    tempFileOStream_ << status_.CDDelta << " ";
    tempFileOStream_ << status_.CDPDelta << " ";
    tempFileOStream_ << status_.paramDelta << " ";
    tempFileOStream_ << status_.timeDelta << "\n";
}

/*******************************************************************************
 * ストリームの内容をテンポラリファイルに書き込み
*******************************************************************************/
void ExplorationBase::WriteTempFile() {
    /* テンポラリファイルの相対パスを決定 */
    /* 命名規則は "日付"_e"現在の実験回数"t"現在の試行回数".dat */
    std::string rslTempFilePath = TEMP_FILE_DIR;
    std::string solTempFilePath = TEMP_FILE_DIR;
    rslTempFilePath += "rsl" + date_;
    solTempFilePath += "sol" + date_;
    std::stringstream fns;
    fns << condition_->examNum;
    rslTempFilePath += "_e" + fns.str();
    solTempFilePath += "_e" + fns.str();
    fns.str("");
    fns << status_.currTrial;
    rslTempFilePath += "t" + fns.str() + ".dat";
    solTempFilePath += "t" + fns.str() + ".dat";

    /* ストリームの内容を書き込み */
    std::ofstream rslTempFile(rslTempFilePath.c_str());
    rslTempFile << tempFileOStream_.str();
    tempFileOStream_.str("");
    rslTempFile.close();

    /* 解順列を書き込み */
    std::ofstream solTempFile(solTempFilePath.c_str());
    solTempFile << qapInstance_->cost_ << "\n";
    int cnt = 0;
    for (int i = 0; i < qapInstance_->size_; ++i) {
        cnt++;
        solTempFile << qapInstance_->pi_[i] << " ";
        if (cnt >= 20) {
            solTempFile << "\n";
            cnt = 0;
        }
    }
}
