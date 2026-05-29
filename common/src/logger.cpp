#include "common/logger.h"

#include <ctime>
#include <filesystem>

namespace common {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::Logger()
    : min_level_(LogLevel::DEBUG), color_enabled_(true), format_flags_(LogFormat::kFull) {}

Logger::~Logger() {
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
}

void Logger::set_level(LogLevel level) {
    min_level_ = level;
}

void Logger::set_output_file(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
    file_stream_.open(filepath, std::ios::app);
    if (!file_stream_.is_open()) {
        std::cerr << "[Logger] Failed to open log file: " << filepath << "\n";
    }
}

void Logger::set_color_enabled(bool enabled) {
    color_enabled_ = enabled;
}

void Logger::set_format(uint8_t flags) {
    format_flags_ = flags;
}

void Logger::log(LogLevel level, const std::string& file, int line, const std::string& message) {
    if (level < min_level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // Build formatted output based on enabled flags
    std::ostringstream plain_oss;

    if (format_flags_ & LogFormat::Timestamp) {
        plain_oss << "[" << get_timestamp() << "] ";
    }
    if (format_flags_ & LogFormat::Level) {
        plain_oss << "[" << level_to_string(level) << "] ";
    }
    if (format_flags_ & LogFormat::ThreadId) {
        plain_oss << "[tid:" << get_thread_id() << "] ";
    }
    if (format_flags_ & LogFormat::Location) {
        plain_oss << "[" << extract_filename(file) << ":" << line << "] ";
    }
    if (format_flags_ & LogFormat::Message) {
        plain_oss << message;
    }

    // Colored terminal output
    if (color_enabled_) {
        std::string color = level_to_color(level);
        std::string reset = "\033[0m";
        std::cerr << color << plain_oss.str() << reset << "\n";
    } else {
        std::cerr << plain_oss.str() << "\n";
    }

    // File output (no color codes)
    if (file_stream_.is_open()) {
        file_stream_ << plain_oss.str() << "\n";
        file_stream_.flush();
    }
}

std::string Logger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO ";
        case LogLevel::WARNING:
            return "WARN ";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::FATAL:
            return "FATAL";
        default:
            return "?????";
    }
}

std::string Logger::level_to_color(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:
            return "\033[36m";  // Cyan
        case LogLevel::INFO:
            return "\033[32m";  // Green
        case LogLevel::WARNING:
            return "\033[33m";  // Yellow
        case LogLevel::ERROR:
            return "\033[31m";  // Red
        case LogLevel::FATAL:
            return "\033[35;1m";  // Bold Magenta
        default:
            return "\033[0m";
    }
}

std::string Logger::get_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_buf{};
    localtime_r(&time_t_now, &tm_buf);

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3)
        << ms.count();
    return oss.str();
}

std::string Logger::get_thread_id() const {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

std::string Logger::extract_filename(const std::string& filepath) const {
    std::filesystem::path p(filepath);
    return p.filename().string();
}

}  // namespace common
