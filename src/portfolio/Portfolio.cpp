// src/portfolio/Portfolio.cpp
#include "portfolio/Portfolio.h"
#include "utils/Logger.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <cmath>

namespace trading {

static const char* TAG = "Portfolio";

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Portfolio::Portfolio(double initialCash)
    : initialCash_(initialCash)
    , cash_(initialCash)
    , shares_(0)
    , entryPrice_(0.0)
{
    if (initialCash_ <= 0.0)
        throw std::invalid_argument("[Portfolio] Initial cash must be positive");

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2)
        << "Initialized | Cash=$" << initialCash_;
    LOG_INFO(TAG, oss.str());
}

// ─────────────────────────────────────────────
//  run() — main simulation loop
//
//  For each bar:
//    1. Check signal
//    2. Execute BUY or SELL if signalled
//    3. Record equity snapshot
// ─────────────────────────────────────────────
void Portfolio::run(const std::vector<TradeSignal>& signals,
                    const MarketData&               data)
{
    if (signals.size() != data.size()) {
        throw std::runtime_error(
            "[Portfolio] signals.size() != data.size(). "
            "Make sure signals come from the same MarketData."
        );
    }

    trades_.clear();
    equityCurve_.clear();
    cash_       = initialCash_;
    shares_     = 0;
    entryPrice_ = 0.0;
    entryDate_  = "";

    LOG_INFO(TAG, "Starting simulation over " + std::to_string(data.size()) + " bars");

    for (size_t i = 0; i < signals.size(); ++i) {
        const auto& sig = signals[i];
        const auto& bar = data[i];

        // Execute trades
        if (sig.signal == Signal::BUY && shares_ == 0) {
            executeBuy(sig);
        } else if (sig.signal == Signal::SELL && shares_ > 0) {
            executeSell(sig);
        }

        // Record equity at this bar (mark-to-market)
        recordEquity(bar.date, bar.close);
    }

    // If we end with an open position, log a warning
    if (shares_ > 0) {
        LOG_WARNING(TAG, "Simulation ended with open position: "
            + std::to_string(shares_) + " shares at entry $"
            + std::to_string(entryPrice_));
    }

    std::ostringstream summary;
    summary << std::fixed << std::setprecision(2)
            << "Simulation complete"
            << " | Trades=" << trades_.size()
            << " | FinalEquity=$" << finalEquity()
            << " | Return=" << totalReturn() << "%";
    LOG_INFO(TAG, summary.str());
}

// ─────────────────────────────────────────────
//  executeBuy()
//
//  Spend all available cash.
//  shares = floor(cash / price)
//  Leftover cash stays as cash (fractional shares not supported)
// ─────────────────────────────────────────────
void Portfolio::executeBuy(const TradeSignal& signal) {
    int affordableShares = static_cast<int>(std::floor(cash_ / signal.price));

    if (affordableShares <= 0) {
        LOG_WARNING(TAG, "BUY signal on " + signal.date + " but insufficient cash ($"
            + std::to_string(cash_) + ") to buy at $" + std::to_string(signal.price));
        return;
    }

    shares_     = affordableShares;
    entryPrice_ = signal.price;
    entryDate_  = signal.date;
    cash_      -= shares_ * signal.price;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2)
        << "BUY  | Date=" << signal.date
        << " | Shares=" << shares_
        << " | Price=$" << signal.price
        << " | Cost=$"  << (shares_ * signal.price)
        << " | CashLeft=$" << cash_;
    LOG_INFO(TAG, oss.str());
}

// ─────────────────────────────────────────────
//  executeSell()
//
//  Close entire position.
//  Record the completed trade with PnL.
// ─────────────────────────────────────────────
void Portfolio::executeSell(const TradeSignal& signal) {
    double proceeds = shares_ * signal.price;
    double cost     = shares_ * entryPrice_;
    double pnl      = proceeds - cost;
    double pnlPct   = (pnl / cost) * 100.0;

    // Record completed trade
    Trade trade;
    trade.entryDate  = entryDate_;
    trade.exitDate   = signal.date;
    trade.entryPrice = entryPrice_;
    trade.exitPrice  = signal.price;
    trade.shares     = shares_;
    trade.pnl        = pnl;
    trade.pnlPct     = pnlPct;
    trades_.push_back(trade);

    cash_  += proceeds;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2)
        << "SELL | Date=" << signal.date
        << " | Shares=" << shares_
        << " | Price=$" << signal.price
        << " | PnL=$"   << pnl
        << " (" << pnlPct << "%)"
        << " | Cash=$"  << cash_;
    LOG_INFO(TAG, oss.str());

    // Reset position
    shares_     = 0;
    entryPrice_ = 0.0;
    entryDate_  = "";
}

// ─────────────────────────────────────────────
//  recordEquity()
//
//  Mark-to-market: value open position at current close price.
//  Store as EquityPoint for the equity curve.
// ─────────────────────────────────────────────
void Portfolio::recordEquity(const std::string& date, double currentPrice) {
    EquityPoint ep;
    ep.date        = date;
    ep.cash        = cash_;
    ep.positionVal = shares_ * currentPrice;
    ep.totalEquity = ep.cash + ep.positionVal;
    equityCurve_.push_back(ep);
}

// ─────────────────────────────────────────────
//  Accessors
// ─────────────────────────────────────────────
double Portfolio::finalEquity() const {
    return equityCurve_.empty() ? cash_ : equityCurve_.back().totalEquity;
}

double Portfolio::totalReturn() const {
    return ((finalEquity() - initialCash_) / initialCash_) * 100.0;
}

double Portfolio::totalPnL() const {
    return finalEquity() - initialCash_;
}

// ─────────────────────────────────────────────
//  printSummary()
// ─────────────────────────────────────────────
void Portfolio::printSummary() const {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n======================================\n";
    std::cout << "  Portfolio Summary\n";
    std::cout << "======================================\n";
    std::cout << "  Initial Cash  : $" << initialCash_  << "\n";
    std::cout << "  Final Equity  : $" << finalEquity() << "\n";
    std::cout << "  Total PnL     : $" << totalPnL()    << "\n";
    std::cout << "  Total Return  : "  << totalReturn() << "%\n";
    std::cout << "  Total Trades  : "  << trades_.size() << "\n";

    if (!trades_.empty()) {
        int wins = 0;
        for (const auto& t : trades_) if (t.pnl > 0) ++wins;
        double winRate = (double)wins / trades_.size() * 100.0;
        std::cout << "  Win Rate      : " << winRate << "%"
                  << " (" << wins << "/" << trades_.size() << ")\n";
    }

    std::cout << "======================================\n\n";

    LOG_INFO(TAG, "Summary | InitialCash=$" + std::to_string(initialCash_)
        + " FinalEquity=$" + std::to_string(finalEquity())
        + " Return=" + std::to_string(totalReturn()) + "%"
        + " Trades=" + std::to_string(trades_.size()));
}

// ─────────────────────────────────────────────
//  printTrades()
// ─────────────────────────────────────────────
void Portfolio::printTrades() const {
    if (trades_.empty()) {
        std::cout << "  No completed trades.\n";
        return;
    }

    std::cout << "\n--- Trade History ---\n";
    std::cout << std::left
              << std::setw(12) << "Entry"
              << std::setw(12) << "Exit"
              << std::setw(8)  << "Shares"
              << std::setw(10) << "EntryPx"
              << std::setw(10) << "ExitPx"
              << std::setw(12) << "PnL($)"
              << std::setw(10) << "PnL(%)"
              << "\n";
    std::cout << std::string(74, '-') << "\n";

    for (const auto& t : trades_) {
        std::cout << std::fixed << std::setprecision(2)
                  << std::left
                  << std::setw(12) << t.entryDate
                  << std::setw(12) << t.exitDate
                  << std::setw(8)  << t.shares
                  << std::setw(10) << t.entryPrice
                  << std::setw(10) << t.exitPrice
                  << std::setw(12) << t.pnl
                  << std::setw(10) << t.pnlPct
                  << (t.pnl >= 0 ? "  WIN" : "  LOSS")
                  << "\n";
    }
}

// ─────────────────────────────────────────────
//  printEquityCurve()
// ─────────────────────────────────────────────
void Portfolio::printEquityCurve() const {
    std::cout << "\n--- Equity Curve ---\n";
    std::cout << std::left
              << std::setw(22) << "Date"
              << std::setw(14) << "Cash"
              << std::setw(14) << "Position"
              << std::setw(14) << "TotalEquity"
              << "\n";
    std::cout << std::string(64, '-') << "\n";

    std::cout << std::fixed << std::setprecision(2);
    for (const auto& ep : equityCurve_) {
        std::cout << std::left
                  << std::setw(22) << ep.date
                  << std::setw(14) << ep.cash
                  << std::setw(14) << ep.positionVal
                  << std::setw(14) << ep.totalEquity
                  << "\n";
    }
}

} // namespace trading