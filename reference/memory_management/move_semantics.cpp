/**
 * @file move_semantics.cpp
 * @brief Move Semantics - Avoid unnecessary copies, boost performance
 *
 * [Learning Objectives]
 * 1. Understand lvalues and rvalues
 * 2. Understand move construction and move assignment
 * 3. Learn proper use of std::move and perfect forwarding
 *
 * [Core Concept - Analogy]
 * Moving houses analogy:
 * - Copy = duplicate everything to new house (slow and expensive)
 * - Move = just take things to new house, old one is empty (fast and cheap)
 *
 * For example, a vector<int> with 1M elements:
 * - Copy: allocate new memory, copy 1M elements one by one
 * - Move: just transfer 3 pointers (data, size, capacity)
 *
 * [Applications in AI/ML]
 * - Move large tensors between functions to avoid copies
 * - Model weight loading and transfer
 * - Efficient transfer of inference requests/responses
 */

#include <common/logger.h>
#include <common/timer.h>

#include <cstring>
#include <string>
#include <utility>
#include <vector>

// ============================================================================
// Custom Tensor class demonstrating move semantics
// ============================================================================
class Tensor {
   public:
    // Constructor
    Tensor(size_t size, const std::string& name)
        : size_(size), name_(name), data_(new float[size]) {
        std::memset(data_, 0, size * sizeof(float));
        LOG_INFO_FMT("  [Construct] " << name_ << " (size=" << size_ << ")");
    }

    // Destructor
    ~Tensor() {
        if (data_) {
            LOG_DEBUG_FMT("  [Destruct] " << name_ << " (freeing memory)");
            delete[] data_;
        }
    }

    // ========== Copy Semantics ==========

    // Copy constructor (deep copy, slow!)
    Tensor(const Tensor& other)
        : size_(other.size_), name_(other.name_ + "_copy"), data_(new float[other.size_]) {
        std::memcpy(data_, other.data_, size_ * sizeof(float));
        LOG_WARNING_FMT("  [Copy construct] " << name_ << " (deep copying " << size_
                                              << "  floats)");
    }

    // Copy assignment operator
    Tensor& operator=(const Tensor& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            name_ = other.name_ + "_copy";
            data_ = new float[size_];
            std::memcpy(data_, other.data_, size_ * sizeof(float));
            LOG_WARNING_FMT("  [Copy assign] " << name_);
        }
        return *this;
    }

    // ========== Move Semantics ==========

    // Move constructor (fast! pointer transfer only)
    Tensor(Tensor&& other) noexcept
        : size_(other.size_), name_(std::move(other.name_)), data_(other.data_) {
        // Steal resources, leave source empty
        other.data_ = nullptr;
        other.size_ = 0;
        LOG_INFO_FMT("  [Move construct] " << name_ << " (zero-copy!)");
    }

    // Move assignment operator
    Tensor& operator=(Tensor&& other) noexcept {
        if (this != &other) {
            delete[] data_;  // Free own old resource
            // Steal other's resources
            size_ = other.size_;
            name_ = std::move(other.name_);
            data_ = other.data_;
            other.data_ = nullptr;
            other.size_ = 0;
            LOG_INFO_FMT("  [Move assign] " << name_ << " (zero-copy!)");
        }
        return *this;
    }

    // Utility methods
    size_t size() const { return size_; }
    const std::string& name() const { return name_; }
    bool is_valid() const { return data_ != nullptr; }

    void fill(float value) {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = value;
        }
    }

   private:
    size_t size_;
    std::string name_;
    float* data_;
};

// ============================================================================
// Example 1: Lvalues vs Rvalues
// ============================================================================
void example_lvalue_rvalue() {
    LOG_INFO("=== Example 1: Lvalues vs Rvalues ===");

    // lvalue = named, addressable, persistent object
    int x = 42;               // x is an lvalue
    std::string s = "hello";  // s is an lvalue

    // rvalue = temporary, about to be destroyed, unnamed
    // 42 is an rvalue
    // std::string("temp") is an rvalue
    // x + 1 result is an rvalue

    LOG_INFO_FMT("  x = " << x << " (x is lvalue: named, addressable)");
    LOG_INFO_FMT("  s = " << s << " (s is lvalue)");
    LOG_INFO("  42, x+1, string(\"temp\") are rvalues (temporaries)");

    // std::move does NOT move! It just casts lvalue to rvalue reference
    // It tells the compiler: "I no longer need this, you may steal its resources"
    std::string s2 = std::move(s);  // s 's content was stolen
    LOG_INFO_FMT("  After move: s=\"" << s << "\", s2=\"" << s2 << "\"");
}

// ============================================================================
// Example 2: Move vs Copy Performance
// ============================================================================
void example_move_performance() {
    LOG_INFO("=== Example 2: Move vs Copy Performance ===");

    const size_t tensor_size = 1000000;  // 1M floats = 4MB

    // Copy method (slow)
    {
        SCOPED_TIMER("Copy Tensor");
        Tensor t1(tensor_size, "original");
        t1.fill(1.0f);
        Tensor t2 = t1;  // Invokes copy constructor (deep-copies 4MB)
        (void)t2;
    }

    LOG_INFO("---");

    // Move method (fast)
    {
        SCOPED_TIMER("Move Tensor");
        Tensor t1(tensor_size, "original");
        t1.fill(1.0f);
        Tensor t2 = std::move(t1);  // Invokes move constructor (pointer transfer only)
        LOG_INFO_FMT("  After move, t1 valid: " << (t1.is_valid() ? "yes" : "no"));
        LOG_INFO_FMT("  After move, t2 valid: " << (t2.is_valid() ? "yes" : "no"));
    }
}

// ============================================================================
// Example 3: Return Value Optimization (RVO) + Move Semantics
// ============================================================================
Tensor create_tensor(size_t size) {
    Tensor t(size, "created_in_function");
    t.fill(3.14f);
    return t;  // Compiler applies RVO or move semantics, no copy!
}

void example_return_value() {
    LOG_INFO("=== Example 3: Return Value (RVO + Move) ===");

    // Compiler optimizes: constructs in caller's memory, zero copy/move
    Tensor t = create_tensor(1000);
    LOG_INFO_FMT("  Result: " << t.name() << " size=" << t.size());
}

// ============================================================================
// Example 4: Perfect Forwarding (std::forward)
// ============================================================================
// Perfect forwarding: preserves lvalue/rvalue nature of args to next function
template <typename T>
void wrapper(T&& arg) {
    // If arg was originally lvalue, forward keeps it as lvalue ref
    // If arg was originally rvalue, forward keeps it as rvalue ref
    process_impl(std::forward<T>(arg));
}

void process_impl(const Tensor& t) {
    LOG_INFO_FMT("  process_impl(lvalue ref): " << t.name());
}

void process_impl(Tensor&& t) {
    LOG_INFO_FMT("  process_impl(rvalue ref): " << t.name());
    Tensor local = std::move(t);  // Safe to move
    (void)local;
}

void example_perfect_forwarding() {
    LOG_INFO("=== Example 4: Perfect Forwarding ===");

    Tensor t(100, "my_tensor");

    wrapper(t);                   // Pass lvalue → calls process_impl(const Tensor&)
    wrapper(std::move(t));        // Pass rvalue → calls process_impl(Tensor&&)
    wrapper(Tensor(50, "temp"));  // Pass rvalue → calls process_impl(Tensor&&)
}

// ============================================================================
// Example 5: Move Semantics in vector
// ============================================================================
void example_vector_move() {
    LOG_INFO("=== Example 5: Move in vector ===");

    std::vector<Tensor> tensors;
    tensors.reserve(3);  // Pre-allocate to avoid reallocation moves

    // emplace_back: In-place construction (most efficient)
    tensors.emplace_back(100, "tensor_a");
    tensors.emplace_back(200, "tensor_b");

    // push_back + move: Move existing object into container
    Tensor t(300, "tensor_c");
    tensors.push_back(std::move(t));  // Move instead of copy

    LOG_INFO_FMT("  vector has " << tensors.size() << " tensors");
    LOG_INFO_FMT("  Original t valid: " << (t.is_valid() ? "yes" : "no"));
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Move Semantics");
    LOG_INFO("========================================");

    example_lvalue_rvalue();
    example_move_performance();
    example_return_value();
    example_perfect_forwarding();
    example_vector_move();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
