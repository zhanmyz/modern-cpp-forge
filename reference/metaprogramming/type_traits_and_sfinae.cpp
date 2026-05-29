/**
 * @file type_traits_and_sfinae.cpp
 * @brief Type Traits & SFINAE - Compile-time type inspection and selection
 *
 * [Learning Objectives]
 * 1. Understand type_traits: compile-time type property checking
 * 2. Learn SFINAE principle: Substitution Failure Is Not An Error
 * 3. Master if constexpr for compile-time branching
 * 4. Understand how these techniques enable efficient generic code
 *
 * [Core Concept - Analogy]
 * type_traits are like a "material inspector":
 * - Is this metal? (is_integral)
 * - Can this float on water? (is_floating_point)
 * - Can this be copied? (is_copy_constructible)
 *
 * SFINAE is like "automatic routing":
 * - You bring metal to the metalworking shop -> success
 * - You bring metal to the woodworking shop -> no error, just skipped
 * - The compiler automatically finds the right "shop" (function overload)
 *
 * [Applications in AI/ML]
 * - Select compute kernels based on data type (FP32/FP16/INT8)
 * - Compile-time detection of operation support (e.g., GPU acceleration)
 * - Serialization frameworks: detect whether a type has a serialize() method
 */

#include <common/logger.h>

#include <string>
#include <type_traits>
#include <vector>

// ============================================================================
// Example 1: type_traits Basics - Compile-time type inspection
// ============================================================================
template <typename T>
void inspect_type(const std::string& name) {
    LOG_INFO_FMT("  Type " << name << ":");
    LOG_INFO_FMT("    is_integral?       " << std::is_integral_v<T>);
    LOG_INFO_FMT("    is_floating_point? " << std::is_floating_point_v<T>);
    LOG_INFO_FMT("    is_pointer?        " << std::is_pointer_v<T>);
    LOG_INFO_FMT("    is_class?          " << std::is_class_v<T>);
    LOG_INFO_FMT("    sizeof:            " << sizeof(T) << " bytes");
}

void example_type_traits_basic() {
    LOG_INFO("=== Example 1: type_traits Basics ===");

    inspect_type<int>("int");
    inspect_type<double>("double");
    inspect_type<std::string>("std::string");
    inspect_type<int*>("int*");
}

// ============================================================================
// Example 2: if constexpr - Compile-time conditional branching
// ============================================================================
template <typename T>
std::string to_string_smart(const T& value) {
    // if constexpr: compile-time branch selection (unmatched branches are not compiled)
    if constexpr (std::is_arithmetic_v<T>) {
        return "number: " + std::to_string(value);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "string: " + value;
    } else if constexpr (std::is_same_v<T, const char*>) {
        return "c-string: " + std::string(value);
    } else {
        return "unknown type";
    }
}

void example_if_constexpr() {
    LOG_INFO("=== Example 2: if constexpr Compile-time Branching ===");

    LOG_INFO_FMT("  " << to_string_smart(42));
    LOG_INFO_FMT("  " << to_string_smart(3.14));
    LOG_INFO_FMT("  " << to_string_smart(std::string("hello")));
    LOG_INFO_FMT("  " << to_string_smart("world"));
}

// ============================================================================
// Example 3: SFINAE - Overload selection based on type traits
// ============================================================================

// Detect whether a type has a size() method
template <typename T, typename = void>
struct has_size : std::false_type {};

template <typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

// Select implementation based on whether size() exists
template <typename T>
auto get_length(const T& container) -> std::enable_if_t<has_size<T>::value, size_t> {
    return container.size();
}

template <typename T>
auto get_length(const T& /*value*/) -> std::enable_if_t<!has_size<T>::value, size_t> {
    return 1;  // Scalar type, length is 1
}

void example_sfinae() {
    LOG_INFO("=== Example 3: SFINAE Overload Selection ===");

    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::string str = "hello";
    int num = 42;

    LOG_INFO_FMT("  vector length: " << get_length(vec));
    LOG_INFO_FMT("  string length: " << get_length(str));
    LOG_INFO_FMT("  int length:    " << get_length(num));
}

// ============================================================================
// Example 4: Practical Application - Generic Serialization
// ============================================================================

// Detect whether a type has a serialize() method
template <typename T, typename = void>
struct is_serializable : std::false_type {};

template <typename T>
struct is_serializable<T, std::void_t<decltype(std::declval<T>().serialize())>> : std::true_type {};

// Serializable class
struct ModelConfig {
    std::string name = "GPT";
    int layers = 12;

    std::string serialize() const {
        return "{name:" + name + ",layers:" + std::to_string(layers) + "}";
    }
};

// Non-serializable class
struct RawData {
    int value = 0;
};

template <typename T>
void save_to_disk(const T& obj) {
    if constexpr (is_serializable<T>::value) {
        std::string data = obj.serialize();
        LOG_INFO_FMT("  Saved: " << data);
    } else {
        LOG_WARNING_FMT("  Type not serializable, skipped (size=" << sizeof(obj) << " bytes)");
    }
}

void example_serialization() {
    LOG_INFO("=== Example 4: Generic Serialization ===");

    ModelConfig config;
    RawData raw;

    save_to_disk(config);  // Has serialize(), saves normally
    save_to_disk(raw);     // No serialize(), prints warning
}

// ============================================================================
// Example 5: Type Transformations - Remove/Add qualifiers
// ============================================================================
void example_type_transforms() {
    LOG_INFO("=== Example 5: Type Transformations ===");

    // remove_const: strip const qualifier
    using T1 = std::remove_const_t<const int>;
    bool t1_is_int = std::is_same_v<T1, int>;
    LOG_INFO_FMT("  remove_const<const int> is int? " << t1_is_int);

    // remove_reference: strip reference
    using T2 = std::remove_reference_t<int&>;
    bool t2_is_int = std::is_same_v<T2, int>;
    LOG_INFO_FMT("  remove_reference<int&> is int? " << t2_is_int);

    // decay: array->pointer, function->function pointer, strip cv and reference
    using T3 = std::decay_t<const int (&)[10]>;
    bool t3_check = std::is_same_v<T3, const int*>;
    LOG_INFO_FMT("  decay<const int(&)[10]> is const int*? " << t3_check);

    // conditional: compile-time ternary operator
    using BigType = std::conditional_t<(sizeof(int) > 4), int, long>;
    LOG_INFO_FMT("  conditional type size = " << sizeof(BigType) << " bytes");
}

// ============================================================================
// Example 6: Compile-time Computation (constexpr + type_traits)
// ============================================================================
template <typename T>
constexpr size_t optimal_alignment() {
    if constexpr (std::is_floating_point_v<T>) {
        return 32;  // 32-byte align for floats (SIMD-friendly)
    } else if constexpr (sizeof(T) <= 4) {
        return 4;
    } else {
        return 8;
    }
}

void example_compiletime_compute() {
    LOG_INFO("=== Example 6: Compile-time Computation ===");

    constexpr auto float_align = optimal_alignment<float>();
    constexpr auto int_align = optimal_alignment<int>();
    constexpr auto double_align = optimal_alignment<double>();

    LOG_INFO_FMT("  float  optimal alignment: " << float_align);
    LOG_INFO_FMT("  int    optimal alignment: " << int_align);
    LOG_INFO_FMT("  double optimal alignment: " << double_align);

    // These are all compile-time constants - zero runtime overhead!
    static_assert(float_align == 32, "float should be 32-byte aligned");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Type Traits & SFINAE");
    LOG_INFO("========================================");

    example_type_traits_basic();
    example_if_constexpr();
    example_sfinae();
    example_serialization();
    example_type_transforms();
    example_compiletime_compute();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
