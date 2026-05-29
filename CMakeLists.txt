cmake_minimum_required(VERSION 3.16)
project(modern-cpp-forge
    VERSION 1.0.0
    DESCRIPTION "Advanced C++ Learning Project"
    LANGUAGES CXX
)

# ============================================================================
# Global Settings
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Compiler Warnings
# ============================================================================
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wshadow -Wconversion -Wsign-conversion
    -Wnon-virtual-dtor -Wold-style-cast
    -Wcast-align -Wunused -Woverloaded-virtual
)

# Debug specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

# ============================================================================
# Threading
# ============================================================================
find_package(Threads REQUIRED)

# ============================================================================
# Subdirectories
# ============================================================================
add_subdirectory(common)
add_subdirectory(src)
add_subdirectory(reference)
add_subdirectory(tests)
add_subdirectory(benchmarks)
