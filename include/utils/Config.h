// include/utils/Config.h
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

namespace trading {

// ─────────────────────────────────────────────
//  Config — INI-style config file reader
//
//  Supports sections and key=value pairs.
//  Lines starting with # or ; are comments.
//
//  Example config.ini:
//    [backtester]
//    initial_cash = 100000
//    data_path    = data/prices.csv
//    trading_days = 252
//
//    [logging]
//    log_level    = DEBUG
//    log_file     = logs/backtester.log
//
//    [strategy.ma]
//    short_window = 5
//    long_window  = 20
//    ma_type      = SMA
//
//  Usage:
//    Config cfg("config.ini");
//    double cash = cfg.getDouble("backtester", "initial_cash");
//    std::string path = cfg.getString("backtester", "data_path");
// ─────────────────────────────────────────────
class Config {
public:
    // Load from file — throws if file not found
    explicit Config(const std::string& filepath);

    // Create with defaults only (no file)
    Config() = default;

    // ── Getters with type conversion ──────────
    std::string getString(const std::string& section,
                          const std::string& key,
                          const std::string& defaultVal = "") const;

    double      getDouble(const std::string& section,
                          const std::string& key,
                          double             defaultVal = 0.0) const;

    int         getInt   (const std::string& section,
                          const std::string& key,
                          int                defaultVal = 0) const;

    bool        getBool  (const std::string& section,
                          const std::string& key,
                          bool               defaultVal = false) const;

    // ── Existence checks ──────────────────────
    bool hasSection(const std::string& section) const;
    bool hasKey    (const std::string& section, const std::string& key) const;

    // ── List all sections and keys ────────────
    std::vector<std::string> sections()                       const;
    std::vector<std::string> keys(const std::string& section) const;

    // ── Print loaded config to terminal ───────
    void print() const;

    // ── Filepath used ─────────────────────────
    const std::string& filepath() const { return filepath_; }

private:
    // [section][key] = value
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::string>> data_;
    std::string filepath_;

    void        load(const std::string& filepath);
    static std::string trim(const std::string& s);
    static std::string toLower(const std::string& s);
};

} // namespace trading