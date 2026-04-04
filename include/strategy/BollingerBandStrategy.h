// include/strategy/BollingerBandStrategy.h
#pragma once

#include "core/Strategy.h"
#include "core/MarketData.h"

#include <vector>

namespace trading {

// ─────────────────────────────────────────────
//  BollingerBandStrategy
//
//  Bollinger Bands = SMA ± (k * stdDev)
//    Upper band = SMA + k * stdDev
//    Middle     = SMA
//    Lower band = SMA - k * stdDev
//
//  Price touches lower band → BUY  (statistically cheap)
//  Price touches upper band → SELL (statistically expensive)
//
//  Classic params: period=20, k=2.0
// ─────────────────────────────────────────────

struct BollingerBands {
    std::vector<double> upper;
    std::vector<double> middle;  // SMA
    std::vector<double> lower;
};

class BollingerBandStrategy : public Strategy {
public:
    BollingerBandStrategy(int    period = 20,
                          double k      = 2.0);

    std::vector<TradeSignal> generateSignals(const MarketData& data) const override;
    const char*              name()          const override;

    // Public for external access (charting, v0.8 optimization)
    BollingerBands computeBands(const std::vector<double>& prices) const;

    int    period() const { return period_; }
    double k()      const { return k_;      }

private:
    int    period_;
    double k_;
};

} // namespace trading