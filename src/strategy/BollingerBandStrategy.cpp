// src/strategy/BollingerBandStrategy.cpp
#include "strategy/BollingerBandStrategy.h"
#include "utils/Logger.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <numeric>

namespace trading {

static const char* TAG = "BollingerBandStrategy";

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
BollingerBandStrategy::BollingerBandStrategy(int period, double k)
    : period_(period)
    , k_(k)
{
    if (period_ < 2)
        throw std::invalid_argument("[BollingerBandStrategy] Period must be >= 2");
    if (k_ <= 0.0)
        throw std::invalid_argument("[BollingerBandStrategy] k must be > 0");

    std::ostringstream oss;
    oss << "Initialized | Period=" << period_ << " | k=" << k_;
    LOG_INFO(TAG, oss.str());
}

const char* BollingerBandStrategy::name() const { return "BollingerBandStrategy"; }

// ─────────────────────────────────────────────
//  computeBands()
//
//  For each bar i >= period-1:
//    middle[i] = SMA of last `period` closes
//    stdDev[i] = population std dev of same window
//    upper[i]  = middle[i] + k * stdDev[i]
//    lower[i]  = middle[i] - k * stdDev[i]
//
//  Bars before period-1 are set to 0.0 (invalid).
// ─────────────────────────────────────────────
BollingerBands BollingerBandStrategy::computeBands(
    const std::vector<double>& prices) const
{
    BollingerBands bands;
    size_t n = prices.size();
    bands.upper .assign(n, 0.0);
    bands.middle.assign(n, 0.0);
    bands.lower .assign(n, 0.0);

    for (int i = period_ - 1; i < (int)n; ++i) {
        // Window slice
        double sum = 0.0;
        for (int j = i - period_ + 1; j <= i; ++j) sum += prices[j];
        double mean = sum / period_;

        // Population std dev
        double variance = 0.0;
        for (int j = i - period_ + 1; j <= i; ++j)
            variance += (prices[j] - mean) * (prices[j] - mean);
        double stdDev = std::sqrt(variance / period_);

        bands.middle[i] = mean;
        bands.upper [i] = mean + k_ * stdDev;
        bands.lower [i] = mean - k_ * stdDev;
    }

    return bands;
}

// ─────────────────────────────────────────────
//  generateSignals()
//
//  BUY  when price crosses FROM above lower band
//       TO below it (touches/breaches lower band)
//  SELL when price crosses FROM below upper band
//       TO above it (touches/breaches upper band)
//
//  We use crossover direction to get clean signals
//  rather than firing every bar price is outside band.
// ─────────────────────────────────────────────
std::vector<TradeSignal> BollingerBandStrategy::generateSignals(
    const MarketData& data) const
{
    if ((int)data.size() < period_) {
        LOG_WARNING(TAG, "Not enough bars. Need " + std::to_string(period_)
            + ", got " + std::to_string(data.size()));
        return {};
    }

    auto closes = data.closePrices();
    auto bands  = computeBands(closes);

    std::vector<TradeSignal> signals(data.size());
    for (size_t i = 0; i < data.size(); ++i)
        signals[i] = { Signal::HOLD, data[i].close, data[i].date, (int)i };

    int buys = 0, sells = 0;

    for (size_t i = (size_t)period_; i < data.size(); ++i) {
        if (bands.lower[i] == 0.0) continue; // not yet valid

        bool prevAboveLower = closes[i-1] >= bands.lower[i-1];
        bool currBelowLower = closes[i]   <  bands.lower[i];
        bool prevBelowUpper = closes[i-1] <= bands.upper[i-1];
        bool currAboveUpper = closes[i]   >  bands.upper[i];

        if (prevAboveLower && currBelowLower) {
            signals[i].signal = Signal::BUY;
            ++buys;
            LOG_DEBUG(TAG, "BUY  on " + data[i].date
                + " price=" + std::to_string(closes[i])
                + " lower=" + std::to_string(bands.lower[i]));

        } else if (prevBelowUpper && currAboveUpper) {
            signals[i].signal = Signal::SELL;
            ++sells;
            LOG_DEBUG(TAG, "SELL on " + data[i].date
                + " price=" + std::to_string(closes[i])
                + " upper=" + std::to_string(bands.upper[i]));
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