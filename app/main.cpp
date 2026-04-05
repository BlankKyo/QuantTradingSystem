// app/main.cpp
#include <iostream>
#include <memory>
#include <stdexcept>

#include "core/Backtester.h"
#include "core/Metrics.h"
#include "core/Optimizer.h"
#include "strategy/MovingAverageStrategy.h"
#include "strategy/RSIStrategy.h"
#include "strategy/BollingerBandStrategy.h"
#include "utils/Logger.h"

int main(int argc, char* argv[]) {
    // ── Init Logger ───────────────────────────────────────
    try {
        trading::Logger::instance().init("logs/backtester.log", trading::LogLevel::DEBUG);
    } catch (const std::exception& e) {
        std::cerr << "[WARN] Could not open log file: " << e.what() << "\n";
    }

    LOG_INFO("Main", "QuantTradingSystem v0.8 starting");

    std::string dataPath    = (argc > 1) ? argv[1] : "../data/prices.csv";
    double      initialCash = 100000.0;

    try {
        // ── Load data once — shared across all runs ───────
        trading::MarketData data(dataPath);
        data.printSummary();

        // ════════════════════════════════════════
        //  SECTION 1 — Full Backtest (all strategies)
        // ════════════════════════════════════════
        trading::Backtester bt(dataPath, initialCash);
        bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(5, 20, trading::MAType::SMA));
        bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(5, 20, trading::MAType::EMA));
        bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(3, 10, trading::MAType::SMA));
        bt.addStrategy(std::make_shared<trading::RSIStrategy>(14, 30.0, 70.0));
        bt.addStrategy(std::make_shared<trading::BollingerBandStrategy>(20, 2.0));
        bt.run();
        bt.printSummary();

        auto metrics = trading::Metrics::computeAll(bt.results());
        trading::Metrics::printComparison(metrics);

        // ════════════════════════════════════════
        //  SECTION 2 — Optimize MA (SMA) parameters
        // ════════════════════════════════════════
        std::cout << "\n\n";
        std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "  PARAMETER OPTIMIZATION — MovingAverageStrategy (SMA)\n";
        std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";

        trading::Optimizer maOpt(data, initialCash);
        maOpt.setMetric(trading::OptimizationMetric::SHARPE_RATIO);

        int total = maOpt.totalCombinations({
            {"shortWindow", 2, 8, 1},
            {"longWindow",  10, 30, 5}
        });
        std::cout << "  Searching " << total << " combinations...\n";

        auto maResults = maOpt.gridSearch(
            { {"shortWindow", 2, 8, 1}, {"longWindow", 10, 30, 5} },
            [](std::vector<double> p) {
                int sw = (int)p[0], lw = (int)p[1];
                if (sw >= lw) return std::shared_ptr<trading::Strategy>(nullptr);
                return std::static_pointer_cast<trading::Strategy>(
                    std::make_shared<trading::MovingAverageStrategy>(sw, lw, trading::MAType::SMA)
                );
            }
        );
        maOpt.printResults(maResults, 5);
        maOpt.printBest(maResults);

        // ════════════════════════════════════════
        //  SECTION 3 — Optimize RSI parameters
        // ════════════════════════════════════════
        std::cout << "\n\n";
        std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "  PARAMETER OPTIMIZATION — RSIStrategy\n";
        std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";

        trading::Optimizer rsiOpt(data, initialCash);
        rsiOpt.setMetric(trading::OptimizationMetric::SHARPE_RATIO);

        int rsiTotal = rsiOpt.totalCombinations({
            {"period",     10, 20, 2},
            {"oversold",   20, 35, 5},
            {"overbought", 65, 80, 5}
        });
        std::cout << "  Searching " << rsiTotal << " combinations...\n";

        auto rsiResults = rsiOpt.gridSearch(
            { {"period", 10, 20, 2}, {"oversold", 20, 35, 5}, {"overbought", 65, 80, 5} },
            [](std::vector<double> p) {
                int    period      = (int)p[0];
                double oversold    = p[1];
                double overbought  = p[2];
                if (oversold >= overbought) return std::shared_ptr<trading::Strategy>(nullptr);
                return std::static_pointer_cast<trading::Strategy>(
                    std::make_shared<trading::RSIStrategy>(period, oversold, overbought)
                );
            }
        );
        rsiOpt.printResults(rsiResults, 5);
        rsiOpt.printBest(rsiResults);

        std::cout << "\nReady for v0.9 - Logging + Config\n\n";
        LOG_INFO("Main", "v0.8 complete - ready for v0.9");

    } catch (const std::exception& e) {
        LOG_ERROR("Main", std::string("Fatal: ") + e.what());
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }

    LOG_INFO("Main", "Session complete");
    return 0;
}