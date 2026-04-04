// src/core/Backtester.cpp
#include "core/Backtester.h"
#include "utils/Logger.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace trading {

static const char* TAG = "Backtester";

// ─────────────────────────────────────────────
//  Constructor — load data immediately
// ─────────────────────────────────────────────
Backtester::Backtester(const std::string& dataPath, double initialCash)
    : data_(dataPath)
    , initialCash_(initialCash)
{
    if (initialCash_ <= 0.0)
        throw std::invalid_argument("[Backtester] initialCash must be positive");

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2)
        << "Initialized | Bars=" << data_.size()
        << " | InitialCash=$"    << initialCash_;
    LOG_INFO(TAG, oss.str());
}

// ─────────────────────────────────────────────
//  addStrategy()
// ─────────────────────────────────────────────
void Backtester::addStrategy(std::shared_ptr<Strategy> strategy) {
    if (!strategy)
        throw std::invalid_argument("[Backtester] Cannot add null strategy");

    strategies_.push_back(strategy);
    LOG_INFO(TAG, std::string("Strategy added: ") + strategy->name());
}

// ─────────────────────────────────────────────
//  run() — execute all strategies
// ─────────────────────────────────────────────
void Backtester::run() {
    if (strategies_.empty()) {
        LOG_WARNING(TAG, "run() called with no strategies — nothing to do");
        return;
    }

    results_.clear();
    LOG_INFO(TAG, "Starting backtest | Strategies=" + std::to_string(strategies_.size()));

    for (const auto& strategy : strategies_) {
        LOG_INFO(TAG, std::string("Running strategy: ") + strategy->name());
        results_.push_back(runSingle(*strategy));
    }

    LOG_INFO(TAG, "Backtest complete | Results=" + std::to_string(results_.size()));
}

// ─────────────────────────────────────────────
//  runSingle() — run one strategy through the pipeline
//
//  Pipeline:
//    Strategy::generateSignals(data)
//      → Portfolio::run(signals, data)
//        → BacktestResult
// ─────────────────────────────────────────────
BacktestResult Backtester::runSingle(const Strategy& strategy) const {
    // 1. Generate signals
    auto signals = strategy.generateSignals(data_);

    if (signals.empty()) {
        LOG_WARNING(TAG, std::string("No signals from strategy: ") + strategy.name());
        BacktestResult empty;
        empty.strategyName = strategy.name();
        empty.initialCash  = initialCash_;
        empty.finalEquity  = initialCash_;
        return empty;
    }

    // 2. Run portfolio simulation
    Portfolio portfolio(initialCash_);
    portfolio.run(signals, data_);

    // 3. Package result
    BacktestResult result;
    result.strategyName = strategy.name();
    result.signals      = signals;
    result.trades       = portfolio.trades();
    result.equityCurve  = portfolio.equityCurve();
    result.initialCash  = initialCash_;
    result.finalEquity  = portfolio.finalEquity();
    result.totalReturn  = portfolio.totalReturn();
    result.totalPnL     = portfolio.totalPnL();
    result.totalBars    = (int)data_.size();

    return result;
}

// ─────────────────────────────────────────────
//  printResults() — detailed output per strategy
// ─────────────────────────────────────────────
void Backtester::printResults() const {
    if (results_.empty()) {
        std::cout << "[Backtester] No results to display. Call run() first.\n";
        return;
    }

    for (const auto& r : results_) {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════╗\n";
        std::cout << "  Strategy : " << r.strategyName << "\n";
        std::cout << "╚══════════════════════════════════════════╝\n";

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Bars          : " << r.totalBars    << "\n";
        std::cout << "  Initial Cash  : $" << r.initialCash << "\n";
        std::cout << "  Final Equity  : $" << r.finalEquity << "\n";
        std::cout << "  Total PnL     : $" << r.totalPnL    << "\n";
        std::cout << "  Total Return  :  " << r.totalReturn << "%\n";
        std::cout << "  Trades Done   : " << r.trades.size() << "\n";

        // Win rate
        if (!r.trades.empty()) {
            int wins = 0;
            for (const auto& t : r.trades) if (t.pnl > 0) ++wins;
            double winRate = (double)wins / r.trades.size() * 100.0;
            std::cout << "  Win Rate      :  " << winRate
                      << "% (" << wins << "/" << r.trades.size() << ")\n";
        }

        // Trade table
        if (!r.trades.empty()) {
            std::cout << "\n  -- Trades --\n";
            std::cout << "  " << std::left
                      << std::setw(12) << "Entry"
                      << std::setw(12) << "Exit"
                      << std::setw(8)  << "Shares"
                      << std::setw(10) << "BuyPx"
                      << std::setw(10) << "SellPx"
                      << std::setw(12) << "PnL($)"
                      << std::setw(10) << "PnL(%)"
                      << "\n";
            std::cout << "  " << std::string(72, '-') << "\n";

            for (const auto& t : r.trades) {
                std::cout << "  " << std::left
                          << std::setw(12) << t.entryDate
                          << std::setw(12) << t.exitDate
                          << std::setw(8)  << t.shares
                          << std::setw(10) << t.entryPrice
                          << std::setw(10) << t.exitPrice
                          << std::setw(12) << t.pnl
                          << std::setw(10) << t.pnlPct
                          << (t.pnl >= 0 ? "WIN" : "LOSS")
                          << "\n";
            }
        }

        // Equity curve (last 5 bars)
        if (!r.equityCurve.empty()) {
            std::cout << "\n  -- Equity Curve (last 5 bars) --\n";
            std::cout << "  " << std::left
                      << std::setw(12) << "Date"
                      << std::setw(14) << "Cash"
                      << std::setw(14) << "Position"
                      << std::setw(14) << "TotalEquity"
                      << "\n";
            std::cout << "  " << std::string(52, '-') << "\n";

            size_t start = r.equityCurve.size() > 5
                         ? r.equityCurve.size() - 5 : 0;
            for (size_t i = start; i < r.equityCurve.size(); ++i) {
                const auto& ep = r.equityCurve[i];
                std::cout << "  " << std::left
                          << std::setw(12) << ep.date
                          << std::setw(14) << ep.cash
                          << std::setw(14) << ep.positionVal
                          << std::setw(14) << ep.totalEquity
                          << "\n";
            }
        }
    }
}

// ─────────────────────────────────────────────
//  printSummary() — comparison table across strategies
// ─────────────────────────────────────────────
void Backtester::printSummary() const {
    if (results_.empty()) return;

    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  BACKTEST SUMMARY\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";

    std::cout << std::left << std::fixed << std::setprecision(2)
              << std::setw(30) << "Strategy"
              << std::setw(12) << "Return(%)"
              << std::setw(14) << "PnL($)"
              << std::setw(10) << "Trades"
              << "\n";
    std::cout << std::string(66, '-') << "\n";

    for (const auto& r : results_) {
        std::cout << std::left << std::fixed << std::setprecision(2)
                  << std::setw(30) << r.strategyName
                  << std::setw(12) << r.totalReturn
                  << std::setw(14) << r.totalPnL
                  << std::setw(10) << r.trades.size()
                  << "\n";
    }

    // Best performer
    const BacktestResult* best = &results_[0];
    for (const auto& r : results_)
        if (r.totalReturn > best->totalReturn) best = &r;

    std::cout << "\n  Best strategy : " << best->strategyName
              << " (" << best->totalReturn << "%)\n";

    LOG_INFO(TAG, "Best strategy: " + best->strategyName
        + " return=" + std::to_string(best->totalReturn) + "%");
}

} // namespace trading