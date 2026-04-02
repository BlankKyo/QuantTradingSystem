// include/core/Strategy.h
#pragma once

#include <vector>
#include <string>
#include "core/MarketData.h"

namespace trading {

// ─────────────────────────────────────────────
//  Signal — what the strategy tells us to do
// ─────────────────────────────────────────────
enum class Signal { HOLD = 0, BUY = 1, SELL = -1 };

// ─────────────────────────────────────────────
//  TradeSignal — signal + price context
//  This is what v0.4 Portfolio will consume
// ─────────────────────────────────────────────
struct TradeSignal {
    Signal      signal     = Signal::HOLD;
    double      price      = 0.0;   // price at signal bar (close)
    std::string date       = "";    // date of signal
    int         barIndex   = -1;    // index in MarketData
};

// ─────────────────────────────────────────────
//  Strategy — abstract base class
//  All strategies must implement these two methods
// ─────────────────────────────────────────────
class Strategy {
public:
    virtual ~Strategy() = default;

    // Returns one TradeSignal per bar
    virtual std::vector<TradeSignal> generateSignals(const MarketData& data) const = 0;

    // Human-readable name for logging and reporting
    virtual const char* name() const = 0;
};

} // namespace trading