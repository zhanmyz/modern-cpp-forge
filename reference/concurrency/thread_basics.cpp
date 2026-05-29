/**
 * @file thread_basics.cpp
 * @brief C++ Thread Basics - Creating, managing, and synchronizing threads
 *
 * [Learning Objectives]
 * 1. Understand what threads are and why multithreading is needed
 * 2. Learn to create and manage threads with std::thread
 * 3. Understand the difference between join() and detach()
 * 4. Learn to pass arguments to threads
 *
 * [Core Concept]
 * Thread = an independent path of execution within a program.
 * Imagine cooking: you can boil water (thread 1) and chop vegetables (thread 2)
 * simultaneously, rather than sequentially. This is "concurrency."
 *
 * [Why Multithreading in AI/ML?]
 * - During inference, one thread handles requests while another loads data
 * - Multiple requests can be processed in parallel for higher throughput
 * - CPU can do preprocessing while GPU performs computation
 */

#include <common/logger.h>
#include <common/timer.h>

#include <functional>
#include <thread>
#include <vector>

// ============================================================================
// Example 1: Simplest thread creation
// ============================================================================
void example_basic_thread() {
    LOG_INFO("=== Example 1: Basic Thread Creation ===");

    // Create a thread executing a lambda function
    // lambda = anonymous function: [captures](params){body}
    std::thread t1([]() {
        LOG_INFO("  Hello! I'm running in a separate thread!");
        LOG_INFO_FMT("  My thread ID: " << std::this_thread::get_id());
    });

    LOG_INFO_FMT("Main thread ID: " << std::this_thread::get_id());

    // join() = wait for the thread to finish
    // Like asking a friend to buy something - you wait at the door until they return.
    // If you neither join nor detach, the program will crash!
    t1.join();

    LOG_INFO("Thread t1 completed");
}

// ============================================================================
// Example 2: Passing arguments to threads
// ============================================================================
void example_thread_with_args() {
    LOG_INFO("=== Example 2: Passing Arguments to Threads ===");

    // Approach 1: Pass arguments directly (by value)
    auto print_number = [](int n, const std::string& prefix) {
        LOG_INFO_FMT("  " << prefix << ": " << n);
    };

    std::thread t1(print_number, 42, "number");
    t1.join();

    // Approach 2: Pass by reference (must wrap with std::ref!)
    // Note: without std::ref, the thread receives a copy, not a reference
    int counter = 0;
    auto increment = [](int& c, int times) {
        for (int i = 0; i < times; ++i) {
            ++c;
        }
    };

    std::thread t2(increment, std::ref(counter), 1000);
    t2.join();

    LOG_INFO_FMT("  counter final value: " << counter << " (should be 1000)");
}

// ============================================================================
// Example 3: join() vs detach()
// ============================================================================
void example_join_vs_detach() {
    LOG_INFO("=== Example 3: join() vs detach() ===");

    // join() - blocks until thread completes
    // Use case: you need the thread's result, or must ensure completion before proceeding
    std::thread t1([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOG_INFO("  [joined thread] Done! Main thread was waiting for me");
    });
    t1.join();  // Main thread pauses here until t1 completes
    LOG_INFO("  Main thread: t1 finished, continuing execution");

    // detach() - let thread run independently ("fire and forget")
    // Use case: background tasks where you don't care when they finish
    // Warning: accessing destroyed variables from a detached thread = UB!
    std::thread t2([]() { LOG_INFO("  [detached thread] Running independently"); });
    t2.detach();  // Thread runs independently, main thread moves on

    // Give detached thread time to complete (don't do this in production)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

// ============================================================================
// Example 4: Creating multiple threads and waiting for all
// ============================================================================
void example_multiple_threads() {
    LOG_INFO("=== Example 4: Parallel Multi-threaded Work ===");

    SCOPED_TIMER("parallel computation");

    const int num_threads = 4;
    std::vector<std::thread> threads;
    std::vector<int> results(static_cast<size_t>(num_threads), 0);

    // Create multiple threads, each computing a portion of the task
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, &results]() {
            int sum = 0;
            for (int j = i * 1000; j < (i + 1) * 1000; ++j) {
                sum += j;
            }
            results[static_cast<size_t>(i)] = sum;
            LOG_INFO_FMT("  Thread " << i << " done, result: " << sum);
        });
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    // Aggregate results
    int total = 0;
    for (int r : results) {
        total += r;
    }
    LOG_INFO_FMT("  Total sum across all threads: " << total);
}

// ============================================================================
// Example 5: Move semantics with threads (non-copyable, move-only)
// ============================================================================
void example_thread_move() {
    LOG_INFO("=== Example 5: Thread Move Semantics ===");

    std::thread t1([]() { LOG_INFO("  I am the moved thread"); });

    // Threads cannot be copied: std::thread t2 = t1;  // compile error!
    // But they can be moved:
    std::thread t2 = std::move(t1);

    // Now t1 no longer represents any thread (it's "empty")
    LOG_INFO_FMT("  t1 joinable: " << (t1.joinable() ? "yes" : "no"));
    LOG_INFO_FMT("  t2 joinable: " << (t2.joinable() ? "yes" : "no"));

    t2.join();
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  C++ Thread Basics");
    LOG_INFO("========================================");

    example_basic_thread();
    example_thread_with_args();
    example_join_vs_detach();
    example_multiple_threads();
    example_thread_move();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
