/**
 * @file factory.cpp
 * @brief Factory Pattern - Flexible object creation without specifying concrete classes
 *
 * [Learning Objectives]
 * 1. Understand Simple Factory, Factory Method, and Abstract Factory patterns
 * 2. Learn to decouple object creation from usage with factory pattern
 * 3. Master Registry Factory (most practical form)
 *
 * [Core Concept - Analogy]
 * Imagine ordering at a restaurant:
 * - You just say "I want a steak" (tell the factory what you want)
 * - The chef (factory) handles the details (Create an object details)
 * - You don't need to know how the steak is cut or grilled
 * Benefit: if you change chefs (implementations), your ordering stays the same
 *
 * [Applications in AI/ML]
 * - Model factory:create different AI models from config (BERT, GPT, LLaMA)
 * - Optimizer factory: create by name Adam, SGD, AdaGrad, etc.
 * - Data loader factory:create different loaders based on data format DataLoader
 * - Inference Engine Factory:select based on hardware CPU/GPU/TPU inference backend
 */

#include <common/logger.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

// ============================================================================
// Define abstract base class (interface)
// ============================================================================

/**
 * @brief Abstract interface for AI models
 * All concrete models must implement this interface
 */
class Model {
   public:
    virtual ~Model() = default;
    virtual std::string predict(const std::string& input) = 0;
    virtual std::string name() const = 0;
    virtual size_t parameter_count() const = 0;
};

// ============================================================================
// Concrete model implementations
// ============================================================================

class BertModel : public Model {
   public:
    std::string predict(const std::string& input) override {
        return "[BERT] Classification result: positive (input: " + input + ")";
    }
    std::string name() const override { return "BERT-base"; }
    size_t parameter_count() const override { return 110'000'000; }  // 1.1 parameters
};

class GPTModel : public Model {
   public:
    std::string predict(const std::string& input) override {
        return "[GPT] Generated: The answer to '" + input + "' is 42.";
    }
    std::string name() const override { return "GPT-2"; }
    size_t parameter_count() const override { return 1'500'000'000; }  // 15 parameters
};

class LlamaModel : public Model {
   public:
    std::string predict(const std::string& input) override {
        return "[LLaMA] Response: '" + input + "' -> That is a great question...";
    }
    std::string name() const override { return "LLaMA-7B"; }
    size_t parameter_count() const override { return 7'000'000'000; }  // 70 parameters
};

// ============================================================================
// Method 1: Simple Factory
// ============================================================================
class SimpleModelFactory {
   public:
    /**
     * @brief Create model from type string
     * Drawback: must modify this function for each new model (violates Open/Closed principle)
     */
    static std::unique_ptr<Model> create(const std::string& type) {
        if (type == "bert")
            return std::make_unique<BertModel>();
        if (type == "gpt")
            return std::make_unique<GPTModel>();
        if (type == "llama")
            return std::make_unique<LlamaModel>();
        return nullptr;
    }
};

// ============================================================================
// Method 2: Registry Factory - Recommended!
// ============================================================================
/**
 * @brief Registry-based Model factory
 *
 * Benefits:
 * - Adding models requires no factory code changes (Open/Closed Principle)
 * - Models can be registered dynamically at runtime
 * - Perfect for plugin architectures
 */
class ModelFactory {
   public:
    // Type alias: creator function type
    using Creator = std::function<std::unique_ptr<Model>()>;

    // Get factory singleton
    static ModelFactory& instance() {
        static ModelFactory factory;
        return factory;
    }

    // Register a model type
    void register_model(const std::string& name, Creator creator) {
        creators_[name] = std::move(creator);
        LOG_INFO_FMT("  [Factory] Registered model: " << name);
    }

    // Create model
    std::unique_ptr<Model> create(const std::string& name) const {
        auto it = creators_.find(name);
        if (it == creators_.end()) {
            LOG_ERROR_FMT("  [Factory] Unknown model type: " << name);
            return nullptr;
        }
        return it->second();  // Invoke creator function
    }

    // List all available models
    std::vector<std::string> available_models() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : creators_) {
            names.push_back(name);
        }
        return names;
    }

   private:
    ModelFactory() = default;
    std::unordered_map<std::string, Creator> creators_;
};

// ============================================================================
// Auto-registration macro - Elegant registration
// ============================================================================
/**
 * This macro auto-registers models before main()
 * Usage: REGISTER_MODEL("name", ClassName)
 */
#define REGISTER_MODEL(name, cls)                                                                \
    static bool _registered_##cls = []() {                                                       \
        ModelFactory::instance().register_model(name, []() { return std::make_unique<cls>(); }); \
        return true;                                                                             \
    }()

// Auto-register all models
REGISTER_MODEL("bert", BertModel);
REGISTER_MODEL("gpt", GPTModel);
REGISTER_MODEL("llama", LlamaModel);

// ============================================================================
// Usage Examples
// ============================================================================
void example_simple_factory() {
    LOG_INFO("=== Example 1: Simple Factory ===");

    auto model = SimpleModelFactory::create("bert");
    if (model) {
        LOG_INFO_FMT("  Model: " << model->name() << " (" << model->parameter_count() << " params)");
        LOG_INFO_FMT("  Predict: " << model->predict("This movie is great"));
    }
}

void example_registry_factory() {
    LOG_INFO("=== Example 2: Registry Factory (Recommended) ===");

    auto& factory = ModelFactory::instance();

    // List available models
    LOG_INFO("  Available models:");
    for (const auto& name : factory.available_models()) {
        LOG_INFO_FMT("    - " << name);
    }

    // Dynamically create models from config
    std::vector<std::string> configs = {"bert", "gpt", "llama", "unknown"};
    for (const auto& config : configs) {
        auto model = factory.create(config);
        if (model) {
            LOG_INFO_FMT("  Created " << model->name() << " successfully");
            LOG_INFO_FMT("  " << model->predict("Hello World"));
        }
    }
}

void example_runtime_register() {
    LOG_INFO("=== Example 3: Runtime dynamic registration ===");

    // Simulating a plugin that registers a new model at runtime
    ModelFactory::instance().register_model("custom_model", []() {
        // Can use lambda to create any object satisfying the Model interface
        class CustomModel : public Model {
           public:
            std::string predict(const std::string& input) override {
                return "[Custom] Processing: " + input;
            }
            std::string name() const override { return "CustomModel-v1"; }
            size_t parameter_count() const override { return 1000; }
        };
        return std::make_unique<CustomModel>();
    });

    auto model = ModelFactory::instance().create("custom_model");
    if (model) {
        LOG_INFO_FMT("  Dynamically registered model: " << model->name());
        LOG_INFO_FMT("  " << model->predict("test input"));
    }
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Factory Pattern");
    LOG_INFO("========================================");

    example_simple_factory();
    example_registry_factory();
    example_runtime_register();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete!");
    LOG_INFO("========================================");

    return 0;
}
