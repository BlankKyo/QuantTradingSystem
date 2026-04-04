# C++ Quantitative Trading Backtester

## Overview
A modular, incremental trading backtesting engine written in C++17.
Currently at **v0.7** — Multiple strategies (RSI, BollingerBand).

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
| v0.8    | Parameter optimization          | 🔜 Next     |
| v0.9    | Logging + config                | ⬜ Planned  |
| v1.0    | Complete backtesting engine     | ⬜ Planned  |

## Features (v0.7)
- RSI-based trading strategy implementation
- Bollinger Bands trading strategy implementation

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
│   │   └── Metrics.h          
│   ├── strategy/
│   │   ├── MovingAverageStrategy.h
│   │   ├── RSIStrategy.h                 ← NEW
│   │   └── BollingerBandStrategy.h       ← NEW
│   ├── portfolio/
│   │   └── Portfolio.h
│   └── utils/
│       └── Logger.h
├── src/
│   ├── core/
│   │   ├── MarketData.cpp
│   │   ├── Backtester.cpp
│   │   └── Metrics.cpp        
│   ├── strategy/
│   │   ├── MovingAverageStrategy.cpp
│   │   ├── RSIStrategy.cpp                  ← NEW
│   │   └── BollingerBandStrategy.cpp        ← NEW
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
.\bin\Debug\QuantTradingSystem.exe data\prices.csv
```

**Linux / macOS:**
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./bin/QuantTradingSystem data/prices.csv
```


## CSV Format

```
date,open,high,low,close,volume
2024-01-02,185.20,186.95,184.30,185.85,52341200
```

## Requirements
- CMake 3.16+
- C++17 compiler (MSVC 2019+, GCC 8+, Clang 10+)
