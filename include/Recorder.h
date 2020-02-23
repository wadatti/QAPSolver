/*******************************************************************************
 * Recorder
 *
 * 実験結果を記録
*******************************************************************************/
#ifndef RECORDER_H
#define RECORDER_H

#include <sstream>
#include "Problem.h"
#include "struct.h"

class Recorder {
   private:
    std::string date_;                      // 実行日時
    std::string problemFileName_;           // 問題名
    QAP *qapInstance_;                      // QAPインスタンスのポインタ
    ExpCond *condition_;                    // 実験の条件のポインタ
    std::ostringstream resultFileOStream_;  // 結果ファイル出力用ストリーム

   public:
    /* コンストラクタ */
    Recorder(std::string date, std::string fileName);
    /* デストラクタ */
    ~Recorder();
    /* 実験の条件を設定 */
    void SetCondition(ExpCond *condition);
    /* QAPインスタンスを設定 */
    void SetProblem(QAP *qapInstance);
    /* 結果を出力 (ダミー) */
    void ShowResult();
    /* 結果をファイルに出力 */
    void RecResult();
    /* 解をファイルに出力 */
    void RecSolution();

   private:
    /* テンポラリファイルを読み込む */
    bool IsOpenTempFiles(std::vector<std::vector<long> > &cost,
                         std::vector<std::vector<long> > &iter,
                         std::vector<std::vector<long> > &trans,
                         std::vector<std::vector<long> > &callCD,
                         std::vector<std::vector<long> > &callCDP,
                         std::vector<std::vector<long> > &param,
                         std::vector<std::vector<double> > &time,
                         std::vector<std::vector<double> > &compDelta);
    /* 個別の試行結果の設定を記録 */
    void RecExamInfo();
    /* 個別の試行結果のラベルを記録 */
    void RecMatrixLbl(std::string label, int n);

    /***************************************************************************
	 * 平均値を行列ごとに計算
	 *
	 * @param  &ave :		求める平均値
	 * @param  &matrix :	行列
	***************************************************************************/
    template <class T>
    void ComputeAve(std::vector<double> &ave, std::vector<std::vector<T> > &matrix) {
        int n = matrix.size();
        int m = matrix[0].size();

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                ave[j] += matrix[i][j];
            }
        }
        for (int j = 0; j < m; ++j) {
            ave[j] = ave[j] / n;
        }
    }

    /***************************************************************************
	 * 個別の試行結果を記録
	 *
	 * @param  matrix :	記録する行列
	***************************************************************************/
    template <class T>
    void RecMatrix(std::vector<std::vector<T> > matrix) {
        int n = matrix.size();
        int m = matrix[0].size();

        for (int j = 0; j < m; ++j) {
            resultFileOStream_ << ",";
            for (int i = 0; i < n; ++i) {
                if (matrix[i][j] == EXC) {
                    resultFileOStream_ << "N/A,";
                } else if (matrix[i][j] == NA) {
                    resultFileOStream_ << "N/A,";
                } else if (matrix[i][j] == INF) {
                    resultFileOStream_ << "INF,";
                } else {
                    resultFileOStream_ << std::fixed << matrix[i][j] << ",";
                }
            }
            resultFileOStream_ << "\n";
        }
    }

    /***************************************************************************
	 * 記録メッセージ
	***************************************************************************/
    void MsgRecordResult() {
        std::cout << "Result was recorded!" << std::endl;
    }

    /***************************************************************************
	 * 記録メッセージ
	***************************************************************************/
    void MsgRecordSolution() {
        std::cout << "Solution was recorded!" << std::endl;
    }
};
#endif /* RECORDER */
