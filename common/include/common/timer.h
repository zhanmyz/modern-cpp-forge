#pragma once
/**
 * @file timer.h
 * @brief High-precision timer utility
 *
 * Measures code execution time with support for:
 * - Manual start/stop
 * - RAII-based automatic timing (ScopedTimer)
 */

#include <chrono>
#include <string>

#include "common/logger.h"

namespace common {

// ============================================================================
// Manual Timer
// ============================================================================
class Timer {
   public:
    Timer() : running_(false), elapsed_(0) {}

    void start() {
        start_time_ = std::chrono::high_resolution_clock::now();
        running_ = true;
    }

    void stop() {
        if (running_) {
            auto end_time = std::chrono::high_resolution_clock::now();
            elapsed_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time_)
                            .count();
            running_ = false;
        }
    }

    void reset() {
        running_ = false;
        elapsed_ = 0;
    }

    // Get elapsed time in milliseconds
    double elapsed_ms() const { return static_cast<double>(elapsed_) / 1'000'000.0; }

    // Get elapsed time in microseconds
    double elapsed_us() const { return static_cast<double>(elapsed_) / 1'000.0; }

    // Get elapsed time in nanoseconds
    int64_t elapsed_ns() const { return elapsed_; }

   private:
    std::chrono::high_resolution_clock::time_point start_time_;
    bool running_;
    int64_t elapsed_;
};

// ============================================================================
// RAII Scoped Timer - Starts on construction, logs elapsed time on destruction
// ============================================================================
class ScopedTimer {
   public:
    explicit ScopedTimer(const std::string& name) : name_(name) { timer_.start(); }

    ~ScopedTimer() {
        timer_.stop();
        LOG_INFO_FMT("[Timer] " << name_ << " took " << timer_.elapsed_ms() << " ms");
    }

    // Non-copyable, non-movable
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

   private:
    std::string name_;
    Timer timer_;
};

// Convenience macro - automatically times the current scope
#define SCOPED_TIMER(name) common::ScopedTimer _scoped_timer_##__LINE__(name)

}  // namespace common
