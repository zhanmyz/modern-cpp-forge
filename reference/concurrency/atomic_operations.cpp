/**
 * @file atomic_operations.cpp
 * @brief Atomic Operations - Lock-free concurrent programming
 *
 * [Learning Objectives]
 * 1. Understand atomic operations and memory ordering
 * 2. Learn std::atomic usage for lock-free data structures
 * 3. Understand compare-and-swap (CAS) operations
 * 4. Compare performance: atomic vs mutex
 *
 * [Core Concept - Analogy]
 * Atomic operations are like "instant transactions":
 * - Normal operations (read-modify-write) can be interrupted mid-way
 * - Atomic operations are indivisible - they either complete fully or not at all
 * - Like a bank transfer that either succeeds completely or doesn't happen
 *
 * [Applications in AI/ML]
 * - Lock-free counters for request statistics
 * - Atomic flags for graceful shutdown signals
 * - Reference counting in shared model instances
 * - Lock-free queues for high-performance inference pipelines
 */

#include <common/logger.h>
#include <common/timer.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

// ============================================================================
// Example 1: Basic atomic operations
// ============================================================================
void example_basic_atomic() {
    LOG_INFO("=== Example 1: Basic Atomic Operations ===");

    std::atomic<int> counter{0};
    const int iterations = 100000;

    // Multiple threads incrementing atomically - no mutex needed!
    auto increment = [&counter, iterations]() {
        for (int i = 0; i < iterations; ++i) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();
}

// ============================================================================
// Example 2: Compare-And-Swap (CAS) - Foundation of lock-free algorithms
// ============================================================================
void example_cas() {
    LOG_INFO("=== Example 2: Compare-And-Swap (CAS) ===");

    std::atomic<int> value{100};

    // CAS: only update if current value matches expected
    int expected = 100;
    bool success = value.compare_exchange_strong(expected, 200);
    LOG_INFO_FMT("  CAS(100->200): " << (success ? "success" : "failed")
                                     << ", value=" << value.load());

    // Second CAS will fail because value is now 200, not 100
    expected = 100;
    success = value.compare_exchange_strong(expected, 300);
    LOG_INFO_FMT("  CAS(100->300): " << (success ? "success" : "failed") << ", value="
                                     << value.load() << ", expected updated to=" << expected);
}

// ============================================================================
// Example 3: Atomic flag - Lightweight synchronization primitive
// ============================================================================
void example_atomic_flag() {
    LOG_INFO("=== Example 3: Atomic Flag (Spinlock) ===");

    std::atomic_flag lock = ATOMIC_FLAG_INIT;
    int shared_counter = 0;
    const int iterations = 100000;

    // Simple spinlock using atomic_flag
    auto worker = [&](int /*id*/) {
        for (int i = 0; i < iterations; ++i) {
            // Spin until we acquire the lock
            while (lock.test_and_set(std::memory_order_acquire)) {
                // Busy wait (spin)
            }
            ++shared_counter;                       // Critical section
            lock.clear(std::memory_order_release);  // Release the lock
        }
    };

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    t1.join();
    t2.join();

    LOG_INFO_FMT("  Spinlock counter: " << shared_counter << " (expected: " << 2 * iterations
                                        << ")");
}

// ============================================================================
// Example 4: Memory ordering
// ============================================================================
void example_memory_order() {
    LOG_INFO("=== Example 4: Memory Ordering ===");

    std::atomic<bool> ready{false};
    std::atomic<int> data{0};

    // Producer: write data, then signal ready
    std::thread producer([&]() {
        data.store(42, std::memory_order_relaxed);
        ready.store(true, std::memory_order_release);  // Release: all prior writes visible
    });

    // Consumer: wait for ready, then read data
    std::thread consumer([&]() {
        while (!ready.load(std::memory_order_acquire)) {
            // Spin wait
        }
        LOG_INFO_FMT("  Consumer read data: " << data.load(std::memory_order_relaxed));
    });

    producer.join();
    consumer.join();
}

// ============================================================================
// Example 5: Performance comparison - Atomic vs Mutex
// ============================================================================
void example_performance_comparison() {
    LOG_INFO("=== Example 5: Performance - Atomic vs Mutex ===");

    const int iterations = 1000000;
    const int num_threads = 4;

    // Approach 1: std::mutex
    {
        SCOPED_TIMER("mutex counter");
        int counter = 0;
        std::mutex mtx;
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < iterations / num_threads; ++j) {
                    std::lock_guard<std::mutex> lock(mtx);
                    ++counter;
                }
            });
        }
        for (auto& t : threads)
            t.join();
    }

    // Approach 2: std::atomic
    {
        SCOPED_TIMER("atomic counter");
        std::atomic<int> counter{0};
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < iterations / num_threads; ++j) {
                    counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        for (auto& t : threads)
            t.join();
    }
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Atomic Operations");
    LOG_INFO("========================================");

    example_basic_atomic();
    example_cas();
    example_atomic_flag();
    example_memory_order();
    example_performance_comparison();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
