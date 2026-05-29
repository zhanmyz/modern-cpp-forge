/**
 * @file example_thread_basics.cpp
 * @brief [Your Exercise] Thread Basics - Implement your own version here
 *
 * Reference: reference/concurrency/thread_basics.cpp
 *
 * [Exercise Objectives]
 * 1. Create a thread and run a simple task
 * 2. Pass arguments to a thread
 * 3. Create multiple threads and wait for all to complete
 *
 * Compare your implementation with the reference code in reference/
 * to see how it differs from the professional approach.
 */

#include <common/logger.h>
#include <common/timer.h>

#include <thread>
#include <vector>

// TODO: Implement your code here
// Hint: refer to reference/concurrency/thread_basics.cpp

int main() {
    LOG_INFO("========================================");
    LOG_INFO("  [Exercise] Thread Basics");
    LOG_INFO("========================================");

    // TODO: Implement the following:
    // 1. Create a thread that prints "Hello from thread!"
    // 2. Create a thread that takes an integer argument and prints it
    // 3. Create 4 threads, each computing a partial sum

    LOG_INFO("========================================");
    LOG_INFO("  Exercise complete! Compare with reference/ code");
    LOG_INFO("========================================");

    return 0;
}
