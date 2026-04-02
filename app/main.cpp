// app/main.cpp
#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "core/MarketData.h"
#include "core/Strategy.h"
#include "strategy/MovingAverageStrategy.h"
#include "utils/Logger.h"


// ─────────────────────────────────────────────
//  formatTimestamp — Convert 16-digit string 
//  (Microseconds) to [YYYY-MM-DD HH:MM:SS]
// ─────────────────────────────────────────────
std::string formatTimestamp(const std::string& timestampStr) {
    if (timestampStr.empty()) return "0000-00-00 00:00:00";

    try {
        long long rawTs = std::stoll(timestampStr);

        // 16 digits = Microseconds (10^6), 13 digits = Milliseconds (10^3)
        long long seconds;
        if (timestampStr.length() >= 16) {
            seconds = rawTs / 1000000LL;
        } else {
            seconds = rawTs / 1000LL;
        }

        std::time_t t = static_cast<std::time_t>(seconds);
        std::tm* tm_ptr = std::gmtime(&t); 

        // Format into: YYYY-MM-DD HH:MM:SS
        std::ostringstream oss;
        oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M:%S");
        
        return oss.str();
    } catch (const std::exception& e) {
        // Return a placeholder if the string wasn't a valid number
        return "[INVALID TIMESTAMP]";
    }
}

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
                << std::setw(25) << formatTimestamp(s.date) 
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
        std::cerr << "[WARN] Continuing with terminal output only.\n\n";
    }

    LOG_INFO("Main", "QuantTradingSystem v0.3 starting");

    std::string dataPath = (argc > 1) ? argv[1] : "data/prices.csv";

    try {
        // ── Load Market Data ──────────────────────────────
        trading::MarketData data(dataPath);
        data.printSummary();

        // ── SMA Crossover Strategy (5/20) ────────────────
        trading::MovingAverageStrategy smaStrategy(5, 20, trading::MAType::SMA);
        auto smaSignals = smaStrategy.generateSignals(data);
        printSignals(smaSignals, "SMA Crossover (5/20)");

        // ── EMA Crossover Strategy (5/20) ────────────────
        trading::MovingAverageStrategy emaStrategy(5, 20, trading::MAType::EMA);
        auto emaSignals = emaStrategy.generateSignals(data);
        printSignals(emaSignals, "EMA Crossover (5/20)");

        std::cout << "\nReady for v0.4 - Portfolio Simulation\n\n";
        LOG_INFO("Main", "v0.3 complete - ready for v0.4 Portfolio");

    } catch (const std::exception& e) {
        LOG_ERROR("Main", std::string("Fatal: ") + e.what());
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }

    LOG_INFO("Main", "Session complete");
    return 0;
}