// src/core/Optimizer.cpp
#include "core/Optimizer.h"
#include "utils/Logger.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace trading {

static const char* TAG = "Optimizer";

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Optimizer::Optimizer(const MarketData& data,
                     double            initialCash,
                     int               tradingDays)
    : data_(data)
    , initialCash_(initialCash)
    , tradingDays_(tradingDays)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2)
        << "Initialized | Bars=" << data_.size()
        << " | Cash=$" << initialCash_
        << " | Metric=" << metricName(metric_);
    LOG_INFO(TAG, oss.str());
}

// ─────────────────────────────────────────────
//  totalCombinations() — count before running
// ─────────────────────────────────────────────
int Optimizer::totalCombinations(const std::vector<ParamRange>& ranges) const {
    int total = 1;
    for (const auto& r : ranges)
        total *= (int)r.values().size();
    return total;
}

// ─────────────────────────────────────────────
//  gridSearch()
//
//  Recursively expands all parameter combinations,
//  runs each through Portfolio, computes Metrics,
//  stores result.
//
//  After all runs, sorts by score (ascending or
//  descending depending on metric).
// ─────────────────────────────────────────────
std::vector<OptimizationResult> Optimizer::gridSearch(
    const std::vector<ParamRange>&                            ranges,
    std::function<std::shared_ptr<Strategy>(std::vector<double>)> factory)
{
    if (ranges.empty())
        throw std::invalid_argument("[Optimizer] No parameter ranges provided");

    int total = totalCombinations(ranges);
    LOG_INFO(TAG, "Starting grid search | Combinations=" + std::to_string(total)
        + " | Metric=" + metricName(metric_));

    std::vector<OptimizationResult> results;
    results.reserve(total);

    std::vector<double> current;
    expand(ranges, factory, current, 0, results);

    // Remove invalid results
    results.erase(
        std::remove_if(results.begin(), results.end(),
            [](const OptimizationResult& r) { return !r.valid; }),
        results.end()
    );

    // Sort by score
    bool minimize = (metric_ == OptimizationMetric::MAX_DRAWDOWN);
    std::sort(results.begin(), results.end(),
        [&](const OptimizationResult& a, const OptimizationResult& b) {
            return minimize
                ? score(a.metrics) < score(b.metrics)
                : score(a.metrics) > score(b.metrics);
        }
    );

    LOG_INFO(TAG, "Grid search complete | Valid=" + std::to_string(results.size())
        + "/" + std::to_string(total));

    if (!results.empty()) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4)
            << "Best | " << metricName(metric_) << "="
            << score(results[0].metrics);
        for (size_t i = 0; i < results[0].paramNames.size(); ++i)
            oss << " | " << results[0].paramNames[i] << "=" << results[0].params[i];
        LOG_INFO(TAG, oss.str());
    }

    return results;
}

// ─────────────────────────────────────────────
//  expand() — recursive parameter expansion
//
//  Builds every combination by fixing each param
//  one level at a time, recursing into the next.
//  At the leaf (depth == ranges.size()), runs
//  the strategy and stores the result.
// ─────────────────────────────────────────────
void Optimizer::expand(
    const std::vector<ParamRange>&                                ranges,
    std::function<std::shared_ptr<Strategy>(std::vector<double>)> factory,
    std::vector<double>&                                          current,
    size_t                                                        depth,
    std::vector<OptimizationResult>&                              results)
{
    if (depth == ranges.size()) {
        // ── Leaf: run this parameter combination ──────────
        OptimizationResult result;
        result.params = current;
        for (const auto& r : ranges)
            result.paramNames.push_back(r.name);

        try {
            auto strategy = factory(current);
            if (!strategy) { result.valid = false; results.push_back(result); return; }

            auto signals = strategy->generateSignals(data_);
            if (signals.empty()) { result.valid = false; results.push_back(result); return; }

            Portfolio portfolio(initialCash_);
            portfolio.run(signals, data_);

            BacktestResult br;
            br.strategyName = strategy->name();
            br.trades       = portfolio.trades();
            br.equityCurve  = portfolio.equityCurve();
            br.initialCash  = initialCash_;
            br.finalEquity  = portfolio.finalEquity();
            br.totalReturn  = portfolio.totalReturn();
            br.totalPnL     = portfolio.totalPnL();

            result.metrics = Metrics::compute(br, 0.0, tradingDays_);
            result.valid   = true;

        } catch (const std::exception& e) {
            LOG_DEBUG(TAG, std::string("Skipped combination: ") + e.what());
            result.valid = false;
        }

        results.push_back(result);
        return;
    }

    // ── Branch: iterate this parameter's values ───────────
    for (double val : ranges[depth].values()) {
        current.push_back(val);
        expand(ranges, factory, current, depth + 1, results);
        current.pop_back();
    }
}

// ─────────────────────────────────────────────
//  score() — maps MetricsResult to a single
//  comparable number for ranking
// ─────────────────────────────────────────────
double Optimizer::score(const MetricsResult& m) const {
    switch (metric_) {
        case OptimizationMetric::SHARPE_RATIO:  return m.sharpeRatio;
        case OptimizationMetric::TOTAL_RETURN:  return m.totalReturn;
        case OptimizationMetric::MAX_DRAWDOWN:  return m.maxDrawdown;  // minimize
        case OptimizationMetric::PROFIT_FACTOR: return m.profitFactor;
        case OptimizationMetric::WIN_RATE:      return m.winRate;
        default:                                return m.sharpeRatio;
    }
}

// ─────────────────────────────────────────────
//  printResults() — top N results table
// ─────────────────────────────────────────────
void Optimizer::printResults(const std::vector<OptimizationResult>& results,
                              int topN) const
{
    if (results.empty()) {
        std::cout << "[Optimizer] No valid results.\n";
        return;
    }

    int n = std::min(topN, (int)results.size());

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  OPTIMIZATION RESULTS  (top " << n << " by " << metricName(metric_) << ")\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";

    // Header — param names + metric columns
    std::cout << std::left;
    int rankW  = 6;
    int paramW = 10;
    int metW   = 10;

    std::cout << std::setw(rankW) << "Rank";
    for (const auto& name : results[0].paramNames)
        std::cout << std::setw(paramW) << name;
    std::cout << std::setw(metW) << "Return%"
              << std::setw(metW) << "Sharpe"
              << std::setw(metW) << "MaxDD%"
              << std::setw(metW) << "WinRate%"
              << std::setw(metW) << "PFactor"
              << "\n";
    std::cout << std::string(rankW + paramW * results[0].paramNames.size() + metW * 5, '-') << "\n";

    for (int i = 0; i < n; ++i) {
        const auto& r = results[i];
        std::cout << std::setw(rankW) << (i + 1);
        for (double p : r.params)
            std::cout << std::setw(paramW) << p;
        std::cout << std::setw(metW) << r.metrics.totalReturn
                  << std::setw(metW) << r.metrics.sharpeRatio
                  << std::setw(metW) << r.metrics.maxDrawdown
                  << std::setw(metW) << r.metrics.winRate
                  << std::setw(metW) << r.metrics.profitFactor
                  << "\n";
    }
}

// ─────────────────────────────────────────────
//  printBest()
// ─────────────────────────────────────────────
void Optimizer::printBest(const std::vector<OptimizationResult>& results) const {
    if (results.empty()) {
        std::cout << "[Optimizer] No valid results.\n";
        return;
    }

    const auto& best = results[0];
    std::cout << "\n  Best Parameters (" << metricName(metric_) << "):\n";
    for (size_t i = 0; i < best.paramNames.size(); ++i)
        std::cout << "    " << best.paramNames[i] << " = " << best.params[i] << "\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "    Sharpe  = " << best.metrics.sharpeRatio << "\n";
    std::cout << "    Return  = " << best.metrics.totalReturn << "%\n";
    std::cout << "    MaxDD   = " << best.metrics.maxDrawdown << "%\n";
}

// ─────────────────────────────────────────────
//  metricName()
// ─────────────────────────────────────────────
std::string Optimizer::metricName(OptimizationMetric m) {
    switch (m) {
        case OptimizationMetric::SHARPE_RATIO:  return "SharpeRatio";
        case OptimizationMetric::TOTAL_RETURN:  return "TotalReturn";
        case OptimizationMetric::MAX_DRAWDOWN:  return "MaxDrawdown";
        case OptimizationMetric::PROFIT_FACTOR: return "ProfitFactor";
        case OptimizationMetric::WIN_RATE:      return "WinRate";
        default:                                return "Unknown";
    }
}

} // namespace trading