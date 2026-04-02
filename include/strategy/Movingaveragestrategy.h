// include/strategy/MovingAverageStrategy.h
#pragma once

#include "core/Strategy.h"
#include "core/MarketData.h"

#include <vector>
#include <string>

namespace trading {

// ─────────────────────────────────────────────
//  MA Type — which algorithm to use
// ─────────────────────────────────────────────
enum class MAType { SMA, EMA };

// ─────────────────────────────────────────────
//  MovingAverageStrategy
//
//  Crossover logic:
//    short MA crosses ABOVE long MA → BUY
//    short MA crosses BELOW long MA → SELL
//    otherwise                      → HOLD
//
//  Supports both SMA and EMA
// ─────────────────────────────────────────────
class MovingAverageStrategy : public Strategy {
public:
    // Constructor — configure windows and MA type
    MovingAverageStrategy(int    shortWindow = 5,
                          int    longWindow  = 20,
                          MAType maType      = MAType::SMA);

    // Strategy interface
    std::vector<TradeSignal> generateSignals(const MarketData& data) const override;
    const char*              name()          const override;

    // Getters — useful for logging and optimization (v0.8)
    int    shortWindow() const { return shortWindow_; }
    int    longWindow()  const { return longWindow_;  }
    MAType maType()      const { return maType_;      }

    // Compute MA series directly — public so Backtester can access them
    std::vector<double> computeSMA(const std::vector<double>& prices, int window) const;
    std::vector<double> computeEMA(const std::vector<double>& prices, int window) const;

private:
    int    shortWindow_;
    int    longWindow_;
    MAType maType_;

    // Internal helpers
    std::vector<double> computeMA(const std::vector<double>& prices, int window) const;
    static std::string  maTypeToString(MAType t);
};

} // namespace trading