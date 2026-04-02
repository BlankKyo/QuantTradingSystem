// src/strategy/MovingAverageStrategy.cpp
#include "strategy/MovingAverageStrategy.h"
#include "utils/Logger.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace trading {

static const char* TAG = "MovingAverageStrategy";

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
MovingAverageStrategy::MovingAverageStrategy(int shortWindow, int longWindow, MAType maType)
    : shortWindow_(shortWindow)
    , longWindow_ (longWindow)
    , maType_     (maType)
{
    if (shortWindow_ <= 0 || longWindow_ <= 0)
        throw std::invalid_argument("[MovingAverageStrategy] Windows must be positive");

    if (shortWindow_ >= longWindow_)
        throw std::invalid_argument("[MovingAverageStrategy] shortWindow must be < longWindow");

    std::ostringstream oss;
    oss << "Initialized | Type=" << maTypeToString(maType_)
        << " | Short=" << shortWindow_
        << " | Long="  << longWindow_;
    LOG_INFO(TAG, oss.str());
}

// ─────────────────────────────────────────────
//  name()
// ─────────────────────────────────────────────
const char* MovingAverageStrategy::name() const {
    return "MovingAverageStrategy";
}

// ─────────────────────────────────────────────
//  computeSMA — Simple Moving Average
//
//  For each bar i, average the last `window` closes.
//  Bars before the window is full are set to 0.0 (not yet valid).
//
//  Example: window=3, prices=[1,2,3,4,5]
//    index 0 -> 0.0  (not enough data)
//    index 1 -> 0.0  (not enough data)
//    index 2 -> 2.0  (avg of 1,2,3)
//    index 3 -> 3.0  (avg of 2,3,4)
//    index 4 -> 4.0  (avg of 3,4,5)
// ─────────────────────────────────────────────
std::vector<double> MovingAverageStrategy::computeSMA(
    const std::vector<double>& prices, int window) const
{
    std::vector<double> sma(prices.size(), 0.0);

    for (int i = window - 1; i < (int)prices.size(); ++i) {
        double sum = 0.0;
        for (int j = i - window + 1; j <= i; ++j)
            sum += prices[j];
        sma[i] = sum / window;
    }

    return sma;
}

// ─────────────────────────────────────────────
//  computeEMA — Exponential Moving Average
//
//  Gives more weight to recent prices.
//  Multiplier k = 2 / (window + 1)
//  EMA[i] = price[i] * k + EMA[i-1] * (1 - k)
//
//  Seeds the EMA with the first SMA at index (window-1).
//  Bars before that are set to 0.0.
// ─────────────────────────────────────────────
std::vector<double> MovingAverageStrategy::computeEMA(
    const std::vector<double>& prices, int window) const
{
    std::vector<double> ema(prices.size(), 0.0);

    if ((int)prices.size() < window) return ema;

    // Seed with first SMA
    double seed = 0.0;
    for (int i = 0; i < window; ++i) seed += prices[i];
    ema[window - 1] = seed / window;

    // Multiplier
    double k = 2.0 / (window + 1.0);

    // Roll forward
    for (int i = window; i < (int)prices.size(); ++i)
        ema[i] = prices[i] * k + ema[i - 1] * (1.0 - k);

    return ema;
}

// ─────────────────────────────────────────────
//  computeMA — dispatches to SMA or EMA
// ─────────────────────────────────────────────
std::vector<double> MovingAverageStrategy::computeMA(
    const std::vector<double>& prices, int window) const
{
    return (maType_ == MAType::SMA)
        ? computeSMA(prices, window)
        : computeEMA(prices, window);
}

// ─────────────────────────────────────────────
//  generateSignals — crossover logic
//
//  For each bar where both MAs are valid:
//    prevShortBelow + currShortAbove -> BUY  (golden cross)
//    prevShortAbove + currShortBelow -> SELL (death cross)
//    otherwise                       -> HOLD
// ─────────────────────────────────────────────
std::vector<TradeSignal> MovingAverageStrategy::generateSignals(
    const MarketData& data) const
{
    if (data.size() < (size_t)longWindow_) {
        LOG_WARNING(TAG, "Not enough bars. Need " + std::to_string(longWindow_)
            + ", got " + std::to_string(data.size()));
        return {};
    }

    auto closes  = data.closePrices();
    auto shortMA = computeMA(closes, shortWindow_);
    auto longMA  = computeMA(closes, longWindow_);

    std::vector<TradeSignal> signals(data.size());

    // Default all bars to HOLD
    for (size_t i = 0; i < data.size(); ++i) {
        signals[i].signal   = Signal::HOLD;
        signals[i].price    = data[i].close;
        signals[i].date     = data[i].date;
        signals[i].barIndex = (int)i;
    }

    int buys = 0, sells = 0;

    for (size_t i = (size_t)longWindow_; i < data.size(); ++i) {
        bool prevShortBelow = shortMA[i-1] <= longMA[i-1];
        bool currShortAbove = shortMA[i]   >  longMA[i];
        bool prevShortAbove = shortMA[i-1] >= longMA[i-1];
        bool currShortBelow = shortMA[i]   <  longMA[i];

        if (prevShortBelow && currShortAbove) {
            signals[i].signal = Signal::BUY;
            ++buys;
            LOG_DEBUG(TAG, "BUY  on " + data[i].date
                + " price=" + std::to_string(data[i].close)
                + " shortMA=" + std::to_string(shortMA[i])
                + " longMA="  + std::to_string(longMA[i]));

        } else if (prevShortAbove && currShortBelow) {
            signals[i].signal = Signal::SELL;
            ++sells;
            LOG_DEBUG(TAG, "SELL on " + data[i].date
                + " price=" + std::to_string(data[i].close)
                + " shortMA=" + std::to_string(shortMA[i])
                + " longMA="  + std::to_string(longMA[i]));
        }
    }

    std::ostringstream summary;
    summary << "Done | BUY=" << buys
            << " SELL=" << sells
            << " HOLD=" << (data.size() - buys - sells)
            << " Total=" << data.size();
    LOG_INFO(TAG, summary.str());

    return signals;
}

// ─────────────────────────────────────────────
//  maTypeToString
// ─────────────────────────────────────────────
std::string MovingAverageStrategy::maTypeToString(MAType t) {
    return (t == MAType::SMA) ? "SMA" : "EMA";
}

} // namespace trading