/**
 * @file function_test.cpp
 * @brief <functional> header — Advanced usage of std::function and friends
 * Build: cd modern-cpp-forge && cmake -B build && cmake --build build --target src_modern_cpp_function_test 2>&1
 * Reference: https://en.cppreference.com/w/cpp/utility/functional
 *            https://en.cppreference.com/w/cpp/header/functional
 *            https://en.cppreference.com/cpp/utility/functional/function
 */

#include <common/logger.h>

#include <functional>

void print_message(const std::string& msg) {
    LOG_INFO_FMT("  Message: " << msg);
}

struct Foo {
    explicit Foo(int num) : num_(num) {}
    void print_add(int i) const { LOG_INFO_FMT("  Foo::print_add: " << (num_ + i)); }
    int num_;
};

struct PrintNum {
    void operator()(int n) const { LOG_INFO_FMT("  PrintNum: " << n); }
};

int main() {
    // 1.Test std::function with free function
    LOG_INFO("=============================== 1.Free function =============================");
    // std::function<void(const std::string&)> func1 = &print_message;
    std::function<void(const std::string&)> func1 =
        print_message;  // OK，the ordinary function name can be implicity converted to function pointer, no need for &. But for member function, must use &: `std::function<void(const Foo&, int)> func4 = &Foo::print_add;`
    func1("Hello from std::function with free function!");

    // 2.Test std::function with lambda
    LOG_INFO("=============================== 2.Lambda =============================");
    std::function<void(std::string)> func2 = [](std::string str) { print_message(str); };
    func2("Hello from std::function with lambda!");

    // 3.Store the result of a call to std::bind (or a lambda) in a std::function
    LOG_INFO("=============================== 3.Bind =============================");
    std::function<void()> func3 =
        std::bind(print_message, "Hello from std::function with std::bind!");
    func3();

    // 4.Test std::function with member function
    LOG_INFO("=============================== 4.Member function =============================");
    std::function<void(const Foo&, int)> func4 =
        &Foo::print_add;  // 成员函数不能隐式转换为函数指针，必须加 &
    const Foo foo(314159);
    func4(foo, 1);
    // func4(314159, 1);  // Error: cannot convert from int to Foo, because Foo constructor is explicit: `explicit Foo(int num)`
    // func4(314159, 1);  // OK, If there is no explicit: `Foo(int num)`, Implicitly converts to Foo, equivalent to`func4(Foo(314159), 1)`

    return 0;
}