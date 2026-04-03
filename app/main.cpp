// app/main.cpp
#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "core/MarketData.h"
#include "core/Strategy.h"
#include "strategy/MovingAverageStrategy.h"
#include "portfolio/Portfolio.h"
#include "utils/Logger.h"


// ─────────────────────────────────────────────
//  printSignals — print signal table to terminal
// ─────────────────────────────────────────────
void printSignals(const std::vector<trading::TradeSignal>& signals,
                  const std::string& label)
{

    std::cout << "\n------------- " << label << " -------------\n";
    std::cout << std::left
            << std::setw(25) << "Date"      
            << std::setw(15) << "Price"     
            << std::setw(8)  << "Signal"
            << "\n";
    std::cout << std::string(48, '-') << "\n"; 

    for (const auto& s : signals) {
        if (s.signal == trading::Signal::HOLD) continue; 

        std::cout << std::left
                << std::setw(25) << s.date
                << std::setw(15) << std::fixed << std::setprecision(2) << s.price
                << std::setw(8)  << (s.signal == trading::Signal::BUY ? "BUY" : "SELL")
                << "\n";
    }
}

int main(int argc, char* argv[]) {
    // ── Init Logger ───────────────────────────────────────
    try {
        trading::Logger::instance().init("logs/backtester.log", trading::LogLevel::DEBUG);
    } catch (const std::exception& e) {
        std::cerr << "[WARN] Could not open log file: " << e.what() << "\n";
    }

    LOG_INFO("Main", "QuantTradingSystem v0.4 starting");

    std::string dataPath = (argc > 1) ? argv[1] : "data/prices.csv";

    try {
        // ── Load Market Data ──────────────────────────────
        trading::MarketData data(dataPath);
        data.printSummary();

        // ── SMA Strategy + Portfolio ──────────────────────
        std::cout << "========================================\n";
        std::cout << "  SMA Crossover (5/20)\n";
        std::cout << "========================================\n";

        trading::MovingAverageStrategy smaStrategy(5, 20, trading::MAType::SMA);
        auto smaSignals = smaStrategy.generateSignals(data);

        trading::Portfolio smaPortfolio(100000.0);
        smaPortfolio.run(smaSignals, data);
        smaPortfolio.printSummary();
        smaPortfolio.printTrades();
        smaPortfolio.printEquityCurve();

        // ── EMA Strategy + Portfolio ──────────────────────
        std::cout << "\n========================================\n";
        std::cout << "  EMA Crossover (5/20)\n";
        std::cout << "========================================\n";

        trading::MovingAverageStrategy emaStrategy(5, 20, trading::MAType::EMA);
        auto emaSignals = emaStrategy.generateSignals(data);

        trading::Portfolio emaPortfolio(100000.0);
        emaPortfolio.run(emaSignals, data);
        emaPortfolio.printSummary();
        emaPortfolio.printTrades();
        emaPortfolio.printEquityCurve();

        std::cout << "\nReady for v0.5 - Backtester Engine\n\n";
        LOG_INFO("Main", "v0.4 complete - ready for v0.5 Backtester");

    } catch (const std::exception& e) {
        LOG_ERROR("Main", std::string("Fatal: ") + e.what());
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }

    LOG_INFO("Main", "Session complete");
    return 0;
}
