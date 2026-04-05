# C++ Quantitative Trading Backtester

## Overview
A modular, incremental trading backtesting engine written in C++17.
Currently at **v0.8** — Parameter Optimization via Grid Search.

## Roadmap

| Version | Feature                         | Status      |
|---------|---------------------------------|-------------|
| v0.1    | Project structure + CMake + Git | ✅ Done     |
| v0.2    | MarketData CSV loader + Logger  | ✅ Done     |
| v0.3    | Moving Average Strategy         | ✅ Done     |
| v0.4    | Portfolio simulation            | ✅ Done     |
| v0.5    | Backtester engine               | ✅ Done     |
| v0.6    | Performance metrics             | ✅ Done     |
| v0.7    | Multiple strategies             | ✅ Done     |
| v0.8    | Parameter optimization          | ✅ Done     |
| v0.9    | Logging + config                | 🔜 Next     |
| v1.0    | Complete backtesting engine     | ⬜ Planned  |

## Features (v0.8)
- CSV market data loader with full OHLCV validation
- Logger utility — terminal (colored) + `logs/backtester.log`
- 3 strategy types: MovingAverage (SMA/EMA), RSI, Bollinger Bands
- Long-only portfolio simulation with cash management and equity curve
- Backtester engine orchestrating the full pipeline
- Metrics engine: Sharpe, Max Drawdown, Volatility, Win Rate, Profit Factor
- **Optimizer** — grid search over any parameter space:
  - `ParamRange` struct defines search dimensions
  - `OptimizationMetric` enum: SHARPE, RETURN, DRAWDOWN, PROFIT_FACTOR, WIN_RATE
  - Recursive expansion of all parameter combinations
  - Ranked results table + best parameter summary
  - MA grid search: 35 combinations (shortWindow 2–8, longWindow 10–30)
  - RSI grid search: 96 combinations (period, oversold, overbought)

## Project Structure

```
TradingBacktester/
├── app/main.cpp
├── data/prices.csv
├── include/
│   ├── core/
│   │   ├── MarketData.h
│   │   ├── Strategy.h
│   │   ├── Backtester.h
│   │   ├── Metrics.h
│   │   └── Optimizer.h          ← NEW v0.8
│   ├── strategy/
│   │   ├── MovingAverageStrategy.h
│   │   ├── RSIStrategy.h
│   │   └── BollingerBandStrategy.h
│   ├── portfolio/Portfolio.h
│   └── utils/Logger.h
├── src/
│   ├── core/
│   │   ├── MarketData.cpp
│   │   ├── Backtester.cpp
│   │   ├── Metrics.cpp
│   │   └── Optimizer.cpp        ← NEW v0.8
│   ├── strategy/
│   │   ├── MovingAverageStrategy.cpp
│   │   ├── RSIStrategy.cpp
│   │   └── BollingerBandStrategy.cpp
│   ├── portfolio/Portfolio.cpp
│   └── utils/Logger.cpp
├── logs/backtester.log
├── CMakeLists.txt
└── README.md
```

## Build

**Windows (PowerShell):**
```powershell
mkdir build
cd build
cmake ..
cmake --build .
.\bin\Debug\QuantTradingSystem.exe
```

**Linux / macOS:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./bin/QuantTradingSystem
```

## Usage — Optimizer

```cpp
trading::Optimizer opt(data, 100000.0);
opt.setMetric(trading::OptimizationMetric::SHARPE_RATIO);

auto results = opt.gridSearch(
    { {"shortWindow", 2, 8, 1}, {"longWindow", 10, 30, 5} },
    [](std::vector<double> p) -> std::shared_ptr<trading::Strategy> {
        int sw = (int)p[0], lw = (int)p[1];
        if (sw >= lw) return nullptr;
        return std::make_shared<trading::MovingAverageStrategy>(sw, lw, trading::MAType::SMA);
    }
);

opt.printResults(results, 5);  // top 5
opt.printBest(results);
```

## Strategy Reference

| Strategy | Signal Logic | Key Parameters |
|----------|-------------|----------------|
| `MovingAverageStrategy` | Short MA crosses long MA | `shortWindow`, `longWindow`, `MAType` |
| `RSIStrategy` | RSI oversold/overbought crossover | `period`, `oversoldThreshold`, `overboughtThreshold` |
| `BollingerBandStrategy` | Price breaches band | `period`, `k` |

## Metrics Reference

| Metric | Description |
|--------|-------------|
| Sharpe Ratio | Annualized return / volatility. >1 good, >2 excellent |
| Max Drawdown | Largest % drop from peak to trough |
| Volatility | Annualized std dev of daily returns (%) |
| Win Rate | % of completed trades that were profitable |
| Profit Factor | Gross profit / gross loss. >1 = profitable system |

## CSV Format

```
date,open,high,low,close,volume
2024-01-02,185.20,186.95,184.30,185.85,52341200
```

## Requirements
- CMake 3.16+
- C++17 compiler (MSVC 2019+, GCC 8+, Clang 10+)