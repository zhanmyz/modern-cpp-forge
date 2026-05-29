# Build System Guide

This project uses **CMake** (3.16+) with a multi-target layout. Each `.cpp` file compiles into a standalone executable automatically — no manual target registration needed.

## Quick Start

```bash
# Configure (only needed once, or after adding new .cpp files)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build a specific target
cmake --build build --target <target_name>

# Build everything
cmake --build build
```

## Target Naming Convention

Since CMake requires globally unique target names, we use a prefix system:

```
<prefix>_<directory>_<filename>
```

| Prefix | Source Directory | Example File                        | Target Name                    |
| ------ | ---------------- | ----------------------------------- | ------------------------------ |
| `src_` | `src/`           | `src/modern_cpp/function_test.cpp`  | `src_modern_cpp_function_test` |
| `ref_` | `reference/`     | `reference/modern_cpp/function.cpp` | `ref_modern_cpp_function`      |

### Why target name ≠ binary name?

The **target name** (used with `--target`) is a CMake-internal identifier that must be globally unique. The **output binary name** is what actually gets written to disk.

```cmake
# In CMakeLists.txt:
set(FULL_TARGET_NAME "src_modern_cpp_function_test")   # unique target ID

set_target_properties(${FULL_TARGET_NAME} PROPERTIES
    OUTPUT_NAME ${TARGET_NAME}                          # binary = "function_test"
    RUNTIME_OUTPUT_DIRECTORY .../bin/src/modern_cpp/    # placed in category folder
)
```

Result:
- Build command: `cmake --build build --target src_modern_cpp_function_test`
- Binary on disk: `./bin/src/modern_cpp/function_test`

The directory structure already prevents name collisions, so the binary doesn't need the verbose prefix.

## Output Directory Layout

```
bin/
├── src/                          # Your practice code
│   ├── concurrency/
│   │   ├── thread_basics
│   │   └── future_test
│   ├── modern_cpp/
│   │   └── function_test
│   └── ...
└── reference/                    # Reference implementations
    ├── concurrency/
    │   ├── thread_basics
    │   └── mutex_and_lock
    └── modern_cpp/
        ├── function
        └── lambda_and_functional
```

## How Auto-Discovery Works

Both `src/CMakeLists.txt` and `reference/CMakeLists.txt` use `file(GLOB_RECURSE ...)` to find all `.cpp` files:

```cmake
file(GLOB_RECURSE SRC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrency/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/modern_cpp/*.cpp
    ...
)
```

**Important:** `GLOB_RECURSE` only scans at **configure** time. After adding a new `.cpp` file, you must re-run:

```bash
cmake -B build
```

Otherwise the new file won't be picked up by `cmake --build`.

## Adding a New Example

1. Create your file in the appropriate category:
   ```bash
   touch src/modern_cpp/my_new_example.cpp
   ```

2. Reconfigure to detect the new file:
   ```bash
   cmake -B build
   ```

3. Build and run:
   ```bash
   cmake --build build --target src_modern_cpp_my_new_example
   ./bin/src/modern_cpp/my_new_example
   ```

## List All Available Targets

```bash
cmake --build build --target help | grep -E "^\\.\\.\\."
```

Filter by category:
```bash
cmake --build build --target help | grep "src_modern_cpp"
cmake --build build --target help | grep "ref_concurrency"
```

## Build Types

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug      # with debug symbols (default)
cmake -B build -DCMAKE_BUILD_TYPE=Release     # optimized, no debug
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo  # optimized + debug symbols
```

## Compiler & Standard

- **Compiler:** GCC 11.4+ (or any C++20-compatible compiler)
- **Standard:** C++20 (`CMAKE_CXX_STANDARD 20`)
- **Warnings:** `-Wall -Wextra -Wpedantic`
