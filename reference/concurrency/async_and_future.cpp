/**
 * @file async_and_future.cpp
 * @brief Async Programming - std::async and std::future
 *
 * [Learning Objectives]
 * 1. Understand std::async for launching asynchronous tasks
 * 2. Learn different launch policies (async, deferred)
 * 3. Master std::future, std::promise, std::shared_future
 *
 * [Core Concept - Analogy]
 * std::async is like ordering food delivery:
 * - You place the order (launch async task) and continue doing other things
 * - The future is your order receipt - you can check status or wait for delivery
 * - When you need the food (result), you call get() on the future
 *
 * [Applications in AI/ML]
 * - Async model loading: load model in background while UI remains responsive
 * - Parallel preprocessing: launch multiple data transforms concurrently
 * - Speculative execution: start multiple inference paths, use first result
 */

#include <common/logger.h>
#include <common/timer.h>

#include <chrono>
#include <future>
#include <numeric>
#include <thread>
#include <vector>

// ============================================================================
// Example 1: std::async basics
// ============================================================================
void example_async_basic() {
    LOG_INFO("=== Example 1: std::async Basics ===");

    // Launch an async task - may run in a new thread or deferred
    auto future = std::async(std::launch::async, []() {
        LOG_INFO("  [async task] Computing...");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 42;
    });

    LOG_INFO("  [main] Doing other work while async task runs...");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // get() blocks until result is ready
    int result = future.get();
    LOG_INFO_FMT("  [main] Async result: " << result);
}

// ============================================================================
// Example 2: Launch policies
// ============================================================================
void example_launch_policies() {
    LOG_INFO("=== Example 2: Launch Policies ===");

    // std::launch::async - guaranteed new thread
    auto f1 = std::async(std::launch::async, []() {
        LOG_INFO_FMT("  [async] Thread ID: " << std::this_thread::get_id());
        return 1;
    });

    // std::launch::deferred - lazy evaluation, runs when get() is called
    auto f2 = std::async(std::launch::deferred, []() {
        LOG_INFO_FMT("  [deferred] Thread ID: " << std::this_thread::get_id());
        return 2;
    });

    LOG_INFO_FMT("  [main] Thread ID: " << std::this_thread::get_id());
    LOG_INFO_FMT("  f1 result: " << f1.get());
    LOG_INFO("  (f2 executes now, on get() call)");
    LOG_INFO_FMT("  f2 result: " << f2.get());
}

// ============================================================================
// Example 3: Parallel computation with async
// ============================================================================
void example_parallel_compute() {
    LOG_INFO("=== Example 3: Parallel Computation ===");

    std::vector<int> data(1000000);
    std::iota(data.begin(), data.end(), 1);

    // Sequential sum
    long long sequential_sum = 0;
    {
        SCOPED_TIMER("sequential sum");
        for (int v : data) sequential_sum += v;
    }

    // Parallel sum using async
    long long parallel_sum = 0;
    {
        SCOPED_TIMER("parallel sum (4 tasks)");
        size_t chunk = data.size() / 4;

        auto sum_range = [&data](size_t begin, size_t end) {
            long long s = 0;
            for (size_t i = begin; i < end; ++i) s += data[i];
            return s;
        };

        auto f1 = std::async(std::launch::async, sum_range, 0, chunk);
        auto f2 = std::async(std::launch::async, sum_range, chunk, 2 * chunk);
        auto f3 = std::async(std::launch::async, sum_range, 2 * chunk, 3 * chunk);
        auto f4 = std::async(std::launch::async, sum_range, 3 * chunk, data.size());

        parallel_sum = f1.get() + f2.get() + f3.get() + f4.get();
    }

    LOG_INFO_FMT("  Sequential: " << sequential_sum << ", Parallel: " << parallel_sum);
}

// ============================================================================
// Example 4: std::promise for explicit value setting
// ============================================================================
void example_promise() {
    LOG_INFO("=== Example 4: std::promise ===");

    std::promise<std::string> promise;
    std::future<std::string> future = promise.get_future();

    // Worker thread sets the promise value
    std::thread worker([&promise]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        promise.set_value("Data from worker thread");
    });

    LOG_INFO("  [main] Waiting for promise...");
    std::string result = future.get();
    LOG_INFO_FMT("  [main] Got: " << result);

    worker.join();
}

// ============================================================================
// Example 5: Exception propagation through futures
// ============================================================================
void example_exception_handling() {
    LOG_INFO("=== Example 5: Exception Propagation ===");

    auto future = std::async(std::launch::async, []() -> int {
        throw std::runtime_error("Something went wrong in async task");
        return 0;
    });

    try {
        future.get();  // Re-throws the exception from the async task
    } catch (const std::exception& e) {
        LOG_ERROR_FMT("  Caught exception: " << e.what());
    }
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Async & Future");
    LOG_INFO("========================================");

    example_async_basic();
    example_launch_policies();
    example_parallel_compute();
    example_promise();
    example_exception_handling();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
