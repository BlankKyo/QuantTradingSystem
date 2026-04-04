// src/strategy/RSIStrategy.cpp
#include "strategy/RSIStrategy.h"
#include "utils/Logger.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <numeric>
#include <cmath>

namespace trading {

static const char* TAG = "RSIStrategy";

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
RSIStrategy::RSIStrategy(int    period,
                         double oversoldThreshold,
                         double overboughtThreshold)
    : period_(period)
    , oversoldThreshold_(oversoldThreshold)
    , overboughtThreshold_(overboughtThreshold)
{
    if (period_ <= 1)
        throw std::invalid_argument("[RSIStrategy] Period must be > 1");
    if (oversoldThreshold_ >= overboughtThreshold_)
        throw std::invalid_argument("[RSIStrategy] oversold must be < overbought");

    std::ostringstream oss;
    oss << "Initialized | Period=" << period_
        << " | Oversold="    << oversoldThreshold_
        << " | Overbought="  << overboughtThreshold_;
    LOG_INFO(TAG, oss.str());
}

const char* RSIStrategy::name() const { return "RSIStrategy"; }

// ─────────────────────────────────────────────
//  computeRSI()
//
//  Step 1: compute daily price changes
//    change[i] = close[i] - close[i-1]
//
//  Step 2: seed avgGain and avgLoss using
//    simple average of first `period` changes
//
//  Step 3: smooth using Wilder's method
//    avgGain = (prevAvgGain * (period-1) + gain) / period
//    avgLoss = (prevAvgLoss * (period-1) + loss) / period
//
//  Step 4: RSI = 100 - (100 / (1 + RS))
//    where RS = avgGain / avgLoss
//
//  Bars before period+1 are set to -1.0 (not yet valid).
// ─────────────────────────────────────────────
std::vector<double> RSIStrategy::computeRSI(const std::vector<double>& prices) const {
    std::vector<double> rsi(prices.size(), -1.0);

    if ((int)prices.size() <= period_) return rsi;

    // Step 1 — daily changes
    std::vector<double> gains, losses;
    for (size_t i = 1; i < prices.size(); ++i) {
        double change = prices[i] - prices[i-1];
        gains .push_back(change > 0 ?  change : 0.0);
        losses.push_back(change < 0 ? -change : 0.0);
    }

    // Step 2 — seed: simple average of first `period` changes
    double avgGain = 0.0, avgLoss = 0.0;
    for (int i = 0; i < period_; ++i) {
        avgGain += gains[i];
        avgLoss += losses[i];
    }
    avgGain /= period_;
    avgLoss /= period_;

    // First valid RSI at index period_ (prices index = period_)
    auto calcRSI = [](double g, double l) -> double {
        if (l < 1e-12) return 100.0; // no losses = max RSI
        double rs = g / l;
        return 100.0 - (100.0 / (1.0 + rs));
    };

    rsi[period_] = calcRSI(avgGain, avgLoss);

    // Step 3 — Wilder's smoothing for remaining bars
    for (size_t i = period_ + 1; i < prices.size(); ++i) {
        size_t gi = i - 1; // gains/losses are offset by 1
        avgGain = (avgGain * (period_ - 1) + gains[gi])  / period_;
        avgLoss = (avgLoss * (period_ - 1) + losses[gi]) / period_;
        rsi[i]  = calcRSI(avgGain, avgLoss);
    }

    return rsi;
}

// ─────────────────────────────────────────────
//  generateSignals()
//
//  BUY  when RSI crosses FROM below oversold TO above it
//    (oversold recovery — momentum turning up)
//  SELL when RSI crosses FROM above overbought TO below it
//    (overbought pullback — momentum turning down)
//
//  We use crossover (not level) to avoid holding signals
//  for too many bars.
// ─────────────────────────────────────────────
std::vector<TradeSignal> RSIStrategy::generateSignals(const MarketData& data) const {
    if ((int)data.size() <= period_) {
        LOG_WARNING(TAG, "Not enough bars. Need >" + std::to_string(period_)
            + ", got " + std::to_string(data.size()));
        return {};
    }

    auto closes = data.closePrices();
    auto rsi    = computeRSI(closes);

    std::vector<TradeSignal> signals(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        signals[i] = { Signal::HOLD, data[i].close, data[i].date, (int)i };
    }

    int buys = 0, sells = 0;

    for (size_t i = (size_t)period_ + 1; i < data.size(); ++i) {
        if (rsi[i-1] < 0 || rsi[i] < 0) continue; // not yet valid

        bool prevOversold   = rsi[i-1] <= oversoldThreshold_;
        bool currRecovered  = rsi[i]   >  oversoldThreshold_;
        bool prevOverbought = rsi[i-1] >= overboughtThreshold_;
        bool currPulledBack = rsi[i]   <  overboughtThreshold_;

        if (prevOversold && currRecovered) {
            signals[i].signal = Signal::BUY;
            ++buys;
            LOG_DEBUG(TAG, "BUY  on " + data[i].date
                + " RSI=" + std::to_string(rsi[i])
                + " price=" + std::to_string(data[i].close));

        } else if (prevOverbought && currPulledBack) {
            signals[i].signal = Signal::SELL;
            ++sells;
            LOG_DEBUG(TAG, "SELL on " + data[i].date
                + " RSI=" + std::to_string(rsi[i])
                + " price=" + std::to_string(data[i].close));
        }
    }

    std::ostringstream oss;
    oss << "Done | BUY=" << buys << " SELL=" << sells
        << " HOLD=" << (data.size() - buys - sells)
        << " Total=" << data.size();
    LOG_INFO(TAG, oss.str());

    return signals;
}

} // namespace trading