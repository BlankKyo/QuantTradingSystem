// app/main.cpp
#include <iostream>
#include <memory>
#include <stdexcept>

#include "core/Backtester.h"
#include "strategy/MovingAverageStrategy.h"
#include "utils/Logger.h"

int main(int argc, char* argv[]) {
    // ── Init Logger ───────────────────────────────────────
    try {
        trading::Logger::instance().init("logs/backtester.log", trading::LogLevel::DEBUG);
    } catch (const std::exception& e) {
        std::cerr << "[WARN] Could not open log file: " << e.what() << "\n";
    }

    LOG_INFO("Main", "QuantTradingSystem v0.5 starting");

    std::string dataPath   = (argc > 1) ? argv[1] : "../data/prices.csv";
    double      initialCash = 100000.0;

    try {
        // ── Build Backtester ──────────────────────────────
        trading::Backtester bt(dataPath, initialCash);

        // ── Add Strategies ────────────────────────────────
        bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(
            5, 20, trading::MAType::SMA));

        bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(
            5, 20, trading::MAType::EMA));

        bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(
            3, 10, trading::MAType::SMA));

        // ── Run all strategies ────────────────────────────
        bt.run();

        // ── Print results ─────────────────────────────────
        bt.printResults();
        bt.printSummary();

        std::cout << "\nReady for v0.6 - Performance Metrics\n\n";
        LOG_INFO("Main", "v0.5 complete - ready for v0.6 Metrics");

    } catch (const std::exception& e) {
        LOG_ERROR("Main", std::string("Fatal: ") + e.what());
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }

    LOG_INFO("Main", "Session complete");
    return 0;
}