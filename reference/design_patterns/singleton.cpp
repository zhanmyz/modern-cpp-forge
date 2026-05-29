/**
 * @file singleton.cpp
 * @brief Singleton Pattern - Ensure a class has only one instance
 *
 * [Learning Objectives]
 * 1. Understand use cases for the Singleton pattern
 * 2. Master thread-safe Singleton implementations
 * 3. Learn Meyer's Singleton (recommended approach for C++11+)
 *
 * [Core Concept - Analogy]
 * A country has only one president (singleton); no matter where you refer to
 * "the president," it's always the same person. Programs have similar needs:
 * logging systems, database connection pools, config managers - all need one instance.
 *
 * [Applications in AI/ML]
 * - Model manager: load a large model only once globally
 * - Config center: all modules share one configuration
 * - Logging system: all threads use the same logger
 * - GPU resource manager: centralized GPU memory allocation
 */

#include <common/logger.h>

#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// ============================================================================
// Approach 1: Meyer's Singleton (Recommended! Thread-safe guaranteed by C++11)
// ============================================================================
class ConfigManager {
   public:
    // Get the unique instance
    // C++11 guarantees thread-safe initialization of local static variables
    static ConfigManager& instance() {
        static ConfigManager instance;  // Created on first call, reused thereafter
        return instance;
    }

    // Business methods
    void set(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_[key] = value;
    }

    std::string get(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : "";
    }

    // Non-copyable, non-movable (ensures singleton uniqueness)
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;

   private:
    // Private constructor - only accessible via instance()
    ConfigManager() { LOG_INFO("  [ConfigManager] Singleton created (prints only once)"); }

    ~ConfigManager() { LOG_INFO("  [ConfigManager] Singleton destroyed"); }

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::string> config_;
};

// ============================================================================
// Approach 2: Double-Checked Locking (DCLP) - For reference only
// Classic approach before C++11; Meyer's Singleton is preferred now
// ============================================================================
class LegacySingleton {
   public:
    static LegacySingleton* instance() {
        if (instance_ == nullptr) {  // First check (no lock, fast path)
            std::lock_guard<std::mutex> lock(mutex_);
            if (instance_ == nullptr) {  // Second check (locked, safe)
                instance_ = new LegacySingleton();
            }
        }
        return instance_;
    }

    static void destroy() {
        std::lock_guard<std::mutex> lock(mutex_);
        delete instance_;
        instance_ = nullptr;
    }

    void do_something() { LOG_INFO("  [LegacySingleton] Working..."); }

   private:
    LegacySingleton() = default;
    static LegacySingleton* instance_;
    static std::mutex mutex_;
};

LegacySingleton* LegacySingleton::instance_ = nullptr;
std::mutex LegacySingleton::mutex_;

// ============================================================================
// Approach 3: Template Singleton Base (Reusable)
// ============================================================================
template <typename T>
class Singleton {
   public:
    static T& instance() {
        static T instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

   protected:
    Singleton() = default;
    ~Singleton() = default;
};

// Using the template singleton: just inherit
class ModelManager : public Singleton<ModelManager> {
    friend class Singleton<ModelManager>;  // Allow base class to access private constructor
   public:
    void load_model(const std::string& path) {
        LOG_INFO_FMT("  [ModelManager] Loading model: " << path);
        model_path_ = path;
    }

    std::string get_model_path() const { return model_path_; }

   private:
    ModelManager() { LOG_INFO("  [ModelManager] Singleton created"); }
    std::string model_path_;
};

// ============================================================================
// Test Code
// ============================================================================
void example_meyers_singleton() {
    LOG_INFO("=== Example 1: Meyer's Singleton (Recommended) ===");

    // Multiple threads access singleton concurrently - thread-safe
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            auto& config = ConfigManager::instance();
            config.set("thread_" + std::to_string(i), "value_" + std::to_string(i));
        });
    }
    for (auto& t : threads)
        t.join();

    // Verify all threads operated on the same instance
    auto& config = ConfigManager::instance();
    for (int i = 0; i < 5; ++i) {
        std::string val = config.get("thread_" + std::to_string(i));
        LOG_INFO_FMT("  config[thread_" << i << "] = " << val);
    }
}

void example_template_singleton() {
    LOG_INFO("=== Example 2: Template Singleton ===");

    ModelManager::instance().load_model("/models/llama-7b");
    LOG_INFO_FMT("  Model path: " << ModelManager::instance().get_model_path());
}

int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Singleton Pattern");
    LOG_INFO("========================================");

    example_meyers_singleton();
    example_template_singleton();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete");
    LOG_INFO("========================================");

    return 0;
}
