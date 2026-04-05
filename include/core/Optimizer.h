// include/core/Optimizer.h
#pragma once

#include "core/Backtester.h"
#include "core/Metrics.h"

#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace trading {

// ─────────────────────────────────────────────
//  OptimizationResult — result of one parameter
//  combination tried during grid search
// ─────────────────────────────────────────────
struct OptimizationResult {
    std::vector<double> params;       // parameter values tried
    std::vector<std::string> paramNames; // parameter names
    MetricsResult       metrics;
    bool                valid = false; // false if strategy had no signals
};

// ─────────────────────────────────────────────
//  ParamRange — defines the search space
//  for one parameter
//
//  Usage:
//    ParamRange{"shortWindow", 3, 10, 1}
//    → tries 3, 4, 5, 6, 7, 8, 9, 10
// ─────────────────────────────────────────────
struct ParamRange {
    std::string name;
    double      start;
    double      stop;
    double      step;

    std::vector<double> values() const {
        std::vector<double> v;
        for (double x = start; x <= stop + 1e-9; x += step)
            v.push_back(x);
        return v;
    }
};

// ─────────────────────────────────────────────
//  OptimizationMetric — what to optimize for
// ─────────────────────────────────────────────
enum class OptimizationMetric {
    SHARPE_RATIO,
    TOTAL_RETURN,
    MAX_DRAWDOWN,    // minimize
    PROFIT_FACTOR,
    WIN_RATE
};

// ─────────────────────────────────────────────
//  Optimizer — grid search over parameter space
//
//  Runs every combination of parameters,
//  computes metrics for each, ranks by target metric.
//
//  Usage:
//    Optimizer opt(data, 100000.0);
//    opt.setMetric(OptimizationMetric::SHARPE_RATIO);
//    auto results = opt.gridSearch<MovingAverageStrategy>(
//        {{"shortWindow", 3, 10, 1}, {"longWindow", 10, 30, 5}},
//        [](double sw, double lw) {
//            return std::make_shared<MovingAverageStrategy>(sw, lw, MAType::SMA);
//        }
//    );
//    opt.printResults(results, 5);
// ─────────────────────────────────────────────
class Optimizer {
public:
    Optimizer(const MarketData&  data,
              double             initialCash   = 100000.0,
              int                tradingDays   = 252);

    void setMetric(OptimizationMetric metric) { metric_ = metric; }

    // Grid search — factory function creates strategy from params
    std::vector<OptimizationResult> gridSearch(
        const std::vector<ParamRange>&                            ranges,
        std::function<std::shared_ptr<Strategy>(std::vector<double>)> factory
    );

    // Print top N results
    void printResults(const std::vector<OptimizationResult>& results,
                      int topN = 10) const;

    // Print best result only
    void printBest(const std::vector<OptimizationResult>& results) const;

    int totalCombinations(const std::vector<ParamRange>& ranges) const;

private:
    const MarketData&  data_;
    double             initialCash_;
    int                tradingDays_;
    OptimizationMetric metric_ = OptimizationMetric::SHARPE_RATIO;

    double score(const MetricsResult& m) const;
    static std::string metricName(OptimizationMetric m);

    // Recursive grid expansion
    void expand(
        const std::vector<ParamRange>&                                ranges,
        std::function<std::shared_ptr<Strategy>(std::vector<double>)> factory,
        std::vector<double>&                                          current,
        size_t                                                        depth,
        std::vector<OptimizationResult>&                              results
    );
};

} // namespace trading