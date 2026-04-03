# C++ Trading Backtester

## Overview
A modular, incremental trading backtesting engine written in C++17.
Currently at **v0.4** — Portfolio simulation.

## Roadmap

| Version | Feature                         | Status     |
|---------|---------------------------------|------------|
| v0.1    | Project structure + CMake + Git | ✅ Done    |
| v0.2    | MarketData CSV loader + Logger  | ✅ Done    |
| v0.3    | Moving Average Strategy         | ✅ Done    |
| v0.4    | Portfolio simulation            | ✅ Done    |
| v0.5    | Backtester engine               | 🔜 Next    |
| v0.6    | Performance metrics             | ⬜ Planned |
| v0.7    | Multiple strategies             | ⬜ Planned |
| v0.8    | Parameter optimization          | ⬜ Planned |
| v0.9    | Logging + config                | ⬜ Planned |
| v1.0    | Complete backtesting engine     | ⬜ Planned |

## Features (v0.3)
- Portfolio class with full trade execution engine
- Trade struct to record completed round-trip trades (entry/exit/PnL)
- EquityPoint struct for mark-to-market equity curve
- Long-only cash management (invest all cash on BUY, close all on SELL)
- printSummary(), printTrades(), printEquityCurve() output methods
- Win rate calculation across completed trades

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
│   │   └── MarketData.cpp
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
cmake --build .
cd ..
\build\bin\Debug\QuantTradingSystem.exe data\prices.csv
```

**Linux / macOS:**
```bash
mkdir build
cd build
-cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
cd ..
\build\bin\Debug\QuantTradingSystem.exe data\prices.csv
```

## CSV Format

The loader expects the following column order:

```
date,open,high,low,close,volume
2024-01-02,185.20,186.95,184.30,185.85,52341200
```

## Logs

Logs are written to logs/backtester.log automatically on each run.
Each session is separated with a timestamp header.

```
════════════════════════════════════════════════
  Session started: 2026-04-03 01:40:00
════════════════════════════════════════════════
[2026-04-03 01:40:00] [INFO ]  [Main] QuantTradingSystem v0.4 starting
[2026-04-03 01:40:00] [INFO ]  [MovingAverageStrategy] Initialized | Type=SMA | Short=5 | Long=20
[2026-04-03 01:40:00] [INFO ]  [MovingAverageStrategy] Done | BUY=0 SELL=0 HOLD=23 Total=23
[2026-04-03 01:40:00] [INFO ]  [Portfolio] Initialized | Cash=$100000.00
[2026-04-03 01:40:00] [INFO ]  [Portfolio] Starting simulation over 23 bars
[2026-04-03 01:40:00] [INFO ]  [Portfolio] Simulation complete | Trades=0 | FinalEquity=$100000.00 | Return=0.00%
[2026-04-03 01:40:00] [INFO ]  [Portfolio] Summary | InitialCash=$100000.000000 FinalEquity=$100000.000000 Return=0.000000% Trades=0
[2026-04-03 01:40:00] [INFO ]  [MovingAverageStrategy] Initialized | Type=EMA | Short=5 | Long=20
[2026-04-03 01:40:00] [INFO ]  [MovingAverageStrategy] Done | BUY=0 SELL=0 HOLD=23 Total=23
[2026-04-03 01:40:00] [INFO ]  [Portfolio] Initialized | Cash=$100000.00
[2026-04-03 01:40:00] [INFO ]  [Portfolio] Starting simulation over 23 bars
[2026-04-03 01:40:00] [INFO ]  [Portfolio] Simulation complete | Trades=0 | FinalEquity=$100000.00 | Return=0.00%
[2026-04-03 01:40:00] [INFO ]  [Portfolio] Summary | InitialCash=$100000.000000 FinalEquity=$100000.000000 Return=0.000000% Trades=0
[2026-04-03 01:40:01] [INFO ]  [Main] v0.4 complete - ready for v0.5 Backtester
[2026-04-03 01:40:01] [INFO ]  [Main] Session complete
  Session ended:   2026-04-03 01:40:01
════════════════════════════════════════════════
```

## Requirements
- CMake 3.16+
- C++17 compiler (MSVC 2019+, GCC 8+, Clang 10+)
