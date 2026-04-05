// include/core/Report.h
#pragma once

#include "core/Backtester.h"
#include "core/Metrics.h"
#include "core/Optimizer.h"

#include <string>
#include <vector>

namespace trading {

// ─────────────────────────────────────────────
//  ReportSection — one named block of output
// ─────────────────────────────────────────────
struct ReportSection {
    std::string title;
    std::string content;
};

// ─────────────────────────────────────────────
//  Report — assembles and saves a full session report
//
//  Collects backtest results, metrics, and optimization
//  output into a structured text file.
//
//  Usage:
//    Report report("reports/session_2024.txt");
//    report.addBacktestResults(bt.results());
//    report.addMetrics(metrics);
//    report.addOptimization("MA", maResults, 5);
//    report.save();
//    report.printSummary();
// ─────────────────────────────────────────────
class Report {
public:
    explicit Report(const std::string& outputPath = "");

    void setTitle(const std::string& title) { title_ = title; }
    void setDataInfo(const std::string& path, int bars,
                     const std::string& from, const std::string& to);

    void addBacktestResults(const std::vector<BacktestResult>& results);
    void addMetrics        (const std::vector<MetricsResult>&  metrics);
    void addOptimization   (const std::string& strategyName,
                            const std::vector<OptimizationResult>& results,
                            int topN = 5);

    // Save to file and print to terminal
    void save()         const;
    void printSummary() const;

    // Access raw sections
    const std::vector<ReportSection>& sections() const { return sections_; }

private:
    std::string                title_      = "QuantTradingSystem — Backtest Report";
    std::string                outputPath_;
    std::string                dataPath_;
    int                        bars_       = 0;
    std::string                dateFrom_;
    std::string                dateTo_;
    std::vector<ReportSection> sections_;
    std::string                timestamp_;

    static std::string now();
    static std::string separator(char c = '=', int width = 60);
    static std::string formatDouble(double v, int precision = 4);
};

} // namespace trading