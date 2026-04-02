// include/utils/Logger.h
#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <sstream>

namespace trading {

// ─────────────────────────────────────────────
//  Log Levels
// ─────────────────────────────────────────────
enum class LogLevel {
    DEBUG   = 0,
    INFO    = 1,
    WARNING = 2,
    ERROR   = 3
};

// ─────────────────────────────────────────────
//  Logger — singleton, writes to file + terminal
// ─────────────────────────────────────────────
class Logger {
public:
    // Access the single global instance
    static Logger& instance();

    // Initialise once at startup — call from main()
    void init(const std::string& logFilePath,
              LogLevel           minLevel = LogLevel::DEBUG);

    // Core log method
    void log(LogLevel           level,
             const std::string& component,
             const std::string& message);

    // Convenience wrappers
    void debug  (const std::string& component, const std::string& msg);
    void info   (const std::string& component, const std::string& msg);
    void warning(const std::string& component, const std::string& msg);
    void error  (const std::string& component, const std::string& msg);

    // Change minimum level at runtime
    void setLevel(LogLevel level) { minLevel_ = level; }

    void close();

    // Non-copyable
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;
    ~Logger() { close(); }

    std::ofstream logFile_;
    LogLevel      minLevel_     = LogLevel::DEBUG;
    bool          initialised_  = false;
    std::mutex    mutex_;

    static std::string levelToString(LogLevel level);
    static std::string levelColor   (LogLevel level);
    static std::string timestamp();
};

} // namespace trading

// ─────────────────────────────────────────────
//  Convenience macros — use these everywhere
// ─────────────────────────────────────────────
#define LOG_DEBUG(component, msg)   trading::Logger::instance().debug(component, msg)
#define LOG_INFO(component, msg)    trading::Logger::instance().info(component, msg)
#define LOG_WARNING(component, msg) trading::Logger::instance().warning(component, msg)
#define LOG_ERROR(component, msg)   trading::Logger::instance().error(component, msg)