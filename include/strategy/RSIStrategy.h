// include/strategy/RSIStrategy.h
#pragma once

#include "core/Strategy.h"
#include "core/MarketData.h"

#include <vector>

namespace trading {

// ─────────────────────────────────────────────
//  RSIStrategy — Relative Strength Index
//
//  RSI measures momentum: how fast prices are
//  rising vs falling over a rolling window.
//
//  RSI < oversoldThreshold  → BUY  (price fell too fast, expect bounce)
//  RSI > overboughtThreshold → SELL (price rose too fast, expect drop)
//  Otherwise                → HOLD
//
//  Classic thresholds: oversold=30, overbought=70
//  Classic window: 14 bars
// ─────────────────────────────────────────────
class RSIStrategy : public Strategy {
public:
    RSIStrategy(int    period             = 14,
                double oversoldThreshold  = 30.0,
                double overboughtThreshold = 70.0);

    std::vector<TradeSignal> generateSignals(const MarketData& data) const override;
    const char*              name()          const override;

    // Public so Backtester/Metrics can access the RSI series
    std::vector<double> computeRSI(const std::vector<double>& prices) const;

    int    period()              const { return period_;              }
    double oversoldThreshold()   const { return oversoldThreshold_;   }
    double overboughtThreshold() const { return overboughtThreshold_; }

private:
    int    period_;
    double oversoldThreshold_;
    double overboughtThreshold_;
};

} // namespace trading