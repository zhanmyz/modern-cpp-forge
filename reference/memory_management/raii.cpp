/**
 * @file raii.cpp
 * @brief RAII (Resource Acquisition Is Initialization) - Core C++ resource management
 *
 * [Learning Objectives]
 * 1. Understand RAII: acquire on init, release on destroy
 * 2. Learn to manage resources with RAII (memory, files, locks, network, etc.)
 * 3. Understand why RAII is one of C++'s most important features
 *
 * [Core Concept - Analogy]
 * RAII is like an automatic door:
 * - Walk in, door auto-opens (construction = acquire resource)
 * - Walk out, door auto-closes (destruction = release resource)
 * - Whether you leave normally or are evacuated (exception), the door always closes
 *
 * Without RAII (C-style):
 *   open_file() → do work → manually close_file() on error → close_file() on success too
 *   Easy to forget closing!
 *
 * With RAII (C++ style):
 *   File f("path") → do work → f auto-closes on scope exit, regardless of exceptions
 *
 * [Applications in AI/ML]
 * - GPU memory: allocate on init, auto-free on scope exit
 * - Model loading: RAII manages model lifecycle
 * - Temp files: auto-deleted after processing
 * - Timer: auto-start on entry, auto-report on exit
 */

#include <common/logger.h>
#include <common/timer.h>

#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// ============================================================================
// Example 1: RAII File Resource Management
// ============================================================================
class FileGuard {
   public:
    explicit FileGuard(const std::string& path, const std::string& mode = "r") : path_(path) {
        if (mode == "w") {
            file_.open(path, std::ios::out);
        } else {
            file_.open(path, std::ios::in);
        }
        if (file_.is_open()) {
            LOG_INFO_FMT("  [FileGuard] Opened file: " << path);
        } else {
            LOG_ERROR_FMT("  [FileGuard] Failed to open: " << path);
        }
    }

    ~FileGuard() {
        if (file_.is_open()) {
            file_.close();
            LOG_INFO_FMT("  [FileGuard] Auto-closed: " << path_);
        }
    }

    bool is_open() const { return file_.is_open(); }

    void write(const std::string& content) {
        if (file_.is_open()) {
            file_ << content;
        }
    }

    // Non-copyable (exclusive ownership)
    FileGuard(const FileGuard&) = delete;
    FileGuard& operator=(const FileGuard&) = delete;

   private:
    std::string path_;
    std::fstream file_;
};

void example_file_raii() {
    LOG_INFO("=== Example 1: RAII File Management ===");

    {
        FileGuard file("/tmp/test_raii.txt", "w");
        if (file.is_open()) {
            file.write("Hello RAII!\n");
            LOG_INFO("  Write successful");
        }
        // file auto-closed here!
    }
    LOG_INFO("  File auto-closed (scope exit)");
}

// ============================================================================
// Example 2: RAII GPU Memory Management (simulated)
// ============================================================================
class CudaMemory {
   public:
    CudaMemory(size_t bytes) : size_(bytes), ptr_(nullptr) {
        // Simulate cudaMalloc
        ptr_ = new char[bytes];
        LOG_INFO_FMT("  [CUDA] Allocated " << bytes << " bytes GPU memory");
    }

    ~CudaMemory() {
        if (ptr_) {
            // Simulate cudaFree
            delete[] ptr_;
            LOG_INFO_FMT("  [CUDA] Freed " << size_ << " bytes GPU memory");
        }
    }

    // Move Semantics
    CudaMemory(CudaMemory&& other) noexcept : size_(other.size_), ptr_(other.ptr_) {
        other.ptr_ = nullptr;
        other.size_ = 0;
    }

    CudaMemory& operator=(CudaMemory&& other) noexcept {
        if (this != &other) {
            delete[] ptr_;
            ptr_ = other.ptr_;
            size_ = other.size_;
            other.ptr_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    // Non-copyable
    CudaMemory(const CudaMemory&) = delete;
    CudaMemory& operator=(const CudaMemory&) = delete;

    void* get() { return ptr_; }
    size_t size() const { return size_; }

   private:
    size_t size_;
    char* ptr_;
};

void example_gpu_raii() {
    LOG_INFO("=== Example 2: RAII GPU Memory ===");

    {
        // Allocate GPU memory
        CudaMemory weights(1024 * 1024);     // 1MB
        CudaMemory activations(512 * 1024);  // 512KB

        LOG_INFO("  Computing with GPU memory...");

        // Even if exception thrown here, memory is properly freed
        // throw std::runtime_error("oops"); // Uncomment to try

    }  // weights and activations auto-freed here
    LOG_INFO("  All GPU memory auto-freed");
}

// ============================================================================
// Example 3: ScopeGuard - Universal RAII cleanup utility
// ============================================================================
class ScopeGuard {
   public:
    explicit ScopeGuard(std::function<void()> cleanup)
        : cleanup_(std::move(cleanup)), active_(true) {}

    ~ScopeGuard() {
        if (active_) {
            cleanup_();
        }
    }

    // Dismiss cleanup (no rollback needed on success)
    void dismiss() { active_ = false; }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    ScopeGuard(ScopeGuard&& other) noexcept
        : cleanup_(std::move(other.cleanup_)), active_(other.active_) {
        other.active_ = false;
    }

   private:
    std::function<void()> cleanup_;
    bool active_;
};

// Convenience macro
#define SCOPE_EXIT(code) ScopeGuard _scope_guard_##__LINE__([&]() { code; })

void example_scope_guard() {
    LOG_INFO("=== Example 3: ScopeGuard Universal Cleanup ===");

    // Simulate database transaction
    LOG_INFO("  Starting transaction...");

    ScopeGuard rollback([&]() { LOG_WARNING("  Transaction rolled back! (cleanup executed)"); });

    // Perform operations...
    bool success = true;
    LOG_INFO("  Executing operations...");

    if (success) {
        rollback.dismiss();  // Success, no rollback needed
        LOG_INFO("  Transaction committed (cleanup dismissed)");
    }
}

// ============================================================================
// Example 4: RAII Lock Management (this is how lock_guard works)
// ============================================================================
template <typename Mutex>
class MyLockGuard {
   public:
    explicit MyLockGuard(Mutex& mtx) : mutex_(mtx) {
        mutex_.lock();
        LOG_DEBUG("  [MyLockGuard] Locked");
    }

    ~MyLockGuard() {
        mutex_.unlock();
        LOG_DEBUG("  [MyLockGuard] Unlocked");
    }

    MyLockGuard(const MyLockGuard&) = delete;
    MyLockGuard& operator=(const MyLockGuard&) = delete;

   private:
    Mutex& mutex_;
};

void example_lock_raii() {
    LOG_INFO("=== Example 4: RAII Lock Management ===");

    std::mutex mtx;
    int shared_data = 0;

    {
        MyLockGuard<std::mutex> lock(mtx);
        shared_data = 42;
        LOG_INFO_FMT("  Modified shared data to: " << shared_data);
        // Even if exception thrown here, lock is released
    }  // auto-unlocked
    LOG_INFO("  Lock auto-released");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  RAII Resource Management");
    LOG_INFO("========================================");

    example_file_raii();
    example_gpu_raii();
    example_scope_guard();
    example_lock_raii();

    LOG_INFO("========================================");
    LOG_INFO("  Core principle: manage resources with object lifetimes!");
    LOG_INFO("  Construct = acquire, Destruct = release, never leak!");
    LOG_INFO("========================================");

    return 0;
}
