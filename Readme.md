# C++ Quantitative Trading Backtester

## Overview
A modular, incremental trading backtesting engine written in C++17.
Currently at **v0.5** — Full Backtester Engine with multi-strategy orchestration.

## Roadmap

| Version | Feature                         | Status      |
|---------|---------------------------------|-------------|
| v0.1    | Project structure + CMake + Git | ✅ Done     |
| v0.2    | MarketData CSV loader + Logger  | ✅ Done     |
| v0.3    | Moving Average Strategy         | ✅ Done     |
| v0.4    | Portfolio simulation            | ✅ Done     |
| v0.5    | Backtester engine               | ✅ Done     |
| v0.6    | Performance metrics             | 🔜 Next     |
| v0.7    | Multiple strategies             | ⬜ Planned  |
| v0.8    | Parameter optimization          | ⬜ Planned  |
| v0.9    | Logging + config                | ⬜ Planned  |
| v1.0    | Complete backtesting engine     | ⬜ Planned  |

## Features (v0.5)
- CSV market data loader with full OHLCV validation
- Logger utility — terminal (colored) + `logs/backtester.log`
- SMA and EMA crossover strategy with configurable windows
- Long-only portfolio simulation with cash management and equity curve
- Backtester engine orchestrating the full pipeline: MarketData → Strategy → Portfolio → BacktestResult
- Multi-strategy support via `addStrategy()` — run and compare N strategies in one call
- Cross-strategy comparison summary with best performer detection

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
│   │   └── Backtester.h
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

Add strategies in `main.cpp` and call `run()`:

```cpp
trading::Backtester bt("data/prices.csv", 100000.0);

bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(5, 20, trading::MAType::SMA));
bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(5, 20, trading::MAType::EMA));
bt.addStrategy(std::make_shared<trading::MovingAverageStrategy>(3, 10, trading::MAType::SMA));

bt.run();
bt.printResults();
bt.printSummary();
```

## CSV Format

```
date,open,high,low,close,volume
2024-01-02,185.20,186.95,184.30,185.85,52341200
```

## Logs

Every session appends to `logs/backtester.log`:

```
════════════════════════════════════════════════
  Session started: 2024-01-02 13:45:01
════════════════════════════════════════════════
[2024-01-02 13:45:01] [INFO ]  [Backtester] Starting backtest | Strategies=3
[2024-01-02 13:45:01] [INFO ]  [Backtester] Best strategy: MovingAverageStrategy return=2.54%
```

## Requirements
- CMake 3.16+
- C++17 compiler (MSVC 2019+, GCC 8+, Clang 10+)
