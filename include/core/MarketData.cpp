// MarketData.cpp
#include "MarketData.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool MarketData::loadCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    data.clear();
    std::string line;
    
    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        OHLCV row;

        std::getline(ss, row.date, ',');
        std::getline(ss, token, ','); row.open = std::stod(token);
        std::getline(ss, token, ','); row.high = std::stod(token);
        std::getline(ss, token, ','); row.low = std::stod(token);
        std::getline(ss, token, ','); row.close = std::stod(token);
        std::getline(ss, token, ','); row.volume = std::stod(token);

        data.push_back(row);
    }
    return true;
}

size_t MarketData::size() const { return data.size(); }

const OHLCV& MarketData::operator[](size_t index) const { return data[index]; }