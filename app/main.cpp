#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "core/MarketData.h"
#include "utils/Logger.h"

int main(int argc, char* argv[]) {
    // ── Init Logger ───────────────────────────────────────
    // Writes to both terminal and logs/backtester.log
    try {
        trading::Logger::instance().init("logs/backtester.log", trading::LogLevel::DEBUG);
    } catch (const std::exception& e) {
        std::cerr << "[WARN] Could not open log file: " << e.what() << "\n";
        std::cerr << "[WARN] Continuing with terminal output only.\n\n";
    }

    LOG_INFO("Main", "TradingBacktester v0.2 starting");

    // ── Load Market Data ──────────────────────────────────
    std::string dataPath = (argc > 1) ? argv[1] : "data/prices.csv";

    try {
        trading::MarketData data(dataPath);
        data.printSummary();

        // Extract close prices (ready for v0.3 strategy)
        auto closes = data.closePrices();
        LOG_INFO("Main", "Close prices extracted: " + std::to_string(closes.size()) + " values");
        LOG_INFO("Main", "Ready for v0.3 - Moving Average Strategy");

        std::cout << "Close prices extracted: " << closes.size() << " values  OK\n";
        std::cout << "Ready for v0.3 - Moving Average Strategy\n\n";

    } catch (const std::exception& e) {
        LOG_ERROR("Main", std::string("Fatal error: ") + e.what());
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }

    LOG_INFO("Main", "Session complete");
    return 0;
}