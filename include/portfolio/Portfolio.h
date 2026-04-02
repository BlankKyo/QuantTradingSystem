// include/portfolio/Portfolio.h
#pragma once

#include "core/Strategy.h"
#include "core/MarketData.h"

#include <vector>
#include <string>

namespace trading {

// ─────────────────────────────────────────────
//  Trade — a single completed round-trip trade
//  (BUY entry + SELL exit)
// ─────────────────────────────────────────────
struct Trade {
    std::string entryDate  = "";
    std::string exitDate   = "";
    double      entryPrice = 0.0;
    double      exitPrice  = 0.0;
    int         shares     = 0;
    double      pnl        = 0.0;    // profit or loss in $
    double      pnlPct     = 0.0;    // profit or loss in %
};

// ─────────────────────────────────────────────
//  EquityPoint — portfolio value at one bar
// ─────────────────────────────────────────────
struct EquityPoint {
    std::string date        = "";
    double      cash        = 0.0;   // cash not invested
    double      positionVal = 0.0;   // value of open position
    double      totalEquity = 0.0;   // cash + positionVal
};

// ─────────────────────────────────────────────
//  Portfolio — simulates trade execution
//
//  Usage:
//    Portfolio p(100000.0);
//    p.run(signals, data);
//    p.printSummary();
// ─────────────────────────────────────────────
class Portfolio {
public:
    explicit Portfolio(double initialCash = 100000.0);

    // Run simulation — processes all signals against market data
    void run(const std::vector<TradeSignal>& signals,
             const MarketData&               data);

    // Results accessors
    const std::vector<Trade>&       trades()      const { return trades_;      }
    const std::vector<EquityPoint>& equityCurve() const { return equityCurve_; }

    double initialCash()  const { return initialCash_;  }
    double finalEquity()  const;
    double totalReturn()  const;   // % return overall
    double totalPnL()     const;   // $ profit or loss

    bool   hasOpenPosition() const { return shares_ > 0; }

    // Print full summary to terminal + log
    void printSummary()    const;
    void printTrades()     const;
    void printEquityCurve() const;

private:
    // ── Config ────────────────────────────────
    double initialCash_;

    // ── State during simulation ───────────────
    double      cash_       = 0.0;
    int         shares_     = 0;
    double      entryPrice_ = 0.0;
    std::string entryDate_  = "";

    // ── Results ───────────────────────────────
    std::vector<Trade>       trades_;
    std::vector<EquityPoint> equityCurve_;

    // ── Internal execution ────────────────────
    void executeBuy (const TradeSignal& signal);
    void executeSell(const TradeSignal& signal);
    void recordEquity(const std::string& date, double currentPrice);
};

} // namespace trading