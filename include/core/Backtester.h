// include/core/Backtester.h
#pragma once

#include "core/MarketData.h"
#include "core/Strategy.h"
#include "portfolio/Portfolio.h"

#include <memory>
#include <string>
#include <vector>

namespace trading {

// ─────────────────────────────────────────────
//  BacktestResult — full result of one run
//  Passed to performance metrics in v0.6
// ─────────────────────────────────────────────
struct BacktestResult {
    std::string              strategyName  = "";
    std::vector<TradeSignal> signals;
    std::vector<Trade>       trades;
    std::vector<EquityPoint> equityCurve;
    double                   initialCash   = 0.0;
    double                   finalEquity   = 0.0;
    double                   totalReturn   = 0.0;   // %
    double                   totalPnL      = 0.0;   // $
    int                      totalBars     = 0;
};

// ─────────────────────────────────────────────
//  Backtester — orchestrates the full pipeline
//
//  Pipeline per run:
//    MarketData → Strategy → Portfolio → BacktestResult
//
//  Usage:
//    Backtester bt("data/prices.csv", 100000.0);
//    bt.addStrategy(std::make_shared<MovingAverageStrategy>(5, 20));
//    bt.run();
//    bt.printResults();
// ─────────────────────────────────────────────
class Backtester {
public:
    Backtester(const std::string& dataPath,
               double             initialCash = 100000.0);

    // Add one or more strategies to run
    void addStrategy(std::shared_ptr<Strategy> strategy);

    // Run all strategies against the loaded data
    void run();

    // Output
    void printResults()  const;
    void printSummary()  const;

    // Accessors — used by v0.6 metrics
    const std::vector<BacktestResult>& results()    const { return results_;   }
    const MarketData&                  marketData() const { return data_;      }

private:
    MarketData                             data_;
    double                                 initialCash_;
    std::vector<std::shared_ptr<Strategy>> strategies_;
    std::vector<BacktestResult>            results_;

    BacktestResult runSingle(const Strategy& strategy) const;
};

} // namespace trading