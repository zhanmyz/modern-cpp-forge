/**
 * @file strategy.cpp
 * @brief Strategy Pattern - Runtime algorithm switching
 *
 * [Learning Objectives]
 * 1. Understand strategy pattern: encapsulate algorithms as interchangeable strategy objects
 * 2. Learn to eliminate excessive if-else with strategy pattern
 * 3. Use templates for compile-time strategies in C++ (zero overhead)
 *
 * [Core Concept - Analogy]
 * Getting to the airport:
 * - Strategy A: Taxi (fast but expensive)
 * - Strategy B: Subway (cheap but slow)
 * - Strategy C: Bicycle (free but slowest)
 * Same goal (reach airport), but methods (strategies) vary by situation
 *
 * [Applications in AI/ML]
 * - Optimizer strategy: switch Adam/SGD/AdaGrad during training
 * - Quantization strategy: INT8/FP16/mixed precision
 * - Scheduling strategy: Round-Robin/Priority/Least-Loaded
 * - Sampling strategy: Top-K/Top-P/Greedy/Beam Search
 */

#include <common/logger.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

// ============================================================================
// Method 1: Runtime Strategy (virtual functions, slight overhead but flexible)
// ============================================================================

// Strategy interface: text tokenizer
class Tokenizer {
   public:
    virtual ~Tokenizer() = default;
    virtual std::vector<std::string> tokenize(const std::string& text) = 0;
    virtual std::string name() const = 0;
};

// Concrete Strategy 1: whitespace tokenizer
class WhitespaceTokenizer : public Tokenizer {
   public:
    std::vector<std::string> tokenize(const std::string& text) override {
        std::vector<std::string> tokens;
        std::string token;
        for (char c : text) {
            if (c == ' ' || c == '\t' || c == '\n') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        if (!token.empty())
            tokens.push_back(token);
        return tokens;
    }
    std::string name() const override { return "WhitespaceTokenizer"; }
};

// Concrete Strategy 2: character tokenizer
class CharTokenizer : public Tokenizer {
   public:
    std::vector<std::string> tokenize(const std::string& text) override {
        std::vector<std::string> tokens;
        for (char c : text) {
            tokens.push_back(std::string(1, c));
        }
        return tokens;
    }
    std::string name() const override { return "CharTokenizer"; }
};

// Concrete Strategy 3: N-gram tokenizer
class NgramTokenizer : public Tokenizer {
   public:
    explicit NgramTokenizer(size_t n) : n_(n) {}

    std::vector<std::string> tokenize(const std::string& text) override {
        std::vector<std::string> tokens;
        if (text.size() < n_) {
            tokens.push_back(text);
            return tokens;
        }
        for (size_t i = 0; i <= text.size() - n_; ++i) {
            tokens.push_back(text.substr(i, n_));
        }
        return tokens;
    }
    std::string name() const override { return "NgramTokenizer(n=" + std::to_string(n_) + ")"; }

   private:
    size_t n_;
};

// Context class using strategy
class TextProcessor {
   public:
    void set_tokenizer(std::unique_ptr<Tokenizer> tokenizer) { tokenizer_ = std::move(tokenizer); }

    void process(const std::string& text) {
        if (!tokenizer_) {
            LOG_ERROR("  No tokenizer strategy set!");
            return;
        }
        LOG_INFO_FMT("  Using " << tokenizer_->name() << " Processing: \"" << text << "\"");
        auto tokens = tokenizer_->tokenize(text);
        std::string result;
        for (const auto& t : tokens) {
            result += "[" + t + "] ";
        }
        LOG_INFO_FMT("  Result: " << result);
    }

   private:
    std::unique_ptr<Tokenizer> tokenizer_;
};

// ============================================================================
// Method 2: std::function strategy (lightweight, for simple cases)
// ============================================================================
class Sorter {
   public:
    using CompareStrategy = std::function<bool(int, int)>;

    void set_strategy(CompareStrategy strategy, const std::string& name) {
        strategy_ = std::move(strategy);
        strategy_name_ = name;
    }

    void sort(std::vector<int>& data) {
        if (!strategy_)
            return;
        std::sort(data.begin(), data.end(), strategy_);
        LOG_INFO_FMT("  [" << strategy_name_ << "] Sort result: ");
        std::string result;
        for (int v : data)
            result += std::to_string(v) + " ";
        LOG_INFO_FMT("    " << result);
    }

   private:
    CompareStrategy strategy_;
    std::string strategy_name_;
};

// ============================================================================
// Method 3: Compile-time strategy (templates, zero runtime overhead)
// ============================================================================
struct GreedySampling {
    static int sample(const std::vector<float>& probs) {
        // Greedy: pick highest probability
        return static_cast<int>(
            std::distance(probs.begin(), std::max_element(probs.begin(), probs.end())));
    }
    static std::string name() { return "Greedy"; }
};

struct TopKSampling {
    static int sample(const std::vector<float>& probs) {
        // Simplified Top-K: pick from top 3
        std::vector<size_t> indices(probs.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::partial_sort(indices.begin(), indices.begin() + 3, indices.end(),
                          [&probs](size_t a, size_t b) { return probs[a] > probs[b]; });
        return static_cast<int>(indices[0]);  // Simplified: pick first
    }
    static std::string name() { return "TopK"; }
};

template <typename SamplingStrategy>
class TextGenerator {
   public:
    int generate_next_token(const std::vector<float>& probs) {
        int token_id = SamplingStrategy::sample(probs);
        LOG_INFO_FMT("  [" << SamplingStrategy::name() << "] Selected token: " << token_id);
        return token_id;
    }
};

// ============================================================================
// Usage Examples
// ============================================================================
void example_runtime_strategy() {
    LOG_INFO("=== Example 1: Runtime Strategy Switching ===");

    TextProcessor processor;

    // Switch tokenizer strategies
    processor.set_tokenizer(std::make_unique<WhitespaceTokenizer>());
    processor.process("Hello World AI");

    processor.set_tokenizer(std::make_unique<CharTokenizer>());
    processor.process("ABC");

    processor.set_tokenizer(std::make_unique<NgramTokenizer>(2));
    processor.process("Hello");
}

void example_function_strategy() {
    LOG_INFO("=== Example 2: std::function Strategy ===");

    Sorter sorter;
    std::vector<int> data = {5, 2, 8, 1, 9, 3};

    sorter.set_strategy([](int a, int b) { return a < b; }, "Ascending");
    sorter.sort(data);

    sorter.set_strategy([](int a, int b) { return a > b; }, "Descending");
    sorter.sort(data);

    sorter.set_strategy([](int a, int b) { return (a % 2) < (b % 2); }, "Even-first");
    sorter.sort(data);
}

void example_compiletime_strategy() {
    LOG_INFO("=== Example 3: Compile-time Strategy (Zero Overhead) ===");

    std::vector<float> probs = {0.1f, 0.3f, 0.05f, 0.5f, 0.05f};

    TextGenerator<GreedySampling> greedy_gen;
    greedy_gen.generate_next_token(probs);

    TextGenerator<TopKSampling> topk_gen;
    topk_gen.generate_next_token(probs);
}

int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Strategy Pattern");
    LOG_INFO("========================================");

    example_runtime_strategy();
    example_function_strategy();
    example_compiletime_strategy();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete!");
    LOG_INFO("========================================");

    return 0;
}
