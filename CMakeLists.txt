cmake_minimum_required(VERSION 3.16)
project(QuantTradingSystem VERSION 0.8.0 LANGUAGES CXX)

# ── C++ Standard ──────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build type default ────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ── Compiler warnings ─────────────────────────────────────
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── Include paths ─────────────────────────────────────────
include_directories(include)

# ── Source files ──────────────────────────────────────────
# Uncomment each line as the version is implemented
set(CORE_SOURCES
    src/core/MarketData.cpp
    src/utils/Logger.cpp
    src/core/Backtester.cpp         
    src/core/Metrics.cpp     
    src/core/Optimizer.cpp
)

set(STRATEGY_SOURCES
    src/strategy/MovingAverageStrategy.cpp   
    src/strategy/RSIStrategy.cpp
    src/strategy/BollingerBandStrategy.cpp
)

set(PORTFOLIO_SOURCES
    src/portfolio/Portfolio.cpp      
)

# ── Core static library ───────────────────────────────────
add_library(trading_core STATIC
    ${CORE_SOURCES}
    ${STRATEGY_SOURCES}
    ${PORTFOLIO_SOURCES}
)

target_include_directories(trading_core PUBLIC include)

# ── Main executable ───────────────────────────────────────
add_executable(QuantTradingSystem app/main.cpp)
target_link_libraries(QuantTradingSystem PRIVATE trading_core)

# ── Output binary to bin/ ─────────────────────────────────
set_target_properties(QuantTradingSystem PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ── Status message ────────────────────────────────────────
message(STATUS "QuantTradingSystem v${PROJECT_VERSION} | Build: ${CMAKE_BUILD_TYPE}")