# C++ Trading Backtester

## Overview
A modular, incremental trading backtesting engine written in C++17.
Currently at **v0.2** — MarketData CSV loader + Logger utility.

## Roadmap

| Version | Feature                         | Status     |
|---------|---------------------------------|------------|
| v0.1    | Project structure + CMake + Git | ✅ Done    |
| v0.2    | MarketData CSV loader + Logger  | ✅ Done    |
| v0.3    | Moving Average Strategy         | 🔜 Next    |
| v0.4    | Portfolio simulation            | ⬜ Planned |
| v0.5    | Backtester engine               | ⬜ Planned |
| v0.6    | Performance metrics             | ⬜ Planned |
| v0.7    | Multiple strategies             | ⬜ Planned |
| v0.8    | Parameter optimization          | ⬜ Planned |
| v0.9    | Logging + config                | ⬜ Planned |
| v1.0    | Complete backtesting engine     | ⬜ Planned |

## Features (v0.2)
- CSV market data loader with full validation (OHLCV format)
- Logger utility — writes to terminal and logs/backtester.log
- Stub interfaces for Strategy, Backtester, Portfolio (ready for upcoming versions)

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
│   ├── portfolio/
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
.\bin\Debug\TradingBacktester.exe data\prices.csv
```

**Linux / macOS:**
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./bin/TradingBacktester ../data/prices.csv
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
  Session started: 2024-01-02 13:45:01
════════════════════════════════════════════════
[2024-01-02 13:45:01] [INFO ]  [MarketData] Loading file: data/prices.csv
[2024-01-02 13:45:01] [INFO ]  [MarketData] Loaded 20 bars successfully
[2024-01-02 13:45:01] [INFO ]  [Main] Session complete
```

## Requirements
- CMake 3.16+
- C++17 compiler (MSVC 2019+, GCC 8+, Clang 10+)
