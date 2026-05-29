/**
 * @file smart_pointers.cpp
 * @brief Smart Pointers - Automatic memory management, no more leaks
 *
 * [Learning Objectives]
 * 1. Understand three smart pointer types: unique_ptr, shared_ptr, weak_ptr
 * 2. Know when to use which smart pointer
 * 3. Understand ownership concepts
 *
 * [Core Concept - Analogy]
 * unique_ptr = exclusive ownership (your phone belongs only to you)
 * shared_ptr = shared ownership (shared apartment, last one out turns off lights)
 * weak_ptr   = observe without owning (you know your friend lives there, but you're not a tenant)
 *
 * [Why not raw pointers?]
 * - Forget delete → memory leak
 * - Double delete → crash
 * - When exceptions occur → delete never executes
 * Smart pointers solve all these problems!
 *
 * [Applications in AI/ML]
 * - unique_ptr: manage model weight ownership
 * - shared_ptr: multiple inference threads share one model instance
 * - weak_ptr: cache systems (objects may have been freed)
 */

#include <common/logger.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// A simulated GPU buffer class
class GPUBuffer {
   public:
    GPUBuffer(size_t size, const std::string& name) : size_(size), name_(name) {
        LOG_INFO_FMT("  [GPU] Allocated " << name_ << " (" << size_ << " bytes)");
    }

    ~GPUBuffer() { LOG_INFO_FMT("  [GPU] Freed " << name_ << " (" << size_ << " bytes)"); }

    void process() { LOG_INFO_FMT("  [GPU] Processing " << name_); }

    size_t size() const { return size_; }
    const std::string& name() const { return name_; }

   private:
    size_t size_;
    std::string name_;
};

// ============================================================================
// Example 1: std::unique_ptr - exclusive ownership
// ============================================================================
void example_unique_ptr() {
    LOG_INFO("=== Example 1: unique_ptr exclusive ownership ===");

    // Create unique_ptr (prefer make_unique)
    auto buffer = std::make_unique<GPUBuffer>(1024, "weights");
    buffer->process();

    // unique_ptr Cannot copy (exclusive!)
    // auto buffer2 = buffer;  // Compile error!

    // But can be moved (transfer ownership)
    auto buffer2 = std::move(buffer);
    // Now buffer is nullptr, buffer2 owns the resource
    LOG_INFO_FMT("  buffer is null: " << (buffer == nullptr ? "yes" : "no"));
    buffer2->process();

    // Auto-freed on scope exit (even if exception occurs)
    LOG_INFO("  About to leave scope, auto-freeing...");
}

// ============================================================================
// Example 2: std::shared_ptr - shared ownership
// ============================================================================
void example_shared_ptr() {
    LOG_INFO("=== Example 2: shared_ptr shared ownership ===");

    // Create shared_ptr
    auto model = std::make_shared<GPUBuffer>(4096, "model_weights");
    LOG_INFO_FMT("  Reference count: " << model.use_count());  // 1

    {
        // Copy shared_ptr (reference count +1)
        auto model_copy = model;
        LOG_INFO_FMT("  After copy, ref count: " << model.use_count());  // 2

        auto model_copy2 = model;
        LOG_INFO_FMT("  After another copy, ref count: " << model.use_count());  // 3

        model_copy->process();  // All copies can be used

        // model_copy and model_copy2 leave scope, ref count -2
    }

    LOG_INFO_FMT("  After scope exit, ref count: " << model.use_count());  // 1
    // Resource freed only when last shared_ptr is destroyed
}

// ============================================================================
// Example 3: std::weak_ptr - Observer that does not increase reference count
// ============================================================================
void example_weak_ptr() {
    LOG_INFO("=== Example 3: weak_ptr Weak Reference ===");

    std::weak_ptr<GPUBuffer> cache;  // Cache: observe without owning

    {
        auto buffer = std::make_shared<GPUBuffer>(2048, "cached_data");
        cache = buffer;  // weak_ptr does not increase reference count

        LOG_INFO_FMT("  Ref count (weak excluded): " << buffer.use_count());  // still 1

        // Must lock() weak_ptr to get shared_ptr before use
        if (auto locked = cache.lock()) {
            locked->process();
            LOG_INFO("  Cache hit! Object still exists");
        }
    }
    // buffer destroyed (reference count reached 0)

    // Attempt to access destroyed object
    if (auto locked = cache.lock()) {
        LOG_INFO("  This won't execute");
    } else {
        LOG_WARNING("  Cache miss: object has been freed");
    }

    LOG_INFO_FMT("  weak_ptr expired: " << (cache.expired() ? "yes" : "no"));
}

// ============================================================================
// Example 4: Practical: Model sharing and caching
// ============================================================================
class ModelCache {
   public:
    // Get or load model (using weak_ptr cache)
    std::shared_ptr<GPUBuffer> get_model(const std::string& name) {
        auto it = cache_.find(name);
        if (it != cache_.end()) {
            if (auto model = it->second.lock()) {
                LOG_INFO_FMT("  [Cache] Hit: " << name);
                return model;
            }
            // weak_ptr expired, removing
            cache_.erase(it);
        }

        // Cache miss, creating new model
        LOG_INFO_FMT("  [Cache] Miss, loading: " << name);
        auto model = std::make_shared<GPUBuffer>(8192, name);
        cache_[name] = model;  // Store as weak_ptr
        return model;
    }

   private:
    std::unordered_map<std::string, std::weak_ptr<GPUBuffer>> cache_;
};

void example_model_cache() {
    LOG_INFO("=== Example 4: Model Cache System ===");

    ModelCache cache;

    auto model1 = cache.get_model("bert");  // Miss, load
    auto model2 = cache.get_model("bert");  // Hit!

    model1.reset();                         // Release one reference
    auto model3 = cache.get_model("bert");  // model2still alive, cache hit

    model2.reset();                         // Release last reference
    model3.reset();                         // bert model freed
    auto model4 = cache.get_model("bert");  // miss, reload
}

// ============================================================================
// Example 5: Common Pitfalls & Best Practices
// ============================================================================
void example_best_practices() {
    LOG_INFO("=== Example 5: Best Practices ===");

    // ✅ Recommended: use make_unique/make_shared
    auto good = std::make_unique<GPUBuffer>(100, "good");

    // ❌ Not recommended: raw new (possible leak)
    // std::unique_ptr<GPUBuffer> bad(new GPUBuffer(100, "bad"));

    // ✅ Function params: use raw pointer/reference when ownership not needed
    auto use_buffer = [](const GPUBuffer& buf) { LOG_INFO_FMT("  Using buffer: " << buf.name()); };
    use_buffer(*good);

    // ✅ Use unique_ptr to transfer ownership
    auto take_ownership = [](std::unique_ptr<GPUBuffer> buf) {
        LOG_INFO_FMT("  Taking ownership of buffer: " << buf->name());
        // buf Auto-freed at function end
    };
    take_ownership(std::move(good));
    // Now good is nullptr

    LOG_INFO("  Remember: prefer unique_ptr over shared_ptr (faster, clearer)");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Smart Pointers");
    LOG_INFO("========================================");

    example_unique_ptr();
    example_shared_ptr();
    example_weak_ptr();
    example_model_cache();
    example_best_practices();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
