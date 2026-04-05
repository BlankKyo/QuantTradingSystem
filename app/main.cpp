// app/main.cpp
#include <iostream>
#include <memory>
#include <stdexcept>

#include "core/Backtester.h"
#include "core/Metrics.h"
#include "core/Optimizer.h"
#include "strategy/MovingAverageStrategy.h"
#include "strategy/RSIStrategy.h"
#include "strategy/BollingerBandStrategy.h"
#include "utils/Logger.h"
#include "utils/Config.h"

// ─────────────────────────────────────────────
//  parseLogLevel — maps config string to LogLevel
// ─────────────────────────────────────────────
trading::LogLevel parseLogLevel(const std::string& s) {
    if (s == "DEBUG")   return trading::LogLevel::DEBUG;
    if (s == "WARNING") return trading::LogLevel::WARNING;
    if (s == "ERROR")   return trading::LogLevel::ERROR;
    return trading::LogLevel::INFO; // default
}

// ─────────────────────────────────────────────
//  parseMetric — maps config string to OptimizationMetric
// ─────────────────────────────────────────────
trading::OptimizationMetric parseMetric(const std::string& s) {
    if (s == "RETURN")        return trading::OptimizationMetric::TOTAL_RETURN;
    if (s == "DRAWDOWN")      return trading::OptimizationMetric::MAX_DRAWDOWN;
    if (s == "PROFIT_FACTOR") return trading::OptimizationMetric::PROFIT_FACTOR;
    if (s == "WIN_RATE")      return trading::OptimizationMetric::WIN_RATE;
    return trading::OptimizationMetric::SHARPE_RATIO; // default
}

int main(int argc, char* argv[]) {
    // ── Load Config ───────────────────────────────────────
    std::string configPath = (argc > 1) ? argv[1] : "config.ini";

    trading::Config cfg;
    try {
        cfg = trading::Config(configPath);
    } catch (const std::exception& e) {
        std::cerr << "[WARN] Could not load config: " << e.what() << "\n";
        std::cerr << "[WARN] Running with built-in defaults.\n\n";
    }

    // ── Init Logger from config ───────────────────────────
    std::string logFile  = cfg.getString("logging", "log_file",  "logs/backtester.log");
    std::string logLevel = cfg.getString("logging", "log_level", "INFO");

    try {
        trading::Logger::instance().init(logFile, parseLogLevel(logLevel));
    } catch (const std::exception& e) {
        std::cerr << "[WARN] Could not open log file: " << e.what() << "\n";
    }

    LOG_INFO("Main", "QuantTradingSystem v0.9 starting");
    LOG_INFO("Main", "Config: " + configPath + " | LogLevel: " + logLevel);

    // ── Read config values ────────────────────────────────
    std::string dataPath    = cfg.getString("backtester", "data_path",    "data/prices.csv");
    double      initialCash = cfg.getDouble ("backtester", "initial_cash", 100000.0);
    int         tradingDays = cfg.getInt    ("backtester", "trading_days", 252);

    cfg.print();

    try {
        trading::MarketData data(dataPath);
        data.printSummary();

        // ════════════════════════════════════════
        //  SECTION 1 — Backtest with config params
        // ════════════════════════════════════════
        trading::Backtester bt(dataPath, initialCash);

        // MA strategy from config
        {
            int         sw   = cfg.getInt   ("strategy.ma", "short_window", 5);
            int         lw   = cfg.getInt   ("strategy.ma", "long_window",  20);
            std::string type = cfg.getString("strategy.ma", "ma_type",      "SMA");
            auto maType = (type == "EMA") ? trading::MAType::EMA : trading::MAType::SMA;
            bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(sw, lw, maType));
        }

        // RSI strategy from config
        {
            int    period     = cfg.getInt   ("strategy.rsi", "period",      14);
            double oversold   = cfg.getDouble("strategy.rsi", "oversold",    30.0);
            double overbought = cfg.getDouble("strategy.rsi", "overbought",  70.0);
            bt.addStrategy(std::make_shared<trading::RSIStrategy>(period, oversold, overbought));
        }

        // Bollinger strategy from config
        {
            int    period = cfg.getInt   ("strategy.bollinger", "period", 20);
            double k      = cfg.getDouble("strategy.bollinger", "k",      2.0);
            bt.addStrategy(std::make_shared<trading::BollingerBandStrategy>(period, k));
        }

        bt.run();
        bt.printSummary();

        auto metrics = trading::Metrics::computeAll(bt.results(), 0.0, tradingDays);
        trading::Metrics::printComparison(metrics);

        // ════════════════════════════════════════
        //  SECTION 2 — MA Optimization (if enabled)
        // ════════════════════════════════════════
        if (cfg.getBool("optimization.ma", "enabled", false)) {
            std::cout << "\n\n";
            std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
            std::cout << "  PARAMETER OPTIMIZATION — MovingAverageStrategy (from config)\n";
            std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";

            double shortMin  = cfg.getDouble("optimization.ma", "short_min",  2);
            double shortMax  = cfg.getDouble("optimization.ma", "short_max",  8);
            double longMin   = cfg.getDouble("optimization.ma", "long_min",   10);
            double longMax   = cfg.getDouble("optimization.ma", "long_max",   30);
            double longStep  = cfg.getDouble("optimization.ma", "long_step",  5);
            int    topN      = cfg.getInt   ("optimization.ma", "top_n",      5);
            auto   metric    = parseMetric(cfg.getString("optimization.ma", "metric", "SHARPE"));

            trading::Optimizer maOpt(data, initialCash, tradingDays);
            maOpt.setMetric(metric);

            std::vector<trading::ParamRange> ranges = {
                {"shortWindow", shortMin, shortMax, 1},
                {"longWindow",  longMin,  longMax,  longStep}
            };

            std::cout << "  Searching " << maOpt.totalCombinations(ranges) << " combinations...\n";

            auto results = maOpt.gridSearch(ranges,
                [](std::vector<double> p) -> std::shared_ptr<trading::Strategy> {
                    int sw = (int)p[0], lw = (int)p[1];
                    if (sw >= lw) return nullptr;
                    return std::make_shared<trading::MovingAverageStrategy>(sw, lw, trading::MAType::SMA);
                }
            );
            maOpt.printResults(results, topN);
            maOpt.printBest(results);
        }

        // ════════════════════════════════════════
        //  SECTION 3 — RSI Optimization (if enabled)
        // ════════════════════════════════════════
        if (cfg.getBool("optimization.rsi", "enabled", false)) {
            std::cout << "\n\n";
            std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
            std::cout << "  PARAMETER OPTIMIZATION — RSIStrategy (from config)\n";
            std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";

            double pMin   = cfg.getDouble("optimization.rsi", "period_min",      10);
            double pMax   = cfg.getDouble("optimization.rsi", "period_max",      20);
            double pStep  = cfg.getDouble("optimization.rsi", "period_step",     2);
            double osMin  = cfg.getDouble("optimization.rsi", "oversold_min",    20);
            double osMax  = cfg.getDouble("optimization.rsi", "oversold_max",    35);
            double osStep = cfg.getDouble("optimization.rsi", "oversold_step",   5);
            double obMin  = cfg.getDouble("optimization.rsi", "overbought_min",  65);
            double obMax  = cfg.getDouble("optimization.rsi", "overbought_max",  80);
            double obStep = cfg.getDouble("optimization.rsi", "overbought_step", 5);
            int    topN   = cfg.getInt   ("optimization.rsi", "top_n",           5);
            auto   metric = parseMetric(cfg.getString("optimization.rsi", "metric", "SHARPE"));

            trading::Optimizer rsiOpt(data, initialCash, tradingDays);
            rsiOpt.setMetric(metric);

            std::vector<trading::ParamRange> ranges = {
                {"period",     pMin,  pMax,  pStep},
                {"oversold",   osMin, osMax, osStep},
                {"overbought", obMin, obMax, obStep}
            };

            std::cout << "  Searching " << rsiOpt.totalCombinations(ranges) << " combinations...\n";

            auto results = rsiOpt.gridSearch(ranges,
                [](std::vector<double> p) -> std::shared_ptr<trading::Strategy> {
                    int    period     = (int)p[0];
                    double oversold   = p[1];
                    double overbought = p[2];
                    if (oversold >= overbought) return nullptr;
                    return std::make_shared<trading::RSIStrategy>(period, oversold, overbought);
                }
            );
            rsiOpt.printResults(results, topN);
            rsiOpt.printBest(results);
        }

        std::cout << "\nReady for v1.0 - Complete Backtesting Engine\n\n";
        LOG_INFO("Main", "v0.9 complete - ready for v1.0");

    } catch (const std::exception& e) {
        LOG_ERROR("Main", std::string("Fatal: ") + e.what());
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }

    LOG_INFO("Main", "Session complete");
    return 0;
}