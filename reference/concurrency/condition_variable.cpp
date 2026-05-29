/**
 * @file condition_variable.cpp
 * @brief Condition Variables - Inter-thread notification mechanism
 *
 * [Learning Objectives]
 * 1. Understand condition variables: efficiently wait for a condition to become true
 * 2. Learn the usage of wait/notify_one/notify_all
 * 3. Implement the classic Producer-Consumer pattern
 *
 * [Core Concept - Analogy]
 * Imagine waiting for a table at a restaurant:
 * - You (consumer thread) sit in the waiting area (wait)
 * - When a table is free, the host (producer thread) calls your number (notify_one)
 * - You don't need to ask "is there a table?" every second; you wait for notification
 * This is much more efficient than continuously polling (busy waiting)!
 *
 * [Applications in AI/ML]
 * - Inference request queue: notify worker threads when requests arrive
 * - Data loading pipeline: notify training thread when data is ready
 * - Model hot-swap: notify all inference threads when new model is loaded
 */

#include <common/logger.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// ============================================================================
// Example 1: Basic condition variable usage
// ============================================================================
void example_basic_cv() {
    LOG_INFO("=== Example 1: Condition Variable Basics ===");

    std::mutex mtx;
    std::condition_variable cv;
    bool data_ready = false;
    std::string data;

    // Consumer thread - waits for data
    std::thread consumer([&]() {
        LOG_INFO("  [Consumer] Waiting for data...");

        std::unique_lock<std::mutex> lock(mtx);
        // wait() does three things:
        // 1. Checks condition (lambda return value); if true, doesn't wait
        // 2. If false, releases lock and suspends thread (no CPU usage)
        // 3. When woken, re-acquires lock and re-checks condition
        cv.wait(lock, [&data_ready]() { return data_ready; });

        LOG_INFO_FMT("  [Consumer] Received data: " << data);
    });

    // Producer thread - prepares data then notifies
    std::thread producer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        {
            std::lock_guard<std::mutex> lock(mtx);
            data = "Hello from producer!";
            data_ready = true;
            LOG_INFO("  [Producer] Data ready, notifying consumer");
        }
        // Note: calling notify after unlock is more efficient
        cv.notify_one();  // Wake up one waiting thread
    });

    producer.join();
    consumer.join();
}

// ============================================================================
// Example 2: Producer-Consumer pattern (foundation for AI inference services)
// ============================================================================
void example_producer_consumer() {
    LOG_INFO("=== Example 2: Producer-Consumer Pattern ===");

    std::mutex mtx;
    std::condition_variable cv_not_empty;  // Condition: queue is not empty
    std::condition_variable cv_not_full;   // Condition: queue is not full
    std::queue<int> buffer;
    const size_t max_buffer_size = 5;
    bool done = false;

    // Producer - generates data and puts it into the buffer
    std::thread producer([&]() {
        for (int i = 1; i <= 10; ++i) {
            std::unique_lock<std::mutex> lock(mtx);

            // Wait until buffer has space
            cv_not_full.wait(lock, [&]() { return buffer.size() < max_buffer_size; });

            buffer.push(i);
            LOG_INFO_FMT("  [Produce] Enqueued: " << i << " (queue size: " << buffer.size() << ")");
            lock.unlock();
            cv_not_empty.notify_one();  // Notify consumer: data available

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        // Production complete
        {
            std::lock_guard<std::mutex> lock(mtx);
            done = true;
        }
        cv_not_empty.notify_all();
    });

    // Consumer - takes data from buffer and processes it
    std::thread consumer([&]() {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx);

            // Wait until queue is non-empty or producer is done
            cv_not_empty.wait(lock, [&]() { return !buffer.empty() || done; });

            if (buffer.empty() && done) {
                break;  // Producer done and queue empty, exit
            }

            int item = buffer.front();
            buffer.pop();
            LOG_INFO_FMT("  [Consume] Dequeued: " << item << " (queue size: " << buffer.size() << ")");
            lock.unlock();
            cv_not_full.notify_one();  // Notify producer: space available

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    producer.join();
    consumer.join();
    LOG_INFO("  Producer-Consumer pattern complete");
}

// ============================================================================
// Example 3: notify_all - Broadcast to all waiting threads
// ============================================================================
void example_notify_all() {
    LOG_INFO("=== Example 3: notify_all Broadcast ===");

    std::mutex mtx;
    std::condition_variable cv;
    bool start_signal = false;

    // Simulate a race: all runners wait for the starting gun
    auto runner = [&](int id) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&start_signal]() { return start_signal; });
        LOG_INFO_FMT("  Runner " << id << " starts!");
    };

    std::vector<std::thread> runners;
    for (int i = 1; i <= 5; ++i) {
        runners.emplace_back(runner, i);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    LOG_INFO("  Referee: Ready...");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    {
        std::lock_guard<std::mutex> lock(mtx);
        start_signal = true;
    }
    LOG_INFO("  Referee: GO! (notify_all)");
    cv.notify_all();  // Wake up all waiting threads simultaneously

    for (auto& t : runners) {
        t.join();
    }
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Condition Variables");
    LOG_INFO("========================================");

    example_basic_cv();
    example_producer_consumer();
    example_notify_all();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
