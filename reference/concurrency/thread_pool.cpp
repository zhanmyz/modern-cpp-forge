/**
 * @file thread_pool.cpp
 * @brief Thread Pool - Efficient thread management and reuse
 *
 * [Learning Objectives]
 * 1. Understand thread pool design principles and benefits
 * 2. Implement a complete thread pool
 * 3. Learn to use std::future to retrieve async results
 *
 * [Core Concept - Analogy]
 * Imagine a courier company:
 * - Hiring a new courier for each package, then firing them = too expensive (thread creation overhead)
 * - Better approach: keep a fixed team of couriers (thread pool) taking packages from a queue
 * - When idle, couriers rest and wait (condition_variable wait); woken when new packages arrive
 *
 * [Applications in AI/ML]
 * - LLM inference service: fixed worker threads handling user requests
 * - Data preprocessing pipeline: thread pool processes multiple samples in parallel
 * - Batch inference: distribute large batches across thread pool workers
 */

#include <common/logger.h>
#include <common/timer.h>

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// ============================================================================
// ThreadPool Class - A production-grade thread pool implementation
// ============================================================================
class ThreadPool {
   public:
    /**
     * @brief Constructor - creates the thread pool
     * @param num_threads Number of worker threads (defaults to CPU core count)
     */
    explicit ThreadPool(size_t num_threads = 0) : stop_(false) {
        if (num_threads == 0) {
            num_threads = std::thread::hardware_concurrency();
        }

        LOG_INFO_FMT("Thread pool started, workers: " << num_threads);

        // Create worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i]() { worker_loop(i); });
        }
    }

    /**
     * @brief Destructor - stops all threads and waits for completion
     */
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        LOG_INFO("Thread pool shut down");
    }

    /**
     * @brief Submit a task and get a future for the result
     * @tparam F Callable type
     * @tparam Args Argument types
     * @return std::future for retrieving the result
     */
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using ReturnType = decltype(f(args...));

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_) {
                throw std::runtime_error("Cannot submit to stopped thread pool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();

        return result;
    }

    // Non-copyable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

   private:
    void worker_loop(size_t id) {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();  // Execute the task outside the lock
        }
        (void)id;
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;
};

// ============================================================================
// Example 1: Basic thread pool usage
// ============================================================================
void example_basic_pool() {
    LOG_INFO("=== Example 1: Basic Thread Pool Usage ===");

    ThreadPool pool(4);

    // Submit multiple tasks
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 8; ++i) {
        futures.push_back(pool.submit([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return i * i;
        }));
    }

    // Collect results
    for (int i = 0; i < 8; ++i) {
        LOG_INFO_FMT("  Task " << i << " result: " << futures[static_cast<size_t>(i)].get());
    }
}

// ============================================================================
// Example 2: Simulating AI inference service
// ============================================================================
void example_inference_service() {
    LOG_INFO("=== Example 2: Simulated Inference Service ===");

    SCOPED_TIMER("inference_service");

    ThreadPool pool(4);
    std::vector<std::future<std::string>> results;

    // Simulate batch inference requests
    for (int i = 0; i < 10; ++i) {
        results.push_back(pool.submit([i]() {
            // Simulate model inference time
            std::this_thread::sleep_for(std::chrono::milliseconds(30 + (i % 3) * 10));
            return "request_" + std::to_string(i) + " -> prediction_" + std::to_string(i * 10);
        }));
    }

    // Collect inference results
    for (size_t i = 0; i < results.size(); ++i) {
        LOG_INFO_FMT("  " << results[i].get());
    }
}

// ============================================================================
// Example 3: Performance comparison - Thread pool vs creating new threads
// ============================================================================
void example_performance_comparison() {
    LOG_INFO("=== Example 3: Performance Comparison ===");
    const int num_tasks = 100;

    // Approach 1: Create new threads each time (slow)
    {
        SCOPED_TIMER("new thread per task");
        std::vector<std::thread> threads;
        for (int i = 0; i < num_tasks; ++i) {
            threads.emplace_back([]() {
                volatile int sum = 0;
                for (int j = 0; j < 1000; ++j) sum += j;
                (void)sum;
            });
        }
        for (auto& t : threads) t.join();
    }

    // Approach 2: Thread pool (fast)
    {
        SCOPED_TIMER("thread pool");
        ThreadPool pool(4);
        std::vector<std::future<void>> futures;
        for (int i = 0; i < num_tasks; ++i) {
            futures.push_back(pool.submit([]() {
                volatile int sum = 0;
                for (int j = 0; j < 1000; ++j) sum += j;
                (void)sum;
            }));
        }
        for (auto& f : futures) f.get();
    }
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Thread Pool");
    LOG_INFO("========================================");

    example_basic_pool();
    example_inference_service();
    example_performance_comparison();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
