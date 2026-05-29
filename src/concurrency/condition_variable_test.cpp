/**
 * @file condition_variable_test.cpp
 * @brief <condition_variable> header — Advanced usage of std::condition_variable and friends
 * Build: cd modern-cpp-forge && cmake -B build && cmake --build build --target src_modern_cpp_condition_variable_test 2>&1
 * Reference: https://en.cppreference.com/cpp/header/condition_variable
 *            https://en.cppreference.com/cpp/thread/condition_variable
 */

#include <common/logger.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

using namespace common;

std::mutex mtx;
std::condition_variable cv;
std::string shared_data;
bool data_ready = false;
bool process_ready = false;

void worker_thread() {
    // wait until the main() sends data
    // Need to manually unlock in conjunction whith condition variable and notify_one/notify_all to notify waiting threads after the condition is met.
    // Otherwise the waiting thread will wake up and immediately block again because the mutex is still locked by the notifying thread,
    // leading to inefficient behavior and potential deadlocks if the waiting thread is waiting for a condition that can only be satisfied by the notifying thread
    std::unique_lock lk(mtx);
    // cv.wait(lk);
    // wait without predicate: can wake up spuriously, so must re-check condition after waking
    // if the notify_one() of main() is executed before this thread starts waiting, it will miss the notification and wait indefinitely
    // using the predicate version of wait() solves this problem by checking the condition before waiting and after waking up, ensuring that the thread only waits if the condition is not met and can handle spurious wakeups correctly
    cv.wait(lk, []() -> bool {
        LOG_INFO_FMT("  1./4.[Worker] predicate check: data_ready=" << data_ready);
        return data_ready;
    });

    // after the wait, we own the lock
    LOG_INFO_FMT("5.Worker thread is processing data: " << shared_data);
    shared_data += " processed";

    // send data back to main()
    process_ready = true;

    // Manual unlocking is done before notifying, to avoid waking up the waiting thread only to block again
    lk.unlock();

    LOG_INFO(
        "6.Worker thread has unlocked the mutex and is now simulating some processing time...");
    // Simulate some processing time after unlocking and before notifying
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    LOG_INFO("11.Worker thread is now notifying main thread that processing is done...");

    cv.notify_one();
}

int main() {
    common::Logger::instance().set_format(common::LogFormat::Timestamp | common::LogFormat::Level |
                                          common::LogFormat::Message);

    std::thread worker(worker_thread);
    shared_data = "[Shared Data] Main thread shared data...";
    // send shared data to the worker thread
    {
        // Can't manually unlock until exitting scope, so use lock_guard for exception safety
        std::lock_guard lk(mtx);
        data_ready = true;
        LOG_INFO("2.Main thread has set data_ready to true and is notifying worker thread...");
    }

    // Simulate some work before notifying
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    cv.notify_one();

    // wait for the worker thread to process the data
    {
        LOG_INFO("3.[Main] Trying to lock mtx ...");
        std::unique_lock lk(mtx);
        LOG_INFO("7.[Main] Got the lock! Now calling cv.wait...");
        cv.wait(lk, []() -> bool {
            LOG_INFO_FMT("  8.[Main] predicate check: process_ready=" << process_ready);
            return process_ready;
        });
        // Simulate some processing time after being notified
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        LOG_INFO_FMT("9.Main thread get the processed data: " << shared_data);
    }

    LOG_INFO("10.Main thread is now waiting for worker thread to finish...");
    worker.join();

    return 0;
}