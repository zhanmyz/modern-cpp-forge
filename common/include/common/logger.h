#pragma once
/**
 * @file logger.h
 * @brief High-performance thread-safe logging system
 *
 * Features:
 * - Supports DEBUG, INFO, WARNING, ERROR, FATAL log levels
 * - Thread-safe (mutex-protected)
 * - Format: [timestamp] [level] [thread_id] [file:line] message
 * - Colored terminal output
 * - File output support
 * - Compile-time level filtering (zero overhead)
 */

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace common {

// ============================================================================
// Log Level Enumeration
// ============================================================================
enum class LogLevel : uint8_t {
    DEBUG = 0,    // Debug info, development only
    INFO = 1,     // General info, key steps during normal execution
    WARNING = 2,  // Warnings, non-critical but noteworthy
    ERROR = 3,    // Errors, operation failed
    FATAL = 4     // Fatal errors, program cannot continue
};

// ============================================================================
// Log Format Flags — control which fields appear in output
// ============================================================================
// Usage: Logger::instance().set_format(LogFormat::Level | LogFormat::Message);
//        This outputs only: "[INFO ] Hello world"
//
// Default: All fields enabled (timestamp + level + thread + location + message)
//
// Common presets:
//   kFull     = all fields (for file logging / debugging)
//   kCompact  = level + message only (clean console output)
//   kDefault  = timestamp + level + location + message (no thread id)
namespace LogFormat {
constexpr uint8_t Timestamp = 1 << 0;  // [2024-01-15 10:30:45.123]
constexpr uint8_t Level = 1 << 1;      // [INFO ]
constexpr uint8_t ThreadId = 1 << 2;   // [tid:12345]
constexpr uint8_t Location = 1 << 3;   // [file.cpp:42]
constexpr uint8_t Message = 1 << 4;    // always recommended

constexpr uint8_t kFull = Timestamp | Level | ThreadId | Location | Message;
constexpr uint8_t kDefault = Timestamp | Level | Location | Message;
constexpr uint8_t kCompact = Level | Message;
}  // namespace LogFormat

// ============================================================================
// Logger Singleton Class
// ============================================================================
class Logger {
   public:
    // Get the global singleton instance
    static Logger& instance();

    // Set minimum log level (messages below this level are suppressed)
    void set_level(LogLevel level);

    // Set log output file (outputs to both terminal and file)
    void set_output_file(const std::string& filepath);

    // Enable/disable colored output
    void set_color_enabled(bool enabled);

    // Set format flags (bitwise OR of LogFormat:: values)
    // Example: set_format(LogFormat::Level | LogFormat::Message);
    void set_format(uint8_t flags);

    // Core logging function
    void log(LogLevel level, const std::string& file, int line, const std::string& message);

    // Non-copyable, non-movable
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

   private:
    Logger();
    ~Logger();

    std::string level_to_string(LogLevel level) const;
    std::string level_to_color(LogLevel level) const;
    std::string get_timestamp() const;
    std::string get_thread_id() const;
    std::string extract_filename(const std::string& filepath) const;

    LogLevel min_level_;
    bool color_enabled_;
    uint8_t format_flags_;
    std::mutex mutex_;
    std::ofstream file_stream_;
};

// ============================================================================
// Convenience Macros - Automatically capture file name and line number
// ============================================================================
#define LOG_DEBUG(msg) \
    common::Logger::instance().log(common::LogLevel::DEBUG, __FILE__, __LINE__, msg)

#define LOG_INFO(msg) \
    common::Logger::instance().log(common::LogLevel::INFO, __FILE__, __LINE__, msg)

#define LOG_WARNING(msg) \
    common::Logger::instance().log(common::LogLevel::WARNING, __FILE__, __LINE__, msg)

#define LOG_ERROR(msg) \
    common::Logger::instance().log(common::LogLevel::ERROR, __FILE__, __LINE__, msg)

#define LOG_FATAL(msg) \
    common::Logger::instance().log(common::LogLevel::FATAL, __FILE__, __LINE__, msg)

// Formatted logging macros (using ostringstream)
#define LOG_DEBUG_FMT(expr)      \
    do {                         \
        std::ostringstream oss_; \
        oss_ << expr;            \
        LOG_DEBUG(oss_.str());   \
    } while (0)

#define LOG_INFO_FMT(expr)       \
    do {                         \
        std::ostringstream oss_; \
        oss_ << expr;            \
        LOG_INFO(oss_.str());    \
    } while (0)

#define LOG_WARNING_FMT(expr)    \
    do {                         \
        std::ostringstream oss_; \
        oss_ << expr;            \
        LOG_WARNING(oss_.str()); \
    } while (0)

#define LOG_ERROR_FMT(expr)      \
    do {                         \
        std::ostringstream oss_; \
        oss_ << expr;            \
        LOG_ERROR(oss_.str());   \
    } while (0)

#define LOG_FATAL_FMT(expr)      \
    do {                         \
        std::ostringstream oss_; \
        oss_ << expr;            \
        LOG_FATAL(oss_.str());   \
    } while (0)

}  // namespace common
