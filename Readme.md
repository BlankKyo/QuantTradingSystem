# C++ Quantitative Trading Backtester

## Overview
A modular, incremental trading backtesting engine written in C++17.
Currently at **v0.9** — Config file support and enhanced logging.

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
| v0.9    | Logging + config                | ✅ Done     |
| v1.0    | Complete backtesting engine     | 🔜 Next     |

## Features (v0.9)
- CSV market data loader with full OHLCV validation
- **Config-driven execution** — all parameters read from `config.ini`:
  - Data path, initial cash, trading days
  - Log level and log file path
  - Per-strategy parameters (MA windows, RSI thresholds, Bollinger k)
  - Optimizer search ranges and target metric — all configurable
- Logger utility — terminal (colored) + file, **log level set from config**
- 3 strategy types: MovingAverage (SMA/EMA), RSI, Bollinger Bands
- Long-only portfolio simulation with cash management and equity curve
- Backtester engine orchestrating the full pipeline
- Metrics engine: Sharpe, Max Drawdown, Volatility, Win Rate, Profit Factor
- Optimizer — grid search over configurable parameter ranges

## Project Structure

```
TradingBacktester/
├── app/main.cpp
├── config.ini                   ← NEW v0.9
├── data/prices.csv
├── include/
│   ├── core/
│   │   ├── MarketData.h
│   │   ├── Strategy.h
│   │   ├── Backtester.h
│   │   ├── Metrics.h
│   │   └── Optimizer.h
│   ├── strategy/
│   │   ├── MovingAverageStrategy.h
│   │   ├── RSIStrategy.h
│   │   └── BollingerBandStrategy.h
│   ├── portfolio/Portfolio.h
│   └── utils/
│       ├── Logger.h
│       └── Config.h             ← NEW v0.9
├── src/
│   ├── core/
│   │   ├── MarketData.cpp
│   │   ├── Backtester.cpp
│   │   ├── Metrics.cpp
│   │   └── Optimizer.cpp
│   ├── strategy/
│   │   ├── MovingAverageStrategy.cpp
│   │   ├── RSIStrategy.cpp
│   │   └── BollingerBandStrategy.cpp
│   ├── portfolio/Portfolio.cpp
│   └── utils/
│       ├── Logger.cpp
│       └── Config.cpp           ← NEW v0.9
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
.\bin\Debug\QuantTradingSystem.exe ..\config.ini
```

**Linux / macOS:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./bin/QuantTradingSystem ../config.ini
```

## Config File

All parameters are driven by `config.ini`. Pass a custom config as the first argument:

```bash
./bin/QuantTradingSystem my_config.ini
```

### Config Reference

```ini
[backtester]
data_path     = data/prices.csv
initial_cash  = 100000
trading_days  = 252

[logging]
log_level     = INFO          # DEBUG | INFO | WARNING | ERROR
log_file      = logs/backtester.log

[strategy.ma]
short_window  = 5
long_window   = 20
ma_type       = SMA           # SMA | EMA

[strategy.rsi]
period        = 14
oversold      = 30
overbought    = 70

[strategy.bollinger]
period        = 20
k             = 2.0

[optimization.ma]
enabled       = true
short_min     = 2
short_max     = 8
long_min      = 10
long_max      = 30
long_step     = 5
metric        = SHARPE        # SHARPE | RETURN | DRAWDOWN | PROFIT_FACTOR | WIN_RATE
top_n         = 5

[optimization.rsi]
enabled       = true
period_min    = 10
period_max    = 20
period_step   = 2
oversold_min  = 20
oversold_max  = 35
oversold_step = 5
overbought_min   = 65
overbought_max   = 80
overbought_step  = 5
metric           = SHARPE
top_n            = 5
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