
// Reference: https://en.cppreference.com/w/cpp/thread/future
/**
 * @file future_test.cpp
 * @brief Basic usage of std::future and std::promise
 *
 * The class template std::future provides a mechanism to access the result of asynchronous
 * operations. An asynchronous operation (via std::async, std::packaged_task, or std::promise)
 * can provide a std::future object to the creator of that asynchronous operation.
 * The creator can then query, wait for, or extract a value from the std::future.
 *
 * [Learning Objectives]
 * 1. Understand the relationship between std::future and std::promise
 * 2. Use std::async to create asynchronous tasks and retrieve results
 * 3. Understand future/promise use cases in multithreaded applications
 *
 * [Core Concept - Analogy]
 * std::promise is like making a promise to deliver a result later.
 * std::future is the recipient's handle to wait for and retrieve that result.
 * One thread performs work and sets the value via promise;
 * another thread calls future.get() to obtain it.
 *
 * [Applications in AI/ML]
 * - Model loading: main thread initiates, worker thread notifies on completion
 * - Async inference: submit inference tasks, retrieve results later
 * - Data preprocessing: process samples asynchronously while main thread continues
 */
#include <common/logger.h>
#include <common/timer.h>

#include <future>

using namespace std;

int main(int argc, char* argv[]) {
    // =================================== 1. future from a packaged_task ========================================
    LOG_INFO("=== Example 1: std::packaged_task + std::future ===");

    // Create a packaged_task wrapping a simple function
    std::packaged_task<int(int, int)> task([](int a, int b) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));  // simulate heavy work
        return a * b;
    });

    // Get the future to retrieve the result later
    std::future<int> future_int = task.get_future();

    // Execute the task in another thread
    std::thread t(std::move(task), 8, 9);

    // ======================================= 2. future from async() =========================================
    LOG_INFO("=== Example 2: std::async + std::future ===");

    // Launch an asynchronous task
    std::future<std::string> future_str = std::async([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(6000));  // simulate heavy work
        return std::string("Hello from async!");
    });

    // ====================================== 3. future from promise ========================================
    LOG_INFO("=== Example 3: std::promise + std::future ===");

    std::promise<double> promise;
    std::future<double> future_value = promise.get_future();

    // Set the promise value from another thread
    std::thread([&promise]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(9000));  // simulate heavy work
        double pi = 3.14159;
        promise.set_value_at_thread_exit(pi);
    }).detach();

    // ====================================== 4. Retrieve results ===============================================
    LOG_INFO("Waiting for results...");
    future_int.wait();  // Block until result is available (optional, get() blocks implicitly)
    LOG_INFO("[READY] packaged_task result is ready!");
    future_str.wait();
    LOG_INFO("[READY] async result is ready!");
    future_value.wait();
    LOG_INFO("[READY] promise result is ready!");

    LOG_INFO_FMT("Results: " << future_int.get() << ' ' << future_str.get() << ' '
                             << future_value.get());  // Expected: 72, "Hello from async!", 3.14159

    t.join();

    return 0;
}
