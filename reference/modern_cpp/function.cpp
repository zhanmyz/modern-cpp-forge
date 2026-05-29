/**
 * @file function.cpp
 * @brief <functional> header — Advanced usage of std::function and friends
 * Build: cd learning_cpp && cmake -B build && cmake --build build --target ref_modern_cpp_function 2>&1
 * Reference: https://en.cppreference.com/w/cpp/utility/functional
 *            https://en.cppreference.com/w/cpp/header/functional
 *            https://en.cppreference.com/cpp/utility/functional/function
 *
 * Key components covered:
 *   1. std::function       — type-erased callable wrapper (stores any callable)
 *   2. std::bind / bind_front — partial function application (fix some args)
 *   3. std::invoke         — universal call mechanism (works on anything callable)
 *   4. std::mem_fn         — wraps pointer-to-member into a callable
 *   5. std::ref / std::cref — pass references into things that copy by default
 *   6. std::not_fn         — negate the result of a callable
 *   7. std::move_only_function (C++23) — like std::function but move-only
 *   8. Operator function objects (std::plus, std::less, etc.)
 */

#include <common/logger.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

// =============================================================================
// Example 1: std::function basics
// =============================================================================
// std::function<ReturnType(ArgTypes...)> can hold ANY callable:
//   - free function
//   - lambda
//   - functor (object with operator())
//   - member function (with object bound)

int add(int a, int b) {
    return a + b;
}

struct Multiplier {
    int factor;
    int operator()(int x) const { return x * factor; }
};

void example_std_function() {
    LOG_INFO("=== Example 1: std::function basics ===");

    // Store a free function
    std::function<int(int, int)> fn = add;
    LOG_INFO_FMT("  add(3, 4) = " << fn(3, 4));

    // Store a lambda
    fn = [](int a, int b) { return a * b; };
    LOG_INFO_FMT("  lambda(3, 4) = " << fn(3, 4));

    // Store a functor
    std::function<int(int)> fn2 = Multiplier{5};
    LOG_INFO_FMT("  Multiplier{5}(7) = " << fn2(7));

    // Check if function is empty (not assigned)
    std::function<void()> empty_fn;
    LOG_INFO_FMT("  empty_fn is null: " << (!empty_fn ? "yes" : "no"));

    // Calling an empty std::function throws std::bad_function_call
    try {
        empty_fn();
    } catch (const std::bad_function_call& e) {
        LOG_INFO_FMT("  Caught: " << e.what());
    }
}

// =============================================================================
// Example 2: std::function as callback / strategy
// =============================================================================
// The killer feature: pass behavior as a parameter

using LossFunction = std::function<double(double predicted, double actual)>;

double train_step(double predicted, double actual, const LossFunction& loss_fn) {
    return loss_fn(predicted, actual);
}

void example_function_as_callback() {
    LOG_INFO("=== Example 2: std::function as callback ===");

    // MSE loss
    LossFunction mse = [](double pred, double actual) {
        double diff = pred - actual;
        return diff * diff;
    };

    // MAE loss
    LossFunction mae = [](double pred, double actual) { return std::abs(pred - actual); };

    double pred = 2.5, actual = 3.0;
    LOG_INFO_FMT("  MSE loss: " << train_step(pred, actual, mse));
    LOG_INFO_FMT("  MAE loss: " << train_step(pred, actual, mae));

    // Swap strategy at runtime — that's the power!
    LossFunction current_loss = mse;
    LOG_INFO_FMT("  Current loss: " << current_loss(pred, actual));
    current_loss = mae;  // switch strategy
    LOG_INFO_FMT("  After switch: " << current_loss(pred, actual));
}

// =============================================================================
// Example 3: std::bind — partial application (fix some arguments)
// =============================================================================
// Think of it as: "I have a 3-arg function, let me pre-fill 2 args to get a 1-arg function"

double power(double base, double exp) {
    double result = 1.0;
    for (int i = 0; i < static_cast<int>(exp); ++i)
        result *= base;
    return result;
}

void example_bind() {
    LOG_INFO("=== Example 3: std::bind — partial application ===");

    using namespace std::placeholders;

    // Fix first arg: square(x) = power(x, 2)
    auto square = std::bind(power, _1, 2.0);
    LOG_INFO_FMT("  square(5) = " << square(5.0));

    // Fix second arg: cube(x) = power(x, 3)
    auto cube = std::bind(power, _1, 3.0);
    LOG_INFO_FMT("  cube(3) = " << cube(3.0));

    // Reorder arguments: _2 as first, _1 as second
    auto reversed = std::bind(power, _2, _1);
    LOG_INFO_FMT("  reversed(2, 10) = power(10, 2) = " << reversed(2.0, 10.0));

    // C++20 bind_front is simpler (binds leading args only, no placeholders)
    auto power_of_2 = std::bind_front(power, 2.0);  // power(2, ?)
    LOG_INFO_FMT("  bind_front: 2^8 = " << power_of_2(8.0));
}

// =============================================================================
// Example 4: std::invoke — universal call anything
// =============================================================================
// std::invoke(callable, args...) works uniformly for:
//   - free functions
//   - member functions (pass object as first arg)
//   - member data pointers
//   - functors / lambdas

struct Model {
    std::string name = "GPT";
    int params = 175;
    int get_params() const { return params; }
};

void example_invoke() {
    LOG_INFO("=== Example 4: std::invoke — call anything uniformly ===");

    // Call free function
    auto result = std::invoke(add, 10, 20);
    LOG_INFO_FMT("  invoke(add, 10, 20) = " << result);

    // Call member function on an object
    Model m;
    auto p = std::invoke(&Model::get_params, m);
    LOG_INFO_FMT("  invoke(&Model::get_params, m) = " << p);

    // Access member variable
    auto name = std::invoke(&Model::name, m);
    LOG_INFO_FMT("  invoke(&Model::name, m) = " << name);

    // Call lambda
    auto lam = [](int x) { return x * x; };
    LOG_INFO_FMT("  invoke(lambda, 7) = " << std::invoke(lam, 7));
}

// =============================================================================
// Example 5: std::mem_fn — wrap member function pointer into callable
// =============================================================================
// Useful when STL algorithms need a callable but you have a member function

void example_mem_fn() {
    LOG_INFO("=== Example 5: std::mem_fn — member function wrapper ===");

    std::vector<std::string> words = {"hello", "world", "functional", "programming"};

    // Get sizes using mem_fn (wraps std::string::size into a callable)
    auto get_size = std::mem_fn(&std::string::size);

    LOG_INFO("  Word sizes:");
    for (const auto& w : words) {
        LOG_INFO_FMT("    \"" << w << "\" -> " << get_size(w));
    }

    // Use with std::transform
    std::vector<size_t> sizes(words.size());
    std::transform(words.begin(), words.end(), sizes.begin(), std::mem_fn(&std::string::size));
    std::string sizes_str;
    for (auto s : sizes)
        sizes_str += std::to_string(s) + " ";
    LOG_INFO_FMT("  Sizes: " << sizes_str);
}

// =============================================================================
// Example 6: std::ref / std::cref — pass by reference where copy is default
// =============================================================================
// Problem: std::bind, std::thread, etc. COPY their arguments by default.
// Solution: std::ref(x) creates a reference_wrapper that acts like a reference.

void increment(int& x) {
    ++x;
}

void example_ref() {
    LOG_INFO("=== Example 6: std::ref / std::cref ===");

    int value = 10;

    // Without std::ref: bind copies value, original unchanged
    auto fn_copy = std::bind(increment, value);
    fn_copy();
    LOG_INFO_FMT("  After bind(increment, value): value = " << value);  // still 10

    // With std::ref: bind holds a reference, original gets modified
    auto fn_ref = std::bind(increment, std::ref(value));
    fn_ref();
    LOG_INFO_FMT("  After bind(increment, ref(value)): value = " << value);  // 11

    // std::cref for const reference
    int data = 42;
    auto print_val = [](const int& x) { LOG_INFO_FMT("    cref value: " << x); };
    auto bound = std::bind(print_val, std::cref(data));
    data = 99;  // modify after binding
    bound();    // prints 99, because cref holds reference to original
}

// =============================================================================
// Example 7: std::not_fn — negate any predicate
// =============================================================================
// not_fn(pred) returns a new callable that returns !pred(args...)

void example_not_fn() {
    LOG_INFO("=== Example 7: std::not_fn — negate predicates ===");

    std::vector<int> nums = {1, -2, 3, -4, 5, -6, 7};

    auto is_positive = [](int x) { return x > 0; };
    auto is_not_positive = std::not_fn(is_positive);

    // Count negative/zero numbers
    auto neg_count = std::count_if(nums.begin(), nums.end(), is_not_positive);
    LOG_INFO_FMT("  Non-positive count: " << neg_count);

    // Partition: move positives to front
    std::vector<int> sorted_nums = nums;
    auto it = std::partition(sorted_nums.begin(), sorted_nums.end(), is_positive);
    std::string result;
    for (int n : sorted_nums)
        result += std::to_string(n) + " ";
    LOG_INFO_FMT("  After partition (positives first): " << result);
    LOG_INFO_FMT("  First non-positive at index: " << (it - sorted_nums.begin()));
}

// =============================================================================
// Example 8: Operator function objects (std::plus, std::less, etc.)
// =============================================================================
// Pre-built function objects for common operations.
// The <void> specialization (C++14) deduces types automatically.

void example_operator_objects() {
    LOG_INFO("=== Example 8: Operator function objects ===");

    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {10, 20, 30, 40, 50};
    std::vector<int> result(5);

    // std::plus<> adds two things — works as a callable
    std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::plus<>{});
    std::string s;
    for (int v : result)
        s += std::to_string(v) + " ";
    LOG_INFO_FMT("  a + b = " << s);

    // std::multiplies<> for element-wise multiply
    std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::multiplies<>{});
    s.clear();
    for (int v : result)
        s += std::to_string(v) + " ";
    LOG_INFO_FMT("  a * b = " << s);

    // std::accumulate with custom op
    int sum = std::accumulate(a.begin(), a.end(), 0, std::plus<>{});
    int product = std::accumulate(a.begin(), a.end(), 1, std::multiplies<>{});
    LOG_INFO_FMT("  sum(a) = " << sum << ", product(a) = " << product);

    // std::less<> / std::greater<> for sorting
    std::vector<int> nums = {5, 2, 8, 1, 9};
    std::sort(nums.begin(), nums.end(), std::greater<>{});  // descending
    s.clear();
    for (int v : nums)
        s += std::to_string(v) + " ";
    LOG_INFO_FMT("  Sorted descending: " << s);
}

// =============================================================================
// Example 9: Storing heterogeneous callables in a container
// =============================================================================
// std::function lets you put different callable types in the same vector

void example_callable_container() {
    LOG_INFO("=== Example 9: Callable container (command pattern) ===");

    std::vector<std::function<void()>> commands;

    // Mix different callables in one container
    commands.push_back([]() { LOG_INFO("  [cmd] Initialize model"); });
    commands.push_back([]() { LOG_INFO("  [cmd] Load weights"); });
    commands.push_back([]() { LOG_INFO("  [cmd] Run inference"); });
    commands.push_back([]() { LOG_INFO("  [cmd] Save results"); });

    // Execute all
    for (auto& cmd : commands) {
        cmd();
    }

    // Transform pipeline: store operations with their names
    std::vector<std::pair<std::function<int(int)>, std::string>> transforms = {
        {[](int x) { return x * 2; }, "double"},
        {[](int x) { return x + 10; }, "add 10"},
        {[](int x) { return x - 3; }, "sub 3"},
    };

    int val = 5;
    for (const auto& [fn, name] : transforms) {
        val = fn(val);
        LOG_INFO_FMT("  After " << name << ": " << val);
    }
}

// =============================================================================
// Example 10: std::function with type erasure — EventBus
// =============================================================================
// This shows WHY std::function exists: it erases the concrete type of callables,
// so you can store lambdas with different captures in the same container.

class EventBus {
   public:
    using Handler = std::function<void(const std::string&)>;

    void subscribe(const std::string& event, Handler handler) {
        handlers_[event].push_back(std::move(handler));
    }

    void publish(const std::string& event, const std::string& data) {
        if (auto it = handlers_.find(event); it != handlers_.end()) {
            for (auto& handler : it->second) {
                handler(data);
            }
        }
    }

   private:
    std::unordered_map<std::string, std::vector<Handler>> handlers_;
};

void example_type_erasure() {
    LOG_INFO("=== Example 10: Type erasure — EventBus ===");

    EventBus bus;

    // Subscribe with lambda
    bus.subscribe("model.loaded", [](const std::string& data) {
        LOG_INFO_FMT("  [Logger] Model loaded: " << data);
    });

    // Subscribe with another lambda (different captures, same signature)
    int event_count = 0;
    bus.subscribe("model.loaded", [&event_count](const std::string& /*data*/) {
        ++event_count;
        LOG_INFO_FMT("  [Counter] Events so far: " << event_count);
    });

    bus.subscribe("inference.done", [](const std::string& data) {
        LOG_INFO_FMT("  [Monitor] Inference result: " << data);
    });

    // Publish events
    bus.publish("model.loaded", "bert-base");
    bus.publish("model.loaded", "gpt-2");
    bus.publish("inference.done", "42");
}

// =============================================================================
// Main
// =============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  <functional> Advanced Usage");
    LOG_INFO("========================================");

    example_std_function();
    example_function_as_callback();
    example_bind();
    example_invoke();
    example_mem_fn();
    example_ref();
    example_not_fn();
    example_operator_objects();
    example_callable_container();
    example_type_erasure();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete!");
    LOG_INFO("========================================");

    return 0;
}
