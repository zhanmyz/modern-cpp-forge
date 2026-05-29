/**
 * @file lambda_and_functional.cpp
 * @brief Lambda Expressions & Functional Programming
 *
 * [Learning Objectives]
 * 1. Master various lambda capture modes
 * 2. Learn std::function and std::bind usage
 * 3. Understand the closure concept
 * 4. Master C++20 templated lambdas
 *
 * [Core Concept - Analogy]
 * Lambda = disposable mini-tool
 * Like needing a screwdriver without going to the toolbox (defining a function),
 * just pull out a temporary one (lambda) and discard after use.
 *
 * Capture = what the lambda can "see" from outside
 * [=] Takes a photo (copy by value)
 * [&] Opens a window to see outside (reference)
 *
 * [Applications in AI/ML]
 * - Custom loss functions
 * - Callbacks (what to do when training finishes)
 * - Sorting and filtering rules
 * - Thread task definitions
 */

#include <common/logger.h>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

// ============================================================================
// Example 1: Lambda Basic Syntax
// ============================================================================
void example_lambda_basics() {
    LOG_INFO("=== Example 1: Lambda Basics ===");

    // Simplest lambda
    auto hello = []() { LOG_INFO("  Hello from Lambda!"); };
    hello();

    // Lambda with parameters
    auto add = [](int a, int b) { return a + b; };
    LOG_INFO_FMT("  add(3, 4) = " << add(3, 4));

    // Lambda with return type (usually auto-deduced)
    auto divide = [](double a, double b) -> double {
        if (b == 0)
            return 0;
        return a / b;
    };
    LOG_INFO_FMT("  divide(10, 3) = " << divide(10, 3));
}

// ============================================================================
// Example 2: Capture Modes In Depth
// ============================================================================
void example_captures() {
    LOG_INFO("=== Example 2: Capture Modes ===");

    int x = 10;
    int y = 20;
    std::string name = "World";

    // [=] Capture all by value (take a photo)
    auto by_value = [=]() {
        LOG_INFO_FMT("  [=] x=" << x << ", y=" << y);
        // x = 100; // Compile error! value captures are const by default
    };
    by_value();

    // [&] Capture all by reference (open a window)
    auto by_ref = [&]() {
        x = 100;  // Can modify external variables!
        LOG_INFO_FMT("  [&] Modified x to " << x);
    };
    by_ref();
    LOG_INFO_FMT("  External x is now: " << x);  // x actually changed

    // Mixed capture
    auto mixed = [&x, y, &name]() {
        x += y;  // x captured by ref, modifiable
        // y = 0;     // y captured by value, not modifiable
        name += "!";  // name captured by ref
        LOG_INFO_FMT("  [mixed] x=" << x << ", name=" << name);
    };
    mixed();

    // mutable Lambda:Allows modifying value-captured copies
    int counter = 0;
    auto increment = [counter]() mutable {
        ++counter;  // Modifies lambda's internal copy
        return counter;
    };
    LOG_INFO_FMT("  mutable: " << increment() << ", " << increment() << ", " << increment());
    LOG_INFO_FMT("  External counter is still: " << counter);  // unaffected

    // Init capture (C++14) - can create new variables
    auto ptr = std::make_unique<int>(42);
    auto with_move = [p = std::move(ptr)]() { LOG_INFO_FMT("  [init capture] *p = " << *p); };
    with_move();
}

// ============================================================================
// Example 3: Lambda as Callback
// ============================================================================
template <typename Callback>
void train_model(int epochs, Callback on_epoch_end) {
    for (int i = 1; i <= epochs; ++i) {
        float loss = 1.0f / static_cast<float>(i);
        on_epoch_end(i, loss);  // Call callback
    }
}

void example_callback() {
    LOG_INFO("=== Example 3: Lambda as Callback ===");

    float best_loss = 999.0f;

    train_model(5, [&best_loss](int epoch, float loss) {
        if (loss < best_loss) {
            best_loss = loss;
            LOG_INFO_FMT("  Epoch " << epoch << ": loss=" << loss << " (new best!)");
        } else {
            LOG_INFO_FMT("  Epoch " << epoch << ": loss=" << loss);
        }
    });
}

// ============================================================================
// Example 4: std::function - Storable Callable Objects
// ============================================================================
void example_std_function() {
    LOG_INFO("=== Example 4: std::function ===");

    // std::function Can store any callable
    std::function<int(int, int)> operation;

    operation = [](int a, int b) { return a + b; };
    LOG_INFO_FMT("  Addition: " << operation(3, 4));

    operation = [](int a, int b) { return a * b; };
    LOG_INFO_FMT("  Multiplication: " << operation(3, 4));

    // Store in container
    std::vector<std::function<void()>> tasks;
    tasks.push_back([]() { LOG_INFO("  Task 1 executed"); });
    tasks.push_back([]() { LOG_INFO("  Task 2 executed"); });
    tasks.push_back([]() { LOG_INFO("  Task 3 executed"); });

    for (auto& task : tasks) {
        task();
    }
}

// ============================================================================
// Example 5: Higher-Order Functions
// ============================================================================
auto make_multiplier(int factor) {
    // Returns a lambda (closure) that "remembers" factor
    return [factor](int x) { return x * factor; };
}

auto compose(std::function<int(int)> f, std::function<int(int)> g) {
    return [f, g](int x) { return f(g(x)); };
}

void example_higher_order() {
    LOG_INFO("=== Example 5: Higher-Order Functions ===");

    auto double_it = make_multiplier(2);
    auto triple_it = make_multiplier(3);

    LOG_INFO_FMT("  double(5) = " << double_it(5));
    LOG_INFO_FMT("  triple(5) = " << triple_it(5));

    // Function composition: f(g(x))
    auto six_times = compose(double_it, triple_it);  // 2 * (3 * x)
    LOG_INFO_FMT("  six_times(5) = " << six_times(5));
}

// ============================================================================
// Example 6: C++20 Templated Lambda
// ============================================================================
void example_generic_lambda() {
    LOG_INFO("=== Example 6: Generic Lambda (C++20) ===");

    // C++14: auto-parameter generic lambda
    auto print_any = [](const auto& value) { LOG_INFO_FMT("  value = " << value); };
    print_any(42);
    print_any(3.14);
    print_any("hello");

    // C++20: Templated lambda
    auto typed_max = []<typename T>(T a, T b) -> T { return (a > b) ? a : b; };
    LOG_INFO_FMT("  max(10, 20) = " << typed_max(10, 20));
    LOG_INFO_FMT("  max(3.1, 2.9) = " << typed_max(3.1, 2.9));
}

// ============================================================================
// Example 7: Lambda with STL Algorithms
// ============================================================================
void example_stl_lambda() {
    LOG_INFO("=== Example 7: Lambda in STL Algorithms ===");

    std::vector<int> nums = {5, 2, 8, 1, 9, 3, 7, 4, 6};

    // Sort
    std::sort(nums.begin(), nums.end(), [](int a, int b) { return a > b; });
    std::string sorted;
    for (int n : nums)
        sorted += std::to_string(n) + " ";
    LOG_INFO_FMT("  Descending: " << sorted);

    // Filter (find all even numbers)
    std::vector<int> evens;
    std::copy_if(nums.begin(), nums.end(), std::back_inserter(evens),
                 [](int n) { return n % 2 == 0; });
    std::string even_str;
    for (int n : evens)
        even_str += std::to_string(n) + " ";
    LOG_INFO_FMT("  Evens: " << even_str);

    // Transform
    std::vector<int> squares;
    std::transform(nums.begin(), nums.end(), std::back_inserter(squares),
                   [](int n) { return n * n; });
    std::string sq_str;
    for (int n : squares)
        sq_str += std::to_string(n) + " ";
    LOG_INFO_FMT("  Squares: " << sq_str);
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Lambda & Functional Programming");
    LOG_INFO("========================================");

    example_lambda_basics();
    example_captures();
    example_callback();
    example_std_function();
    example_higher_order();
    example_generic_lambda();
    example_stl_lambda();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
