# 🚀 Learning C++ — Advanced Topics for AI/ML Development

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++20">
  <img src="https://img.shields.io/badge/CMake-3.16+-green.svg" alt="CMake">
  <img src="https://img.shields.io/badge/Platform-Linux-lightgrey.svg" alt="Linux">
  <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="MIT">
</p>

<p align="center">
  <b>English</b> | <a href="README_zh.md">中文</a>
</p>

A hands-on project for systematically learning advanced C++ topics, focused on techniques commonly used in AI/LLM development: concurrency & thread pools, design patterns, modern C++ features, metaprogramming, and performance optimization.

Official C++ reference: https://en.cppreference.com

## 📁 Project Structure

```
learning_cpp/
├── CMakeLists.txt              # Root build file
├── .clang-format               # Code formatting config
├── .clang-tidy                 # Static analysis config
├── common/                     # Shared library (logger, timer)
│   ├── include/common/
│   │   ├── logger.h            # Thread-safe colored logging
│   │   └── timer.h             # High-precision timer
│   └── src/
│       ├── logger.cpp
│       └── timer.cpp
├── reference/                  # 📖 Reference implementations
│   ├── concurrency/            # Multithreading & concurrency
│   │   ├── thread_basics.cpp       # Thread creation & management
│   │   ├── mutex_and_lock.cpp      # Mutexes & lock types
│   │   ├── condition_variable.cpp  # Condition variables & producer-consumer
│   │   ├── thread_pool.cpp         # Complete thread pool
│   │   ├── atomic_operations.cpp   # Atomics & lock-free programming
│   │   └── async_and_future.cpp    # std::async & std::future
│   ├── design_patterns/        # Design patterns
│   │   ├── singleton.cpp           # Singleton (3 variants)
│   │   ├── factory.cpp             # Factory (registry-based)
│   │   ├── observer.cpp            # Observer & event system
│   │   └── strategy.cpp            # Strategy pattern
│   ├── memory_management/      # Memory management
│   │   ├── smart_pointers.cpp      # Smart pointer deep dive
│   │   ├── move_semantics.cpp      # Move semantics & perfect forwarding
│   │   └── raii.cpp                # RAII resource management
│   ├── modern_cpp/             # Modern C++ features
│   │   ├── templates_and_concepts.cpp  # Templates & C++20 Concepts
│   │   ├── lambda_and_functional.cpp   # Lambda & functional programming
│   │   └── coroutines.cpp             # C++20 Coroutines
│   ├── metaprogramming/        # Metaprogramming
│   │   └── type_traits_and_sfinae.cpp  # Type traits & SFINAE
│   └── performance/            # Performance optimization
│       └── cache_friendly.cpp      # Cache-friendly programming
├── src/                        # ✏️ Practice code (your implementations)
│   ├── concurrency/
│   ├── design_patterns/
│   ├── memory_management/
│   ├── modern_cpp/
│   ├── metaprogramming/
│   └── performance/
├── tests/                      # Unit tests
├── benchmarks/                 # Performance benchmarks
├── docs/                       # Documentation
│   └── learning_roadmap.md     # Learning roadmap
├── scripts/                    # Utility scripts
│   ├── build.sh                # Build script
│   ├── clean.sh                # Clean script
│   └── run.sh                  # Run script
└── .vscode/                    # VS Code configuration
    ├── launch.json             # F5 debug config
    ├── tasks.json              # Build tasks
    ├── settings.json           # Editor settings
    └── extensions.json         # Recommended extensions
```

## 🛠️ Prerequisites

- **Compiler**: GCC 10+ or Clang 12+ (C++20 support required)
- **Build system**: CMake 3.16+
- **Debugger**: GDB
- **OS**: Linux (Ubuntu 20.04+)

## 🚀 Quick Start

### 1. Clone

```bash
git clone https://github.com/your-username/learning_cpp.git
cd learning_cpp
```

### 2. Build

```bash
# Option 1: Build script (recommended)
chmod +x scripts/*.sh
./scripts/build.sh debug      # Debug mode (sanitizers enabled)
./scripts/build.sh release    # Release mode (optimized)

# Option 2: Manual CMake
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel $(nproc)
```

### 3. Run Examples

```bash
# Run reference implementations
./bin/reference/concurrency/thread_basics
./bin/reference/design_patterns/singleton
./bin/reference/performance/cache_friendly

# Run tests
./bin/tests/test_logger

# Run benchmarks
./bin/benchmarks/bench_containers

# Using the run script
./scripts/run.sh thread_pool
```

### 4. Debug in VS Code

1. Open any `.cpp` file
2. Press `F5` — auto-builds and debugs the current file
3. Breakpoints, variable inspection, call stack — all work out of the box

## 📚 How to Use

### Recommended Workflow

1. **Read** the reference code under `reference/` (detailed comments included)
2. **Understand** the core concepts and their AI/ML applications
3. **Implement** your own version under `src/` in the corresponding directory
4. **Compare** your implementation against the reference
5. **Experiment** — tweak parameters, test edge cases, observe behavior

### Learning Roadmap

See [docs/learning_roadmap.md](docs/learning_roadmap.md)

Suggested order: **Memory Management → Concurrency → Design Patterns → Modern C++ → Metaprogramming → Performance**

## 🏗️ Features

| Feature           | Description                                               |
| ----------------- | --------------------------------------------------------- |
| 🎨 Colored logging | Timestamped, thread-ID, file:line colored terminal output |
| ⏱️ Scoped timer    | RAII-based auto-timing for performance measurement        |
| 🔧 Modern CMake    | Each source file compiles to its own executable           |
| 🐛 Debug support   | AddressSanitizer + UBSanitizer + GDB                      |
| 📏 Code standards  | clang-format + clang-tidy configured                      |
| 📊 Benchmarks      | Built-in performance comparison framework                 |
| 🧪 Unit tests      | Test framework ready                                      |
| 📖 Annotated code  | Each concept explained with real-world analogies          |

## 🔧 Common Commands

```bash
# Build
./scripts/build.sh debug

# Clean
./scripts/clean.sh

# Format code
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Static analysis
clang-tidy src/**/*.cpp -- -std=c++20 -I common/include

# Run all tests
find build/bin/tests -type f -executable -exec {} \;
```

## 📝 Adding New Topics

1. Create a reference implementation under `reference/<category>/`
2. Create a practice template under `src/<category>/`
3. Rebuild: `./scripts/build.sh debug`
4. New files are auto-discovered by CMake

## License

MIT
