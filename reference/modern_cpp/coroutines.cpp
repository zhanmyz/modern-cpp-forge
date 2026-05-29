/**
 * @file coroutines.cpp
 * @brief C++20 Coroutines - Lightweight concurrent programming
 *
 * [Learning Objectives]
 * 1. Understand coroutines vs threads
 * 2. Learn co_yield, co_return, co_await
 * 3. Implement a simple Generator
 *
 * [Core Concept - Analogy]
 * Threads = multiple chefs cooking simultaneously (true parallelism)
 * Coroutines = one chef alternating between dishes (cooperative switching)
 *   - While water boils → chop veggies (co_await = suspend current task)
 *   - Veggies done → check if water boiled (resume execution)
 *
 * Coroutines are much lighter than threads:
 * - Creating a thread ≈ cost of creating 1M objects
 * - Creating a coroutine ≈ cost of creating 1 object
 * - Thread switch requires OS involvement (slow)
 * - Coroutine switch is user-space only (fast)
 *
 * [Applications in AI/ML]
 * - Streaming inference output (token-by-token like ChatGPT)
 * - Data pipelines: generators produce data on demand
 * - Async I/O: network requests don't block inference threads
 */

#include <common/logger.h>

#include <coroutine>
#include <exception>
#include <string>
#include <vector>

// ============================================================================
// Generator Type - A simple coroutine generator
// ============================================================================
template <typename T>
class Generator {
   public:
    // Compiler-required promise_type
    struct promise_type {
        T current_value;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(T value) {
            current_value = std::move(value);
            return {};
        }

        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    using Handle = std::coroutine_handle<promise_type>;

    explicit Generator(Handle h) : handle_(h) {}
    ~Generator() {
        if (handle_)
            handle_.destroy();
    }

    // Move Semantics
    Generator(Generator&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (handle_)
                handle_.destroy();
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    // Non-copyable
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    // Iterator interface
    bool next() {
        handle_.resume();
        return !handle_.done();
    }

    T value() const { return handle_.promise().current_value; }

   private:
    Handle handle_;
};

// ============================================================================
// Example 1: Simple Number Generator
// ============================================================================
Generator<int> range(int start, int end) {
    for (int i = start; i < end; ++i) {
        co_yield i;  // Suspend and return value, resume on next next()
    }
}

void example_basic_generator() {
    LOG_INFO("=== Example 1: Simple Generator ===");

    auto gen = range(1, 6);
    while (gen.next()) {
        LOG_INFO_FMT("  Generated: " << gen.value());
    }
}

// ============================================================================
// Example 2: Fibonacci Generator (infinite sequence)
// ============================================================================
Generator<long long> fibonacci() {
    long long a = 0, b = 1;
    while (true) {
        co_yield a;
        long long temp = a + b;
        a = b;
        b = temp;
    }
}

void example_fibonacci() {
    LOG_INFO("=== Example 2: Fibonacci Generator (lazy evaluation) ===");

    auto fib = fibonacci();
    for (int i = 0; i < 10; ++i) {
        fib.next();
        LOG_INFO_FMT("  fib(" << i << ") = " << fib.value());
    }
}

// ============================================================================
// Example 3: Simulated streaming token generation (LLM-like)
// ============================================================================
Generator<std::string> generate_tokens(std::string prompt) {
    // NOTE: take prompt BY VALUE! In coroutines, reference args are dangerous
    // because the coroutine suspends and the referenced object may be destroyed.
    std::vector<std::string> tokens = {"The",  " answer",  " to",   " '",   prompt,
                                       "'",    " is",      " that", " C++", " coroutines",
                                       " are", " awesome", "!"};

    for (const auto& token : tokens) {
        // Simulate per-token inference time
        co_yield token;
    }
}

void example_streaming() {
    LOG_INFO("=== Example 3: Streaming Token Generation (LLM-like) ===");

    auto stream = generate_tokens("What is C++?");
    std::string full_response;

    LOG_INFO("  Streaming output: ");
    while (stream.next()) {
        std::string token = stream.value();
        full_response += token;
        // In practice, tokens can be sent to user immediately
    }
    LOG_INFO_FMT("  Full response: " << full_response);
}

// ============================================================================
// Example 4: Data Pipeline (filter + transform)
// ============================================================================
Generator<int> filter_even(Generator<int>& source) {
    while (source.next()) {
        int val = source.value();
        if (val % 2 == 0) {
            co_yield val;
        }
    }
}

Generator<int> multiply(Generator<int>& source, int factor) {
    while (source.next()) {
        co_yield source.value() * factor;
    }
}

void example_pipeline() {
    LOG_INFO("=== Example 4: Coroutine Data Pipeline ===");

    auto numbers = range(1, 20);
    auto evens = filter_even(numbers);
    auto doubled = multiply(evens, 2);

    std::string result;
    while (doubled.next()) {
        result += std::to_string(doubled.value()) + " ";
    }
    LOG_INFO_FMT("  Evens in 1-19 * 2: " << result);
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  C++20 Coroutines");
    LOG_INFO("========================================");

    example_basic_generator();
    example_fibonacci();
    example_streaming();
    example_pipeline();

    LOG_INFO("========================================");
    LOG_INFO("  Coroutine key points:");
    LOG_INFO("  - co_yield: suspend and return value");
    LOG_INFO("  - co_return: end coroutine");
    LOG_INFO("  - co_await: suspend awaiting async operation");
    LOG_INFO("  - 1000x lighter than threads!");
    LOG_INFO("========================================");

    return 0;
}
