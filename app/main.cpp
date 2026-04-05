// app/main.cpp
#include <iostream>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <iomanip>

#include "core/Backtester.h"
#include "core/Metrics.h"
#include "core/Optimizer.h"
#include "core/Report.h"
#include "strategy/MovingAverageStrategy.h"
#include "strategy/RSIStrategy.h"
#include "strategy/BollingerBandStrategy.h"
#include "utils/Logger.h"
#include "utils/Config.h"

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
trading::LogLevel parseLogLevel(const std::string& s) {
    if (s == "DEBUG")   return trading::LogLevel::DEBUG;
    if (s == "WARNING") return trading::LogLevel::WARNING;
    if (s == "ERROR")   return trading::LogLevel::ERROR;
    return trading::LogLevel::INFO;
}

trading::OptimizationMetric parseMetric(const std::string& s) {
    if (s == "RETURN")        return trading::OptimizationMetric::TOTAL_RETURN;
    if (s == "DRAWDOWN")      return trading::OptimizationMetric::MAX_DRAWDOWN;
    if (s == "PROFIT_FACTOR") return trading::OptimizationMetric::PROFIT_FACTOR;
    if (s == "WIN_RATE")      return trading::OptimizationMetric::WIN_RATE;
    return trading::OptimizationMetric::SHARPE_RATIO;
}

// ─────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {

    // ── Config ────────────────────────────────────────────
    std::string configPath = (argc > 1) ? argv[1] : "config.ini";
    trading::Config cfg;
    try {
        cfg = trading::Config(configPath);
    } catch (const std::exception& e) {
        std::cerr << "[WARN] " << e.what() << " — using defaults.\n\n";
    }

    // ── Logger ────────────────────────────────────────────
    std::string logFile  = cfg.getString("logging", "log_file",  "logs/backtester.log");
    std::string logLevel = cfg.getString("logging", "log_level", "INFO");
    try {
        trading::Logger::instance().init(logFile, parseLogLevel(logLevel));
    } catch (const std::exception& e) {
        std::cerr << "[WARN] Log file unavailable: " << e.what() << "\n";
    }

    LOG_INFO("Main", "QuantTradingSystem v1.0 — Complete Backtesting Engine");
    LOG_INFO("Main", "Config: " + configPath);

    // ── Session timer ─────────────────────────────────────
    auto sessionStart = std::chrono::steady_clock::now();

    // ── Read config ───────────────────────────────────────
    std::string dataPath    = cfg.getString("backtester", "data_path",    "data/prices.csv");
    double      initialCash = cfg.getDouble ("backtester", "initial_cash", 100000.0);
    int         tradingDays = cfg.getInt    ("backtester", "trading_days", 252);

    // ── Report path: reports/YYYY-MM-DD_HH-MM.txt ────────
    auto now     = std::chrono::system_clock::now();
    auto time_t  = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif
    std::ostringstream reportPath;
    reportPath << "reports/session_"
               << std::put_time(&tm, "%Y-%m-%d_%H-%M")
               << ".txt";

    try {
        // ── Load Data ─────────────────────────────────────
        trading::MarketData data(dataPath);
        data.printSummary();

        // ── Report setup ──────────────────────────────────
        trading::Report report(reportPath.str());
        report.setTitle("QuantTradingSystem v1.0 — Backtest Session Report");
        report.setDataInfo(
            dataPath,
            (int)data.size(),
            data.bars().front().date,
            data.bars().back().date
        );

        // ════════════════════════════════════════
        //  1. BACKTEST — all strategies from config
        // ════════════════════════════════════════
        trading::Backtester bt(dataPath, initialCash);

        // MA
        {
            int sw = cfg.getInt("strategy.ma", "short_window", 5);
            int lw = cfg.getInt("strategy.ma", "long_window",  20);
            auto type = cfg.getString("strategy.ma", "ma_type", "SMA");
            auto maType = (type == "EMA") ? trading::MAType::EMA : trading::MAType::SMA;
            bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(sw, lw, maType));
        }
        // RSI
        {
            int    p  = cfg.getInt   ("strategy.rsi", "period",     14);
            double os = cfg.getDouble("strategy.rsi", "oversold",   30.0);
            double ob = cfg.getDouble("strategy.rsi", "overbought", 70.0);
            bt.addStrategy(std::make_shared<trading::RSIStrategy>(p, os, ob));
        }
        // Bollinger
        {
            int    p = cfg.getInt   ("strategy.bollinger", "period", 20);
            double k = cfg.getDouble("strategy.bollinger", "k",      2.0);
            bt.addStrategy(std::make_shared<trading::BollingerBandStrategy>(p, k));
        }

        bt.run();
        bt.printResults();
        bt.printSummary();

        auto metrics = trading::Metrics::computeAll(bt.results(), 0.0, tradingDays);
        trading::Metrics::printComparison(metrics);

        report.addBacktestResults(bt.results());
        report.addMetrics(metrics);

        // ════════════════════════════════════════
        //  2. OPTIMIZATION — MA
        // ════════════════════════════════════════
        if (cfg.getBool("optimization.ma", "enabled", false)) {
            std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
            std::cout << "  OPTIMIZATION — MovingAverageStrategy\n";
            std::cout << "╚══════════════════════════════════════════════════════╝\n";

            trading::Optimizer maOpt(data, initialCash, tradingDays);
            maOpt.setMetric(parseMetric(cfg.getString("optimization.ma", "metric", "SHARPE")));

            std::vector<trading::ParamRange> ranges = {
                {"shortWindow", cfg.getDouble("optimization.ma","short_min",2),
                                cfg.getDouble("optimization.ma","short_max",8), 1},
                {"longWindow",  cfg.getDouble("optimization.ma","long_min",10),
                                cfg.getDouble("optimization.ma","long_max",30),
                                cfg.getDouble("optimization.ma","long_step",5)}
            };

            std::cout << "  Searching " << maOpt.totalCombinations(ranges) << " combinations...\n";
            auto maResults = maOpt.gridSearch(ranges,
                [](std::vector<double> p) -> std::shared_ptr<trading::Strategy> {
                    int sw = (int)p[0], lw = (int)p[1];
                    if (sw >= lw) return nullptr;
                    return std::make_shared<trading::MovingAverageStrategy>(sw, lw, trading::MAType::SMA);
                });
            maOpt.printResults(maResults, cfg.getInt("optimization.ma","top_n",5));
            maOpt.printBest(maResults);
            report.addOptimization("MovingAverageStrategy (SMA)", maResults,
                cfg.getInt("optimization.ma","top_n",5));
        }

        // ════════════════════════════════════════
        //  3. OPTIMIZATION — RSI
        // ════════════════════════════════════════
        if (cfg.getBool("optimization.rsi", "enabled", false)) {
            std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
            std::cout << "  OPTIMIZATION — RSIStrategy\n";
            std::cout << "╚══════════════════════════════════════════════════════╝\n";

            trading::Optimizer rsiOpt(data, initialCash, tradingDays);
            rsiOpt.setMetric(parseMetric(cfg.getString("optimization.rsi","metric","SHARPE")));

            std::vector<trading::ParamRange> ranges = {
                {"period",     cfg.getDouble("optimization.rsi","period_min",10),
                               cfg.getDouble("optimization.rsi","period_max",20),
                               cfg.getDouble("optimization.rsi","period_step",2)},
                {"oversold",   cfg.getDouble("optimization.rsi","oversold_min",20),
                               cfg.getDouble("optimization.rsi","oversold_max",35),
                               cfg.getDouble("optimization.rsi","oversold_step",5)},
                {"overbought", cfg.getDouble("optimization.rsi","overbought_min",65),
                               cfg.getDouble("optimization.rsi","overbought_max",80),
                               cfg.getDouble("optimization.rsi","overbought_step",5)}
            };

            std::cout << "  Searching " << rsiOpt.totalCombinations(ranges) << " combinations...\n";
            auto rsiResults = rsiOpt.gridSearch(ranges,
                [](std::vector<double> p) -> std::shared_ptr<trading::Strategy> {
                    int period = (int)p[0]; double os = p[1], ob = p[2];
                    if (os >= ob) return nullptr;
                    return std::make_shared<trading::RSIStrategy>(period, os, ob);
                });
            rsiOpt.printResults(rsiResults, cfg.getInt("optimization.rsi","top_n",5));
            rsiOpt.printBest(rsiResults);
            report.addOptimization("RSIStrategy", rsiResults,
                cfg.getInt("optimization.rsi","top_n",5));
        }

        // ── Session timing ────────────────────────────────
        auto sessionEnd = std::chrono::steady_clock::now();
        double elapsed  = std::chrono::duration<double>(sessionEnd - sessionStart).count();

        std::cout << "\n";
        std::cout << "============================================================\n";
        std::cout << "  QuantTradingSystem v1.0 — Session Complete\n";
        std::cout << "  Strategies run  : " << bt.results().size()    << "\n";
        std::cout << "  Session time    : " << std::fixed
                  << std::setprecision(3) << elapsed << "s\n";
        std::cout << "  Report saved to : " << reportPath.str()        << "\n";
        std::cout << "============================================================\n\n";

        // ── Save report ───────────────────────────────────
        report.save();

        LOG_INFO("Main", "v1.0 session complete | Time=" + std::to_string(elapsed) + "s");

    } catch (const std::exception& e) {
        LOG_ERROR("Main", std::string("Fatal: ") + e.what());
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }

    LOG_INFO("Main", "Shutdown");
    return 0;
}