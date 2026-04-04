// include/core/Metrics.h
#pragma once

#include "core/Backtester.h"

#include <vector>
#include <string>

namespace trading {

// ─────────────────────────────────────────────
//  MetricsResult — all computed metrics for one
//  BacktestResult. Passed to printMetrics().
// ─────────────────────────────────────────────
struct MetricsResult {
    std::string strategyName    = "";

    // ── Return metrics ────────────────────────
    double totalReturn          = 0.0;   // % total return
    double totalPnL             = 0.0;   // $ profit/loss

    // ── Risk metrics ──────────────────────────
    double sharpeRatio          = 0.0;   // annualized Sharpe (risk-free=0)
    double maxDrawdown          = 0.0;   // % peak-to-trough max drawdown
    double maxDrawdownDuration  = 0;     // bars spent in drawdown
    double volatility           = 0.0;   // annualized std dev of daily returns

    // ── Trade metrics ─────────────────────────
    int    totalTrades          = 0;
    int    winningTrades        = 0;
    int    losingTrades         = 0;
    double winRate              = 0.0;   // %
    double avgWin               = 0.0;   // avg $ gain on winning trades
    double avgLoss              = 0.0;   // avg $ loss on losing trades
    double profitFactor         = 0.0;   // gross profit / gross loss
    double avgTradePnL          = 0.0;   // avg $ per trade
    double bestTrade            = 0.0;   // $ best single trade
    double worstTrade           = 0.0;   // $ worst single trade
};

// ─────────────────────────────────────────────
//  Metrics — static computation class
//
//  All methods are stateless — call compute()
//  on a BacktestResult to get a MetricsResult.
//
//  Usage:
//    auto m = Metrics::compute(result);
//    Metrics::print(m);
// ─────────────────────────────────────────────
class Metrics {
public:
    // Compute all metrics from one BacktestResult
    static MetricsResult compute(const BacktestResult& result,
                                  double riskFreeRate  = 0.0,
                                  int    tradingDaysPerYear = 252);

    // Compute for all results at once
    static std::vector<MetricsResult> computeAll(
        const std::vector<BacktestResult>& results,
        double riskFreeRate       = 0.0,
        int    tradingDaysPerYear = 252);

    // Print one MetricsResult
    static void print(const MetricsResult& m);

    // Print comparison table across all strategies
    static void printComparison(const std::vector<MetricsResult>& metrics);

private:
    // ── Core calculations ─────────────────────
    static std::vector<double> dailyReturns(const std::vector<EquityPoint>& curve);

    static double computeSharpe(const std::vector<double>& dailyRet,
                                 double riskFreeRate,
                                 int    tradingDaysPerYear);

    static double computeMaxDrawdown(const std::vector<EquityPoint>& curve);
    static int    computeMaxDrawdownDuration(const std::vector<EquityPoint>& curve);
    static double computeVolatility(const std::vector<double>& dailyRet,
                                     int tradingDaysPerYear);

    static void   computeTradeMetrics(const std::vector<Trade>& trades,
                                       MetricsResult& out);
};

} // namespace trading