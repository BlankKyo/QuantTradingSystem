// src/core/Report.cpp
#include "core/Report.h"
#include "utils/Logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <algorithm>

namespace trading {

static const char* TAG = "Report";

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Report::Report(const std::string& outputPath)
    : outputPath_(outputPath)
    , timestamp_(now())
{}

// ─────────────────────────────────────────────
//  setDataInfo
// ─────────────────────────────────────────────
void Report::setDataInfo(const std::string& path, int bars,
                          const std::string& from, const std::string& to)
{
    dataPath_ = path;
    bars_     = bars;
    dateFrom_ = from;
    dateTo_   = to;
}

// ─────────────────────────────────────────────
//  addBacktestResults
// ─────────────────────────────────────────────
void Report::addBacktestResults(const std::vector<BacktestResult>& results) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);

    ss << "  Data File   : " << dataPath_ << "\n";
    ss << "  Bars        : " << bars_ << "\n";
    ss << "  Period      : " << dateFrom_ << " -> " << dateTo_ << "\n\n";

    // Table header
    ss << "  " << std::left
       << std::setw(28) << "Strategy"
       << std::setw(12) << "Return(%)"
       << std::setw(14) << "PnL($)"
       << std::setw(10) << "Trades"
       << "\n";
    ss << "  " << std::string(64, '-') << "\n";

    for (const auto& r : results) {
        ss << "  " << std::left
           << std::setw(28) << r.strategyName
           << std::setw(12) << r.totalReturn
           << std::setw(14) << r.totalPnL
           << std::setw(10) << r.trades.size()
           << "\n";
    }

    sections_.push_back({"BACKTEST RESULTS", ss.str()});
}

// ─────────────────────────────────────────────
//  addMetrics
// ─────────────────────────────────────────────
void Report::addMetrics(const std::vector<MetricsResult>& metrics) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);

    // Comparison table
    ss << "  " << std::left
       << std::setw(26) << "Strategy"
       << std::setw(10) << "Return%"
       << std::setw(10) << "Sharpe"
       << std::setw(10) << "MaxDD%"
       << std::setw(10) << "Vol%"
       << std::setw(10) << "WinRate%"
       << std::setw(10) << "PFactor"
       << "\n";
    ss << "  " << std::string(86, '-') << "\n";

    for (const auto& m : metrics) {
        ss << "  " << std::left
           << std::setw(26) << m.strategyName
           << std::setw(10) << m.totalReturn
           << std::setw(10) << m.sharpeRatio
           << std::setw(10) << m.maxDrawdown
           << std::setw(10) << m.volatility
           << std::setw(10) << m.winRate
           << std::setw(10) << m.profitFactor
           << "\n";
    }

    // Best strategy
    if (!metrics.empty()) {
        const MetricsResult* best = &metrics[0];
        for (const auto& m : metrics)
            if (m.sharpeRatio > best->sharpeRatio) best = &m;
        ss << "\n  Best Sharpe  : " << best->strategyName
           << " (" << std::setprecision(4) << best->sharpeRatio << ")\n";
    }

    sections_.push_back({"PERFORMANCE METRICS", ss.str()});
}

// ─────────────────────────────────────────────
//  addOptimization
// ─────────────────────────────────────────────
void Report::addOptimization(const std::string& strategyName,
                              const std::vector<OptimizationResult>& results,
                              int topN)
{
    if (results.empty()) return;

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);

    int n = std::min(topN, (int)results.size());
    ss << "  Strategy : " << strategyName << "\n";
    ss << "  Total combinations tested : " << results.size() << "\n\n";

    // Header
    ss << "  " << std::left << std::setw(6) << "Rank";
    for (const auto& name : results[0].paramNames)
        ss << std::setw(14) << name;
    ss << std::setw(10) << "Return%"
       << std::setw(10) << "Sharpe"
       << std::setw(10) << "MaxDD%"
       << "\n";
    ss << "  " << std::string(6 + 14 * results[0].paramNames.size() + 30, '-') << "\n";

    for (int i = 0; i < n; ++i) {
        const auto& r = results[i];
        ss << "  " << std::left << std::setw(6) << (i + 1);
        for (double p : r.params)
            ss << std::setw(14) << p;
        ss << std::setw(10) << r.metrics.totalReturn
           << std::setw(10) << r.metrics.sharpeRatio
           << std::setw(10) << r.metrics.maxDrawdown
           << "\n";
    }

    // Best params
    ss << "\n  Best Parameters:\n";
    for (size_t i = 0; i < results[0].paramNames.size(); ++i)
        ss << "    " << results[0].paramNames[i] << " = " << results[0].params[i] << "\n";
    ss << "    Sharpe = " << results[0].metrics.sharpeRatio << "\n";
    ss << "    Return = " << results[0].metrics.totalReturn << "%\n";

    sections_.push_back({"OPTIMIZATION — " + strategyName, ss.str()});
}

// ─────────────────────────────────────────────
//  save() — write full report to file
// ─────────────────────────────────────────────
void Report::save() const {
    std::ostringstream full;

    // Header
    full << separator('=') << "\n";
    full << "  " << title_ << "\n";
    full << "  Generated: " << timestamp_ << "\n";
    full << separator('=') << "\n\n";

    // Sections
    for (const auto& s : sections_) {
        full << separator('-') << "\n";
        full << "  " << s.title << "\n";
        full << separator('-') << "\n";
        full << s.content << "\n";
    }

    full << separator('=') << "\n";
    full << "  END OF REPORT\n";
    full << separator('=') << "\n";

    std::string content = full.str();

    // Print to terminal
    std::cout << content;

    // Save to file
    if (!outputPath_.empty()) {
        std::ofstream f(outputPath_);
        if (f.is_open()) {
            f << content;
            LOG_INFO(TAG, "Report saved to: " + outputPath_);
        } else {
            LOG_WARNING(TAG, "Could not save report to: " + outputPath_);
        }
    }
}

// ─────────────────────────────────────────────
//  printSummary() — brief terminal summary
// ─────────────────────────────────────────────
void Report::printSummary() const {
    std::cout << "\n" << separator() << "\n";
    std::cout << "  " << title_ << "\n";
    std::cout << "  Sections: " << sections_.size() << "\n";
    if (!outputPath_.empty())
        std::cout << "  Saved to: " << outputPath_ << "\n";
    std::cout << separator() << "\n\n";
}

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
std::string Report::now() {
    auto tp     = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Report::separator(char c, int width) {
    return std::string(width, c);
}

std::string Report::formatDouble(double v, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << v;
    return ss.str();
}

} // namespace trading