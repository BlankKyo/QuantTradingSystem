// src/core/Metrics.cpp
#include "core/Metrics.h"
#include "utils/Logger.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <stdexcept>

namespace trading {

static const char* TAG = "Metrics";

// ─────────────────────────────────────────────
//  compute() — main entry point
// ─────────────────────────────────────────────
MetricsResult Metrics::compute(const BacktestResult& result,
                                double riskFreeRate,
                                int    tradingDaysPerYear)
{
    MetricsResult m;
    m.strategyName = result.strategyName;
    m.totalReturn  = result.totalReturn;
    m.totalPnL     = result.totalPnL;

    LOG_INFO(TAG, "Computing metrics for: " + result.strategyName);

    // ── Equity curve needed for risk metrics ──
    if (result.equityCurve.empty()) {
        LOG_WARNING(TAG, "Empty equity curve — skipping risk metrics");
        return m;
    }

    auto dailyRet = dailyReturns(result.equityCurve);

    m.sharpeRatio         = computeSharpe(dailyRet, riskFreeRate, tradingDaysPerYear);
    m.maxDrawdown         = computeMaxDrawdown(result.equityCurve);
    m.maxDrawdownDuration = computeMaxDrawdownDuration(result.equityCurve);
    m.volatility          = computeVolatility(dailyRet, tradingDaysPerYear);

    // ── Trade metrics ─────────────────────────
    computeTradeMetrics(result.trades, m);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4)
        << "Sharpe=" << m.sharpeRatio
        << " MaxDD="  << m.maxDrawdown  << "%"
        << " Vol="    << m.volatility   << "%"
        << " WinRate="<< m.winRate      << "%"
        << " Trades=" << m.totalTrades;
    LOG_INFO(TAG, oss.str());

    return m;
}

// ─────────────────────────────────────────────
//  computeAll()
// ─────────────────────────────────────────────
std::vector<MetricsResult> Metrics::computeAll(
    const std::vector<BacktestResult>& results,
    double riskFreeRate,
    int    tradingDaysPerYear)
{
    std::vector<MetricsResult> out;
    out.reserve(results.size());
    for (const auto& r : results)
        out.push_back(compute(r, riskFreeRate, tradingDaysPerYear));
    return out;
}

// ─────────────────────────────────────────────
//  dailyReturns()
//
//  Computes bar-to-bar % return from equity curve.
//  Return[i] = (equity[i] - equity[i-1]) / equity[i-1]
//
//  Returns empty vector if fewer than 2 points.
// ─────────────────────────────────────────────
std::vector<double> Metrics::dailyReturns(const std::vector<EquityPoint>& curve) {
    if (curve.size() < 2) return {};

    std::vector<double> ret;
    ret.reserve(curve.size() - 1);

    for (size_t i = 1; i < curve.size(); ++i) {
        double prev = curve[i-1].totalEquity;
        double curr = curve[i].totalEquity;
        if (prev > 0.0)
            ret.push_back((curr - prev) / prev);
        else
            ret.push_back(0.0);
    }
    return ret;
}

// ─────────────────────────────────────────────
//  computeSharpe()
//
//  Sharpe = (mean(dailyRet) - dailyRiskFree) / std(dailyRet)
//           * sqrt(tradingDaysPerYear)   [annualized]
//
//  We use risk-free = 0 by default (conservative).
//  Returns 0.0 if std dev is 0 (flat equity curve).
// ─────────────────────────────────────────────
double Metrics::computeSharpe(const std::vector<double>& dailyRet,
                               double riskFreeRate,
                               int    tradingDaysPerYear)
{
    if (dailyRet.empty()) return 0.0;

    double n    = static_cast<double>(dailyRet.size());
    double mean = std::accumulate(dailyRet.begin(), dailyRet.end(), 0.0) / n;

    // Daily risk-free rate
    double dailyRF = riskFreeRate / tradingDaysPerYear;

    // Standard deviation
    double variance = 0.0;
    for (double r : dailyRet)
        variance += (r - mean) * (r - mean);
    variance /= n;
    double stdDev = std::sqrt(variance);

    if (stdDev < 1e-12) return 0.0;

    // Annualize
    return ((mean - dailyRF) / stdDev) * std::sqrt(static_cast<double>(tradingDaysPerYear));
}

// ─────────────────────────────────────────────
//  computeMaxDrawdown()
//
//  Tracks the running peak equity and computes the
//  largest % drop from any peak to any subsequent trough.
//
//  MaxDD = max over all i of: (peak - trough) / peak * 100
// ─────────────────────────────────────────────
double Metrics::computeMaxDrawdown(const std::vector<EquityPoint>& curve) {
    if (curve.empty()) return 0.0;

    double peak   = curve[0].totalEquity;
    double maxDD  = 0.0;

    for (const auto& ep : curve) {
        if (ep.totalEquity > peak)
            peak = ep.totalEquity;

        if (peak > 0.0) {
            double dd = (peak - ep.totalEquity) / peak * 100.0;
            maxDD = std::max(maxDD, dd);
        }
    }
    return maxDD;
}

// ─────────────────────────────────────────────
//  computeMaxDrawdownDuration()
//
//  Counts the maximum number of consecutive bars
//  where equity is below its previous peak.
// ─────────────────────────────────────────────
int Metrics::computeMaxDrawdownDuration(const std::vector<EquityPoint>& curve) {
    if (curve.empty()) return 0;

    double peak       = curve[0].totalEquity;
    int    duration   = 0;
    int    maxDur     = 0;

    for (const auto& ep : curve) {
        if (ep.totalEquity >= peak) {
            peak     = ep.totalEquity;
            duration = 0;
        } else {
            ++duration;
            maxDur = std::max(maxDur, duration);
        }
    }
    return maxDur;
}

// ─────────────────────────────────────────────
//  computeVolatility()
//
//  Annualized std dev of daily returns (in %).
//  Vol = std(dailyRet) * sqrt(tradingDaysPerYear) * 100
// ─────────────────────────────────────────────
double Metrics::computeVolatility(const std::vector<double>& dailyRet,
                                   int tradingDaysPerYear)
{
    if (dailyRet.size() < 2) return 0.0;

    double n    = static_cast<double>(dailyRet.size());
    double mean = std::accumulate(dailyRet.begin(), dailyRet.end(), 0.0) / n;

    double variance = 0.0;
    for (double r : dailyRet)
        variance += (r - mean) * (r - mean);
    variance /= (n - 1.0); // sample std dev

    return std::sqrt(variance) * std::sqrt(static_cast<double>(tradingDaysPerYear)) * 100.0;
}

// ─────────────────────────────────────────────
//  computeTradeMetrics()
//
//  Win rate, avg win/loss, profit factor, best/worst trade.
//  Profit factor = total gross profit / total gross loss
//  (> 1.0 means system makes more than it loses)
// ─────────────────────────────────────────────
void Metrics::computeTradeMetrics(const std::vector<Trade>& trades,
                                   MetricsResult& out)
{
    out.totalTrades = (int)trades.size();
    if (trades.empty()) return;

    double grossProfit = 0.0;
    double grossLoss   = 0.0;
    double totalPnL    = 0.0;
    out.bestTrade      = trades[0].pnl;
    out.worstTrade     = trades[0].pnl;

    for (const auto& t : trades) {
        totalPnL += t.pnl;

        if (t.pnl > 0.0) {
            ++out.winningTrades;
            grossProfit  += t.pnl;
            out.avgWin   += t.pnl;
        } else {
            ++out.losingTrades;
            grossLoss    += std::abs(t.pnl);
            out.avgLoss  += t.pnl;
        }

        out.bestTrade  = std::max(out.bestTrade,  t.pnl);
        out.worstTrade = std::min(out.worstTrade, t.pnl);
    }

    out.winRate     = (double)out.winningTrades / out.totalTrades * 100.0;
    out.avgTradePnL = totalPnL / out.totalTrades;

    if (out.winningTrades > 0)
        out.avgWin /= out.winningTrades;
    if (out.losingTrades > 0)
        out.avgLoss /= out.losingTrades;

    out.profitFactor = (grossLoss > 1e-12)
        ? grossProfit / grossLoss
        : (grossProfit > 0 ? 999.0 : 0.0); // no losses = perfect
}

// ─────────────────────────────────────────────
//  print() — detailed metrics for one strategy
// ─────────────────────────────────────────────
void Metrics::print(const MetricsResult& m) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "  Performance Metrics: " << m.strategyName << "\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";

    std::cout << "\n  -- Return --\n";
    std::cout << "  Total Return    : " << std::setw(10) << m.totalReturn  << " %\n";
    std::cout << "  Total PnL       : $"<< std::setw(10) << m.totalPnL    << "\n";

    std::cout << "\n  -- Risk --\n";
    std::cout << "  Sharpe Ratio    : " << std::setw(10) << m.sharpeRatio << "\n";
    std::cout << "  Max Drawdown    : " << std::setw(10) << m.maxDrawdown << " %\n";
    std::cout << "  Max DD Duration : " << std::setw(10) << m.maxDrawdownDuration << " bars\n";
    std::cout << "  Volatility      : " << std::setw(10) << m.volatility  << " % (annualized)\n";

    std::cout << "\n  -- Trades --\n";
    std::cout << "  Total Trades    : " << std::setw(10) << m.totalTrades    << "\n";
    std::cout << "  Winning         : " << std::setw(10) << m.winningTrades  << "\n";
    std::cout << "  Losing          : " << std::setw(10) << m.losingTrades   << "\n";
    std::cout << "  Win Rate        : " << std::setw(10) << m.winRate        << " %\n";
    std::cout << "  Avg Win         : $"<< std::setw(10) << m.avgWin        << "\n";
    std::cout << "  Avg Loss        : $"<< std::setw(10) << m.avgLoss       << "\n";
    std::cout << "  Profit Factor   : " << std::setw(10) << m.profitFactor  << "\n";
    std::cout << "  Avg Trade PnL   : $"<< std::setw(10) << m.avgTradePnL  << "\n";
    std::cout << "  Best Trade      : $"<< std::setw(10) << m.bestTrade     << "\n";
    std::cout << "  Worst Trade     : $"<< std::setw(10) << m.worstTrade    << "\n";
}

// ─────────────────────────────────────────────
//  printComparison() — side-by-side table
// ─────────────────────────────────────────────
void Metrics::printComparison(const std::vector<MetricsResult>& metrics) {
    if (metrics.empty()) return;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  METRICS COMPARISON\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════════════╝\n";

    // Header
    std::cout << std::left
              << std::setw(26) << "Strategy"
              << std::setw(10) << "Return%"
              << std::setw(10) << "Sharpe"
              << std::setw(10) << "MaxDD%"
              << std::setw(10) << "Vol%"
              << std::setw(10) << "WinRate%"
              << std::setw(10) << "PFactor"
              << std::setw(8)  << "Trades"
              << "\n";
    std::cout << std::string(92, '-') << "\n";

    for (const auto& m : metrics) {
        std::cout << std::left
                  << std::setw(26) << m.strategyName
                  << std::setw(10) << m.totalReturn
                  << std::setw(10) << m.sharpeRatio
                  << std::setw(10) << m.maxDrawdown
                  << std::setw(10) << m.volatility
                  << std::setw(10) << m.winRate
                  << std::setw(10) << m.profitFactor
                  << std::setw(8)  << m.totalTrades
                  << "\n";
    }

    // Best Sharpe
    const MetricsResult* bestSharpe = &metrics[0];
    const MetricsResult* bestReturn = &metrics[0];
    for (const auto& m : metrics) {
        if (m.sharpeRatio > bestSharpe->sharpeRatio) bestSharpe = &m;
        if (m.totalReturn > bestReturn->totalReturn)  bestReturn = &m;
    }

    std::cout << "\n  Best Sharpe Ratio : " << bestSharpe->strategyName
              << " (" << bestSharpe->sharpeRatio << ")\n";
    std::cout << "  Best Return       : " << bestReturn->strategyName
              << " (" << bestReturn->totalReturn << "%)\n";

    LOG_INFO(TAG, "Best Sharpe: " + bestSharpe->strategyName
        + " | Best Return: " + bestReturn->strategyName);
}

} // namespace trading