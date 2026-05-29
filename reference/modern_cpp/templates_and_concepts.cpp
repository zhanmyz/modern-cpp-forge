/**
 * @file templates_and_concepts.cpp
 * @brief Templates & C++20 Concepts - Generic programming with constraints
 *
 * [Learning Objectives]
 * 1. Understand function templates and class templates
 * 2. Learn variadic templates
 * 3. Master C++20 Concepts for constraining template parameters
 *
 * [Core Concept - Analogy]
 * Templates are like a universal mold:
 * - Same cake mold can make chocolate, matcha, or cheesecake
 * - Mold shape (algorithm) stays fixed, ingredients (data types) vary
 *
 * Concepts are like "ingredient requirements":
 * - This mold requires ingredients that can be "stirred" (Sortable concept)
 * - Put a rock in, and the compiler says "this doesn't meet requirements"
 *
 * [Applications in AI/ML]
 * - Generic tensor class: Tensor<float>, Tensor<half>, Tensor<int8_t>
 * - Generic algorithms: sort, reduce, map work on any type satisfying constraints
 * - Generic representation of computation graph nodes
 */

#include <common/logger.h>

#include <concepts>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

// ============================================================================
// Example 1: Function Template Basics
// ============================================================================
// Simplest function template: automatic type deduction
template <typename T>
T max_value(T a, T b) {
    return (a > b) ? a : b;
}

// Multi-type parameter template
template <typename T, typename U>
auto add(T a, U b) -> decltype(a + b) {
    return a + b;
}

void example_function_template() {
    LOG_INFO("=== Example 1: Function Templates ===");

    LOG_INFO_FMT("  max(3, 7) = " << max_value(3, 7));
    LOG_INFO_FMT("  max(3.14, 2.71) = " << max_value(3.14, 2.71));
    LOG_INFO_FMT("  add(1, 2.5) = " << add(1, 2.5));  // int + double = double
}

// ============================================================================
// Example 2: Class Templates - Generic Container
// ============================================================================
template <typename T, size_t MaxSize = 64>
class FixedBuffer {
   public:
    bool push(const T& value) {
        if (size_ >= MaxSize)
            return false;
        data_[size_++] = value;
        return true;
    }

    T pop() {
        if (size_ == 0)
            throw std::runtime_error("Buffer empty");
        return data_[--size_];
    }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ >= MaxSize; }

   private:
    T data_[MaxSize];
    size_t size_ = 0;
};

void example_class_template() {
    LOG_INFO("=== Example 2: Class Templates ===");

    FixedBuffer<int, 4> int_buf;
    int_buf.push(10);
    int_buf.push(20);
    int_buf.push(30);
    LOG_INFO_FMT("  int_buf size: " << int_buf.size() << ", pop: " << int_buf.pop());

    FixedBuffer<std::string, 2> str_buf;
    str_buf.push("hello");
    str_buf.push("world");
    LOG_INFO_FMT("  str_buf full: " << (str_buf.full() ? "yes" : "no"));
}

// ============================================================================
// Example 3: Variadic Templates
// ============================================================================
// Purpose: accept any number of arguments of any type

// Recursion base case
void print_all() {
    std::cerr << "\n";
}

// Variadic template: print all arguments
template <typename First, typename... Rest>
void print_all(First&& first, Rest&&... rest) {
    std::cerr << first;
    if constexpr (sizeof...(rest) > 0) {
        std::cerr << ", ";
    }
    print_all(std::forward<Rest>(rest)...);
}

// Fold expressions (C++17) - more concise variadic handling
template <typename... Args>
auto sum_all(Args... args) {
    return (args + ...);  // Fold expression: expands to a1 + a2 + a3 + ...
}

void example_variadic_templates() {
    LOG_INFO("=== Example 3: Variadic Templates ===");

    std::cerr << "  print_all: ";
    print_all(1, "hello", 3.14, true);

    LOG_INFO_FMT("  sum_all(1,2,3,4,5) = " << sum_all(1, 2, 3, 4, 5));
    LOG_INFO_FMT("  sum_all(1.1, 2.2, 3.3) = " << sum_all(1.1, 2.2, 3.3));
}

// ============================================================================
// Example 4: C++20 Concepts - Constraining template parameters
// ============================================================================

// Define Concept: require arithmetic type
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

// Define Concept: require printable type
template <typename T>
concept Printable = requires(T t, std::ostream& os) {
    { os << t } -> std::same_as<std::ostream&>;
};

// Define Concept: require type with size() method
template <typename T>
concept Sizeable = requires(T t) {
    { t.size() } -> std::convertible_to<size_t>;
};

// Constrain template with Concept (clear error on constraint violation)
template <Numeric T>
T square(T x) {
    return x * x;
}

// requires clause: More complex constraints
template <typename Container>
    requires Sizeable<Container> && requires(Container c) {
        c.begin();
        c.end();
    }
auto average(const Container& c) {
    using value_type = typename Container::value_type;
    auto sum = std::accumulate(c.begin(), c.end(), value_type{});
    return static_cast<double>(sum) / static_cast<double>(c.size());
}

void example_concepts() {
    LOG_INFO("=== Example 4: C++20 Concepts ===");

    LOG_INFO_FMT("  square(5) = " << square(5));
    LOG_INFO_FMT("  square(3.14) = " << square(3.14));
    // square("hello"); // Compile error! "hello" does not satisfy Numeric concept

    std::vector<int> nums = {1, 2, 3, 4, 5};
    LOG_INFO_FMT("  average({1,2,3,4,5}) = " << average(nums));
}

// ============================================================================
// Example 5: CRTP (Curiously Recurring Template Pattern)
// ============================================================================
// Advanced template technique: static polymorphism (compile-time, zero runtime overhead)

template <typename Derived>
class Shape {
   public:
    double area() const {
        // Call derived implementation (resolved at compile time, no vtable overhead)
        return static_cast<const Derived*>(this)->area_impl();
    }

    void describe() const {
        LOG_INFO_FMT("  Shape: " << static_cast<const Derived*>(this)->name()
                                << ", Area: " << area());
    }
};

class Circle : public Shape<Circle> {
   public:
    explicit Circle(double r) : radius_(r) {}
    double area_impl() const { return 3.14159 * radius_ * radius_; }
    std::string name() const { return "Circle(r=" + std::to_string(radius_) + ")"; }

   private:
    double radius_;
};

class Rectangle : public Shape<Rectangle> {
   public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    double area_impl() const { return width_ * height_; }
    std::string name() const {
        return "Rect(" + std::to_string(width_) + "x" + std::to_string(height_) + ")";
    }

   private:
    double width_, height_;
};

void example_crtp() {
    LOG_INFO("=== Example 5: CRTP Static Polymorphism ===");

    Circle c(5.0);
    Rectangle r(3.0, 4.0);

    c.describe();  // Resolved at compile time: Circle::area_impl
    r.describe();  // Resolved at compile time: Rectangle::area_impl
    LOG_INFO("  CRTP: polymorphism with zero vtable overhead!");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Templates & Concepts");
    LOG_INFO("========================================");

    example_function_template();
    example_class_template();
    example_variadic_templates();
    example_concepts();
    example_crtp();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
