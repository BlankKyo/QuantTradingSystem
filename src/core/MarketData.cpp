// src/core/MarketData.cpp
#include "core/MarketData.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace trading {

// ─────────────────────────────────────────────
//  formatTimestamp — Convert 16-digit string 
//  (Microseconds) to [YYYY-MM-DD HH:MM:SS]
// ─────────────────────────────────────────────
std::string formatTimestamp(const std::string& timestampStr) {
    if (timestampStr.empty()) return "0000-00-00 00:00:00";

    try {
        long long rawTs = std::stoll(timestampStr);

        // 16 digits = Microseconds (10^6), 13 digits = Milliseconds (10^3)
        long long seconds;
        if (timestampStr.length() >= 16) {
            seconds = rawTs / 1000000LL;
        } else {
            seconds = rawTs / 1000LL;
        }

        std::time_t t = static_cast<std::time_t>(seconds);
        std::tm* tm_ptr = std::gmtime(&t); 

        // Format into: YYYY-MM-DD HH:MM:SS
        std::ostringstream oss;
        oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M:%S");
        
        return oss.str();
    } catch (const std::exception& e) {
        // Return a placeholder if the string wasn't a valid number
        return "INVALID TIMESTAMP";
    }
}

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
MarketData::MarketData(const std::string& filepath)
    : filepath_(filepath)
{
    load(filepath);
}

// ─────────────────────────────────────────────
//  load() — open file, validate header, parse rows
// ─────────────────────────────────────────────
void MarketData::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error(
            "[MarketData] Cannot open file: " + filepath
        );
    }

    std::string line;
    int lineNumber = 0;

    // --- Header ---
    if (!std::getline(file, line)) {
        throw std::runtime_error("[MarketData] File is empty: " + filepath);
    }
    // validateHeader(line);
    ++lineNumber;

    // --- Data rows ---
    bars_.reserve(512); // pre-allocate for typical daily data
    while (std::getline(file, line)) {
        ++lineNumber;
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos)
            continue; // skip blank lines

        bars_.push_back(parseRow(line, lineNumber));
    }

    if (bars_.empty()) {
        throw std::runtime_error("[MarketData] No data rows found in: " + filepath);
    }
}

// ─────────────────────────────────────────────
//  validateHeader() — expects: date,open,high,low,close,volume
// ─────────────────────────────────────────────
void MarketData::validateHeader(const std::string& header) {
    // Normalize: lowercase, strip \r
    std::string h = header;
    std::transform(h.begin(), h.end(), h.begin(), ::tolower);
    h.erase(std::remove(h.begin(), h.end(), '\r'), h.end());

    const std::string expected = "date,open,high,low,close,volume";
    if (h != expected) {
        throw std::runtime_error(
            "[MarketData] Unexpected CSV header.\n"
            "  Expected: " + expected + "\n"
            "  Got:      " + h
        );
    }
}

// ─────────────────────────────────────────────
//  parseRow() — tokenize one CSV line into a Bar
// ─────────────────────────────────────────────
Bar MarketData::parseRow(const std::string& line, int lineNumber) {
    std::istringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;
    tokens.reserve(6);

    while (std::getline(ss, token, ',')) {
        // Strip carriage return (Windows CSVs)
        token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
        tokens.push_back(token);
        if (tokens.size() == 6){
            // ignore extra columns
            break; 
        }
    }

    if (tokens.size() != 6) {
        throw std::runtime_error(
            "[MarketData] Line " + std::to_string(lineNumber) +
            ": expected 6 columns, got " + std::to_string(tokens.size()) +
            " → \"" + line + "\""
        );
    }

    try {
        Bar bar;
        bar.date   = formatTimestamp(tokens[0]);
        bar.open   = std::stod(tokens[1]);
        bar.high   = std::stod(tokens[2]);
        bar.low    = std::stod(tokens[3]);
        bar.close  = std::stod(tokens[4]);
        bar.volume = std::stol(tokens[5]);

        // Basic sanity checks
        if (bar.high < bar.low)
            throw std::runtime_error("high < low");
        if (bar.open <= 0 || bar.close <= 0)
            throw std::runtime_error("non-positive price");
        if (bar.volume < 0)
            throw std::runtime_error("negative volume");

        return bar;

    } catch (const std::invalid_argument& e) {
        throw std::runtime_error(
            "[MarketData] Line " + std::to_string(lineNumber) +
            ": failed to parse numeric value → " + std::string(e.what())
        );
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(
            "[MarketData] Line " + std::to_string(lineNumber) +
            ": data validation failed → " + std::string(e.what())
        );
    }
}

// ─────────────────────────────────────────────
//  Price series extractors
// ─────────────────────────────────────────────
std::vector<double> MarketData::closePrices() const {
    std::vector<double> out;
    out.reserve(bars_.size());
    for (const auto& b : bars_) out.push_back(b.close);
    return out;
}

std::vector<double> MarketData::openPrices() const {
    std::vector<double> out;
    out.reserve(bars_.size());
    for (const auto& b : bars_) out.push_back(b.open);
    return out;
}

std::vector<double> MarketData::volumes() const {
    std::vector<double> out;
    out.reserve(bars_.size());
    for (const auto& b : bars_) out.push_back(static_cast<double>(b.volume));
    return out;
}

// ─────────────────────────────────────────────
//  printSummary()
// ─────────────────────────────────────────────
void MarketData::printSummary() const {
    if (bars_.empty()) {
        std::cout << "[MarketData] No data loaded.\n";
        return;
    }

    const auto& first = bars_.front();
    const auto& last  = bars_.back();

    // Compute price range
    double minClose = first.close, maxClose = first.close;
    for (const auto& b : bars_) {
        minClose = std::min(minClose, b.close);
        maxClose = std::max(maxClose, b.close);
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n══════════════════════════════════════\n";
    std::cout << "  MarketData Summary\n";
    std::cout << "  File    : " << filepath_     << "\n";
    std::cout << "  Bars    : " << bars_.size()  << "\n";
    std::cout << "  From    : " << first.date    << "  close=" << first.close << "\n";
    std::cout << "  To      : " << last.date     << "  close=" << last.close  << "\n";
    std::cout << "  Range   : [" << minClose << ", " << maxClose << "]\n";
    std::cout << "══════════════════════════════════════\n\n";
}

} // namespace trading