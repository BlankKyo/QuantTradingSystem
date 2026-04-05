# C++ Quantitative Trading Backtester

## Overview
A complete, modular quantitative trading backtesting engine written in C++17.
**v1.0** — production-ready, config-driven, with full reporting.

## Roadmap

| Version | Feature                         | Status   |
|---------|---------------------------------|----------|
| v0.1    | Project structure + CMake + Git | ✅ Done  |
| v0.2    | MarketData CSV loader + Logger  | ✅ Done  |
| v0.3    | Moving Average Strategy         | ✅ Done  |
| v0.4    | Portfolio simulation            | ✅ Done  |
| v0.5    | Backtester engine               | ✅ Done  |
| v0.6    | Performance metrics             | ✅ Done  |
| v0.7    | Multiple strategies             | ✅ Done  |
| v0.8    | Parameter optimization          | ✅ Done  |
| v0.9    | Logging + config                | ✅ Done  |
| v1.0    | Complete backtesting engine     | ✅ Done  |

## Features
- **Config-driven** — all parameters controlled from `config.ini`
- **3 strategy types** — MovingAverage (SMA/EMA), RSI, Bollinger Bands
- **Full pipeline** — MarketData → Strategy → Portfolio → BacktestResult
- **Performance metrics** — Sharpe Ratio, Max Drawdown, Volatility, Win Rate, Profit Factor
- **Grid search optimizer** — search any parameter space, rank by any metric
- **Report generation** — structured session report saved to `reports/`
- **Session timing** — measures total execution time per run
- **Thread-safe logger** — colored terminal output + persistent log file

## Project Structure

```
TradingBacktester/
├── app/main.cpp
├── config.ini
├── data/prices.csv
├── reports/                     ← auto-generated session reports
├── logs/backtester.log
├── include/
│   ├── core/
│   │   ├── MarketData.h
│   │   ├── Strategy.h
│   │   ├── Backtester.h
│   │   ├── Metrics.h
│   │   ├── Optimizer.h
│   │   └── Report.h             ← NEW v1.0
│   ├── strategy/
│   │   ├── MovingAverageStrategy.h
│   │   ├── RSIStrategy.h
│   │   └── BollingerBandStrategy.h
│   ├── portfolio/Portfolio.h
│   └── utils/
│       ├── Logger.h
│       └── Config.h
└── src/
    ├── core/
    │   ├── MarketData.cpp
    │   ├── Backtester.cpp
    │   ├── Metrics.cpp
    │   ├── Optimizer.cpp
    │   └── Report.cpp           ← NEW v1.0
    ├── strategy/
    │   ├── MovingAverageStrategy.cpp
    │   ├── RSIStrategy.cpp
    │   └── BollingerBandStrategy.cpp
    ├── portfolio/Portfolio.cpp
    └── utils/
        ├── Logger.cpp
        └── Config.cpp
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

## Usage

Pass a config file as the first argument (defaults to `config.ini`):

```bash
./bin/QuantTradingSystem config.ini
./bin/QuantTradingSystem my_experiment.ini
```

Session reports are automatically saved to `reports/session_YYYY-MM-DD_HH-MM.txt`.

## Config Reference

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

## Pipeline

```
config.ini
    │
    ▼
MarketData (CSV loader)
    │
    ├──► MovingAverageStrategy ──► Portfolio ──► BacktestResult ──► MetricsResult
    ├──► RSIStrategy           ──► Portfolio ──► BacktestResult ──► MetricsResult
    └──► BollingerBandStrategy ──► Portfolio ──► BacktestResult ──► MetricsResult
                                                                         │
                                                               Optimizer (grid search)
                                                                         │
                                                                      Report
                                                                  (reports/*.txt)
```

## Metrics Reference

| Metric | Formula | Interpretation |
|--------|---------|----------------|
| Sharpe Ratio | (return - risk_free) / std_dev × √252 | >1 good, >2 excellent |
| Max Drawdown | max((peak - trough) / peak) | Lower is better |
| Volatility | std_dev(daily_returns) × √252 | Annualized risk |
| Win Rate | winning_trades / total_trades | % profitable trades |
| Profit Factor | gross_profit / gross_loss | >1 means system profitable |

## Requirements
- CMake 3.16+
- C++17 compiler (MSVC 2019+, GCC 8+, Clang 10+)