// app/main.cpp
#include <iostream>
#include <memory>
#include <stdexcept>

#include "core/Backtester.h"
#include "core/Metrics.h"
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

    LOG_INFO("Main", "QuantTradingSystem v0.7 starting");

    std::string dataPath    = (argc > 1) ? argv[1] : "../data/prices.csv";
    double      initialCash = 100000.0;

    try {
        trading::Backtester bt(dataPath, initialCash);

        // ── Moving Average strategies ─────────────────────
        bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(
            5, 20, trading::MAType::SMA));
        bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(
            5, 20, trading::MAType::EMA));
        bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(
            3, 10, trading::MAType::SMA));

        // ── RSI strategy ──────────────────────────────────
        bt.addStrategy(std::make_shared<trading::RSIStrategy>(
            14, 30.0, 70.0));

        // ── Bollinger Band strategy ───────────────────────
        bt.addStrategy(std::make_shared<trading::BollingerBandStrategy>(
            20, 2.0));

        // ── Run ───────────────────────────────────────────
        bt.run();
        bt.printResults();
        bt.printSummary();

        // ── Metrics ───────────────────────────────────────
        auto metrics = trading::Metrics::computeAll(bt.results());
        for (const auto& m : metrics)
            trading::Metrics::print(m);
        trading::Metrics::printComparison(metrics);

        std::cout << "\nReady for v0.8 - Parameter Optimization\n\n";
        LOG_INFO("Main", "v0.7 complete - ready for v0.8");

    } catch (const std::exception& e) {
        LOG_ERROR("Main", std::string("Fatal: ") + e.what());
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }

    LOG_INFO("Main", "Session complete");
    return 0;
}