// include/core/MarketData.h
#pragma once
#include <string>
#include <vector>

struct OHLCV {
    std::string date;
    double open;
    double high;
    double low;
    double close;
    double volume;
    OHLCV() = default;
    OHLCV(const std::string& d, double o, double h, double l, double c, double v)
        : date(d), open(o), high(h), low(l), close(c), volume(v) {}
};

class MarketData {
private:
    std::vector<OHLCV> data;

public:
    // Load CSV file
    bool loadCSV(const std::string& filename);

    // Number of rows
    size_t size() const;

    // Access row
    const OHLCV& operator[](size_t index) const;
};