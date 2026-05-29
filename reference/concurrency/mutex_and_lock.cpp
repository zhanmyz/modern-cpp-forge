/**
 * @file mutex_and_lock.cpp
 * @brief Mutex & Locks - Protecting shared data in multithreaded programs
 *
 * [Learning Objectives]
 * 1. Understand why shared data access requires protection in multithreading
 * 2. Learn to use std::mutex and various lock types
 * 3. Understand deadlock and how to avoid it
 *
 * [Core Concept - Analogy]
 * Imagine a public restroom (shared resource) that multiple people (threads) want to use:
 * - mutex = the lock on the door
 * - lock() = locking the door (others must wait)
 * - unlock() = unlocking the door (next person can enter)
 * - lock_guard = auto-lock + auto-unlock (door unlocks automatically when you leave)
 *
 * [Why Locks in AI/ML?]
 * - Multiple inference threads sharing model weights need read-write locks
 * - Request queues require mutex protection
 * - Shared statistics counters need atomic operations or lock protection
 */

#include <common/logger.h>
#include <common/timer.h>

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

// ============================================================================
// Example 1: The danger of no locking - Race Condition
// ============================================================================
void example_race_condition() {
    LOG_INFO("=== Example 1: Race Condition Demo ===");

    int counter = 0;
    const int iterations = 100000;

    // Two threads increment counter simultaneously
    // No protection = non-deterministic result!
    auto increment = [&counter, iterations]() {
        for (int i = 0; i < iterations; ++i) {
            ++counter;  // Not atomic! read-modify-write can be interrupted
        }
    };

    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();

    // Expected: 200000, actual is typically less
    LOG_WARNING_FMT("  No lock - expected: " << 2 * iterations << ", actual: " << counter
                                             << " (may differ! This is a data race)");
}

// ============================================================================
// Example 2: Protecting shared data with std::mutex
// ============================================================================
void example_mutex_basic() {
    LOG_INFO("=== Example 2: Using mutex for protection ===");

    int counter = 0;
    std::mutex mtx;
    const int iterations = 100000;

    auto increment = [&counter, &mtx, iterations]() {
        for (int i = 0; i < iterations; ++i) {
            mtx.lock();    // Lock: other threads must wait
            ++counter;     // Safely modify shared data
            mtx.unlock();  // Unlock: allow other threads to proceed
        }
    };

    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();

    LOG_INFO_FMT("  With lock - expected: " << 2 * iterations << ", actual: " << counter
                                            << " (always equal!)");
}

// ============================================================================
// Example 3: std::lock_guard - RAII-style automatic lock (recommended)
// ============================================================================
void example_lock_guard() {
    LOG_INFO("=== Example 3: lock_guard automatic lock management ===");

    int counter = 0;
    std::mutex mtx;
    const int iterations = 100000;

    auto increment = [&counter, &mtx, iterations]() {
        for (int i = 0; i < iterations; ++i) {
            // lock_guard locks on construction, unlocks on destruction
            // Benefit: never forgets to unlock, even if exception is thrown
            std::lock_guard<std::mutex> lock(mtx);
            ++counter;
            // lock goes out of scope here, automatically unlocks
        }
    };

    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();

    LOG_INFO_FMT("  lock_guard result: " << counter);
}

// ============================================================================
// Example 4: std::unique_lock - More flexible locking
// ============================================================================
void example_unique_lock() {
    LOG_INFO("=== Example 4: unique_lock flexible locking ===");

    std::mutex mtx;
    int shared_data = 0;

    auto worker = [&mtx, &shared_data]() {
        // unique_lock is more flexible than lock_guard:
        // - Can manually lock/unlock
        // - Can defer locking
        // - Can be used with condition variables
        std::unique_lock<std::mutex> lock(mtx);
        shared_data = 42;
        LOG_INFO_FMT("  Set shared_data = " << shared_data);

        // Can unlock early (lock_guard cannot do this)
        lock.unlock();

        // Do work that doesn't need the lock...
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Re-acquire lock when needed
        lock.lock();
        shared_data += 8;
        LOG_INFO_FMT("  shared_data += 8 = " << shared_data);
        // Automatically unlocks on destruction
    };

    std::thread t(worker);
    t.join();
}

// ============================================================================
// Example 5: std::shared_mutex - Reader-Writer lock (multiple readers, single writer)
// ============================================================================
void example_shared_mutex() {
    LOG_INFO("=== Example 5: Reader-Writer Lock (shared_mutex) ===");

    // Scenario: AI model config is read frequently, updated rarely
    // Multiple threads can read simultaneously (shared_lock)
    // Only one thread can write (unique_lock), blocking all readers

    std::shared_mutex rw_mutex;
    int config_value = 100;

    // Read operation - multiple readers can access concurrently
    auto reader = [&rw_mutex, &config_value](int id) {
        std::shared_lock<std::shared_mutex> lock(rw_mutex);
        LOG_INFO_FMT("  Reader " << id << " read: " << config_value);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    // Write operation - exclusive access
    auto writer = [&rw_mutex, &config_value]() {
        std::unique_lock<std::shared_mutex> lock(rw_mutex);
        config_value = 200;
        LOG_INFO_FMT("  Writer updated to: " << config_value);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    };

    std::vector<std::thread> threads;
    threads.emplace_back(reader, 1);
    threads.emplace_back(reader, 2);
    threads.emplace_back(writer);
    threads.emplace_back(reader, 3);
    threads.emplace_back(reader, 4);

    for (auto& t : threads) {
        t.join();
    }
}

// ============================================================================
// Example 6: Avoiding Deadlock - std::scoped_lock
// ============================================================================
void example_avoid_deadlock() {
    LOG_INFO("=== Example 6: Avoiding Deadlock ===");

    // Deadlock = two threads waiting for each other to release locks
    // Thread 1: lock A -> wait for B
    // Thread 2: lock B -> wait for A
    // Result: infinite wait!

    std::mutex mutex_a, mutex_b;
    int resource_a = 0, resource_b = 0;

    // std::scoped_lock locks multiple mutexes simultaneously, avoiding deadlock
    auto worker1 = [&]() {
        // C++17: scoped_lock can lock multiple mutexes at once
        std::scoped_lock lock(mutex_a, mutex_b);
        resource_a = 1;
        resource_b = 2;
        LOG_INFO_FMT("  Worker1: a=" << resource_a << ", b=" << resource_b);
    };

    auto worker2 = [&]() {
        // Even with different lock order, scoped_lock prevents deadlock
        std::scoped_lock lock(mutex_b, mutex_a);
        resource_a = 10;
        resource_b = 20;
        LOG_INFO_FMT("  Worker2: a=" << resource_a << ", b=" << resource_b);
    };

    std::thread t1(worker1);
    std::thread t2(worker2);
    t1.join();
    t2.join();

    LOG_INFO("  Completed safely with scoped_lock, no deadlock!");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Mutex & Locks");
    LOG_INFO("========================================");

    example_race_condition();
    example_mutex_basic();
    example_lock_guard();
    example_unique_lock();
    example_shared_mutex();
    example_avoid_deadlock();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
