// include/core/MarketData.h
#pragma once

#include <string>
#include <vector>
#include <stdexcept>

namespace trading {

// ─────────────────────────────────────────────
//  OHLCV Bar — single price record
// ─────────────────────────────────────────────
struct Bar {
    std::string date;
    double open   = 0.0;
    double high   = 0.0;
    double low    = 0.0;
    double close  = 0.0;
    long   volume = 0;
};

// ─────────────────────────────────────────────
//  MarketData — CSV loader & price series access
// ─────────────────────────────────────────────
class MarketData {
public:
    // Load bars from a CSV file (throws on error)
    explicit MarketData(const std::string& filepath);

    // Accessors
    const std::vector<Bar>& bars()  const { return bars_; }
    size_t                  size()  const { return bars_.size(); }
    bool                    empty() const { return bars_.empty(); }

    const Bar& operator[](size_t i) const { return bars_.at(i); }

    // Convenience: extract a single price series
    std::vector<double> closePrices() const;
    std::vector<double> openPrices()  const;
    std::vector<double> volumes()     const;

    // Print summary to stdout
    void printSummary() const;

private:
    std::vector<Bar> bars_;
    std::string      filepath_;

    void        load(const std::string& filepath);
    static Bar  parseRow(const std::string& line, int lineNumber);
    static void validateHeader(const std::string& header);
};

} // namespace trading