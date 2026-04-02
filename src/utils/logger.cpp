// src/utils/Logger.cpp
#include "utils/Logger.h"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <stdexcept>

namespace trading {

// ─────────────────────────────────────────────
//  Singleton instance
// ─────────────────────────────────────────────
Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

// ─────────────────────────────────────────────
//  init() — open log file, set level
// ─────────────────────────────────────────────
void Logger::init(const std::string& logFilePath, LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(mutex_);

    minLevel_ = minLevel;

    logFile_.open(logFilePath, std::ios::out | std::ios::app);
    if (!logFile_.is_open()) {
        throw std::runtime_error(
            "[Logger] Cannot open log file: " + logFilePath
        );
    }

    initialised_ = true;

    // Opening separator
    logFile_ << "\n════════════════════════════════════════════════\n";
    logFile_ << "  Session started: " << timestamp() << "\n";
    logFile_ << "════════════════════════════════════════════════\n";
    logFile_.flush();

    std::cout << "\n[Logger] Logging to: " << logFilePath << "\n\n";
}

// ─────────────────────────────────────────────
//  log() — core method
// ─────────────────────────────────────────────
void Logger::log(LogLevel level, const std::string& component, const std::string& message) {
    if (level < minLevel_) return;

    std::lock_guard<std::mutex> lock(mutex_);

    const std::string ts    = timestamp();
    const std::string lvl   = levelToString(level);
    const std::string color = levelColor(level);
    const std::string reset = "\033[0m";

    // ── Format ──────────────────────────────────────────────
    // [2024-01-02 13:45:01] [INFO ]  [MarketData] Loaded 20 bars
    std::ostringstream line;
    line << "[" << ts << "]"
         << " [" << lvl << "] "
         << " [" << component << "] "
         << message;

    // ── Write to file (no color codes) ──────────────────────
    if (initialised_ && logFile_.is_open()) {
        logFile_ << line.str() << "\n";
        logFile_.flush();
    }

    // ── Write to terminal (with color) ──────────────────────
    std::ostream& out = (level == LogLevel::ERROR) ? std::cerr : std::cout;
    out << color << line.str() << reset << "\n";
}

// ─────────────────────────────────────────────
//  Convenience wrappers
// ─────────────────────────────────────────────
void Logger::debug  (const std::string& c, const std::string& m) { log(LogLevel::DEBUG,   c, m); }
void Logger::info   (const std::string& c, const std::string& m) { log(LogLevel::INFO,    c, m); }
void Logger::warning(const std::string& c, const std::string& m) { log(LogLevel::WARNING, c, m); }
void Logger::error  (const std::string& c, const std::string& m) { log(LogLevel::ERROR,   c, m); }

// ─────────────────────────────────────────────
//  close()
// ─────────────────────────────────────────────
void Logger::close() {
    if (logFile_.is_open()) {
        logFile_ << "  Session ended:   " << timestamp() << "\n";
        logFile_ << "════════════════════════════════════════════════\n";
        logFile_.close();
    }
}

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO ";
        case LogLevel::WARNING: return "WARN ";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "?????";
    }
}

std::string Logger::levelColor(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "\033[37m";   // white
        case LogLevel::INFO:    return "\033[36m";   // cyan
        case LogLevel::WARNING: return "\033[33m";   // yellow
        case LogLevel::ERROR:   return "\033[31m";   // red
        default:                return "\033[0m";
    }
}

std::string Logger::timestamp() {
    auto now     = std::chrono::system_clock::now();
    auto time_t  = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};

#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace trading