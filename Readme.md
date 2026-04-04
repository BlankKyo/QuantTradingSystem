# C++ Quantitative Trading Backtester

## Overview
A modular, incremental trading backtesting engine written in C++17.
Currently at **v0.6** — Performance Metrics (Sharpe, Drawdown, Volatility, Win Rate).

## Roadmap

| Version | Feature                         | Status      |
|---------|---------------------------------|-------------|
| v0.1    | Project structure + CMake + Git | ✅ Done     |
| v0.2    | MarketData CSV loader + Logger  | ✅ Done     |
| v0.3    | Moving Average Strategy         | ✅ Done     |
| v0.4    | Portfolio simulation            | ✅ Done     |
| v0.5    | Backtester engine               | ✅ Done     |
| v0.6    | Performance metrics             | ✅ Done     |
| v0.7    | Multiple strategies             | 🔜 Next     |
| v0.8    | Parameter optimization          | ⬜ Planned  |
| v0.9    | Logging + config                | ⬜ Planned  |
| v1.0    | Complete backtesting engine     | ⬜ Planned  |

## Features (v0.6)
- CSV market data loader with full OHLCV validation
- Logger utility — terminal (colored) + `logs/backtester.log`
- SMA and EMA crossover strategy with configurable windows
- Long-only portfolio simulation with cash management and equity curve
- Backtester engine orchestrating the full pipeline
- **Metrics engine** computing:
  - Sharpe Ratio (annualized, risk-free configurable)
  - Max Drawdown (%) and Max Drawdown Duration (bars)
  - Annualized Volatility (%)
  - Win Rate, Avg Win, Avg Loss, Profit Factor
  - Best/Worst Trade, Avg Trade PnL
- Cross-strategy metrics comparison table

## Project Structure

```
TradingBacktester/
├── app/
│   └── main.cpp
├── data/
│   └── prices.csv
├── include/
│   ├── core/
│   │   ├── MarketData.h
│   │   ├── Strategy.h
│   │   ├── Backtester.h
│   │   └── Metrics.h          ← NEW
│   ├── strategy/
│   │   └── MovingAverageStrategy.h
│   ├── portfolio/
│   │   └── Portfolio.h
│   └── utils/
│       └── Logger.h
├── src/
│   ├── core/
│   │   ├── MarketData.cpp
│   │   ├── Backtester.cpp
│   │   └── Metrics.cpp        ← NEW
│   ├── strategy/
│   │   └── MovingAverageStrategy.cpp
│   ├── portfolio/
│   │   └── Portfolio.cpp
│   └── utils/
│       └── Logger.cpp
├── logs/
│   └── backtester.log
├── CMakeLists.txt
├── README.md
└── .gitignore
```

## Build

**Windows (PowerShell):**
```powershell
mkdir build
cd build
cmake ..
cmake --build .
.\bin\Debug\QuantTradingSystem.exe ..\..\data\prices.csv
```

**Linux / macOS:**
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./bin/QuantTradingSystem ../data/prices.csv
```

## Usage

```cpp
trading::Backtester bt("data/prices.csv", 100000.0);
bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(5, 20, trading::MAType::SMA));
bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(5, 20, trading::MAType::EMA));
bt.run();
bt.printResults();

// Compute and print metrics
auto metrics = trading::Metrics::computeAll(bt.results());
for (const auto& m : metrics)
    trading::Metrics::print(m);
trading::Metrics::printComparison(metrics);
```

## Metrics Reference

| Metric | Description |
|--------|-------------|
| Sharpe Ratio | Annualized return / annualized volatility. >1 good, >2 excellent |
| Max Drawdown | Largest % drop from any peak to trough |
| Max DD Duration | Longest consecutive bars spent below peak equity |
| Volatility | Annualized std dev of daily returns (%) |
| Win Rate | % of completed trades that were profitable |
| Profit Factor | Gross profit / gross loss. >1 means system makes money |

## CSV Format

```
date,open,high,low,close,volume
2024-01-02,185.20,186.95,184.30,185.85,52341200
```

## Requirements
- CMake 3.16+
- C++17 compiler (MSVC 2019+, GCC 8+, Clang 10+)
