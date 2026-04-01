# C++ Trading Backtester

## Overview
This project is a modular trading backtesting engine written in C++.

## Features
- Market data loader
- Trading strategies
- Portfolio simulation
- Performance metrics

## Project Structure
```
TradingBacktester/
├── app/
│   └── main.cpp
├── data/
│   └── prices.csv
│
├── include/
│   ├── core/
│   │   ├── MarketData.h
│   │   ├── Strategy.h
│   │   ├── Backtester.h
│   │
│   ├── strategy/
│   │   └── MovingAverageStrategy.h
│   │
│   ├── portfolio/
│   │   └── Portfolio.h
│
├── src/
│   ├── core/
│   │   ├── MarketData.cpp
│   │   ├── Backtester.cpp
│   │
│   ├── strategy/
│   │   └── MovingAverageStrategy.cpp
│   │
│   ├── portfolio/
│   │   └── Portfolio.cpp
│
│
├── CMakeLists.txt
├── README.md
└── .gitignore 
```

## Build
mkdir build
cd build
cmake ..
make