/**
 * @file test_logger.cpp
 * @brief Basic tests for the Logger component
 */

#include <common/logger.h>
#include <common/timer.h>

#include <cassert>
#include <thread>
#include <vector>

void test_all_levels() {
    LOG_DEBUG("This is a DEBUG message");
    LOG_INFO("This is an INFO message");
    LOG_WARNING("This is a WARNING message");
    LOG_ERROR("This is an ERROR message");
    LOG_FATAL("This is a FATAL message");
}

void test_formatted_output() {
    int value = 42;
    double pi = 3.14159;
    std::string name = "test";

    LOG_INFO_FMT("int: " << value << ", double: " << pi << ", string: " << name);
}

void test_multithread_safety() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < 5; ++j) {
                LOG_INFO_FMT("thread " << i << " message " << j);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
}

void test_timer() {
    {
        SCOPED_TIMER("timer_test");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    common::Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    timer.stop();
    assert(timer.elapsed_ms() >= 40.0);  // allow small margin of error
    LOG_INFO_FMT("Timer test passed: " << timer.elapsed_ms() << "ms");
}

int main() {
    LOG_INFO("========== Logger Tests ==========");

    test_all_levels();
    test_formatted_output();
    test_multithread_safety();
    test_timer();

    LOG_INFO("========== All Tests Passed! ==========");
    return 0;
}
