/**
 * @file observer.cpp
 * @brief Observer Pattern - Event-driven publish/subscribe
 *
 * [Learning Objectives]
 * 1. Understand observer pattern: auto-notify subscribers when state changes
 * 2. Implement a type-safe event system
 * 3. Learn to decouple event publishers and subscribers
 *
 * [Core Concept - Analogy]
 * Imagine a newsletter subscription:
 * - The publisher (Subject/Publisher) posts a new article
 * - All subscribers (Observer/Subscriber) get notified
 * - Subscribers can unsubscribe anytime
 * - The publisher doesn't need to know details about its subscribers
 *
 * [Applications in AI/ML]
 * - Training: notify visualizer, early stopping, and LR scheduler on loss change
 * - Inference: notify logging, monitoring, and billing on request completion
 * - Model update: notify all inference instances for hot-swap when new model is ready
 */

#include <common/logger.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================================
// Method 1: Classic OOP Observer Pattern
// ============================================================================

// Observer interface
class TrainingObserver {
   public:
    virtual ~TrainingObserver() = default;
    virtual void on_epoch_end(int epoch, float loss, float accuracy) = 0;
    virtual void on_training_complete() = 0;
};

// Subject (Observable) - Trainer
class Trainer {
   public:
    void add_observer(std::shared_ptr<TrainingObserver> observer) {
        observers_.push_back(observer);
    }

    void remove_observer(const std::shared_ptr<TrainingObserver>& observer) {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), observer),
                         observers_.end());
    }

    // Simulate training process
    void train(int epochs) {
        LOG_INFO_FMT("  Starting training, total " << epochs << " epochs");
        for (int e = 1; e <= epochs; ++e) {
            // Simulate training
            float loss = 1.0f / static_cast<float>(e);
            float acc = 1.0f - loss * 0.5f;

            // Notify all observers
            for (auto& obs : observers_) {
                obs->on_epoch_end(e, loss, acc);
            }
        }
        for (auto& obs : observers_) {
            obs->on_training_complete();
        }
    }

   private:
    std::vector<std::shared_ptr<TrainingObserver>> observers_;
};

// Concrete Observer 1: Logger
class LoggingObserver : public TrainingObserver {
   public:
    void on_epoch_end(int epoch, float loss, float accuracy) override {
        LOG_INFO_FMT("  [Logger] Epoch " << epoch << ": loss=" << loss << ", acc=" << accuracy);
    }
    void on_training_complete() override { LOG_INFO("  [Logger] Training complete!"); }
};

// Concrete Observer 2: Early Stopping Checker
class EarlyStopObserver : public TrainingObserver {
   public:
    explicit EarlyStopObserver(float threshold) : threshold_(threshold) {}

    void on_epoch_end(int epoch, float loss, float /*accuracy*/) override {
        if (loss < threshold_) {
            LOG_INFO_FMT("  [EarlyStop] Epoch " << epoch << ": loss=" << loss << " < " << threshold_
                                           << " - recommend stopping training");
        }
    }
    void on_training_complete() override {}

   private:
    float threshold_;
};

// ============================================================================
// Method 2: Modern C++ Event System (std::function, more flexible)
// ============================================================================
template <typename... Args>
class Event {
   public:
    using Handler = std::function<void(Args...)>;
    using HandlerId = size_t;

    // Subscribe to event, returns handle ID for unsubscription
    HandlerId subscribe(Handler handler) {
        HandlerId id = next_id_++;
        handlers_[id] = std::move(handler);
        return id;
    }

    // Unsubscribe
    void unsubscribe(HandlerId id) { handlers_.erase(id); }

    // Emit event, notify all subscribers
    void emit(Args... args) const {
        for (const auto& [id, handler] : handlers_) {
            handler(args...);
        }
    }

    // Get subscriber count
    size_t subscriber_count() const { return handlers_.size(); }

   private:
    std::unordered_map<HandlerId, Handler> handlers_;
    HandlerId next_id_ = 0;
};

// ============================================================================
// Usage Examples
// ============================================================================
void example_classic_observer() {
    LOG_INFO("=== Example 1: Classic Observer Pattern ===");

    Trainer trainer;

    auto logger = std::make_shared<LoggingObserver>();
    auto early_stop = std::make_shared<EarlyStopObserver>(0.3f);

    trainer.add_observer(logger);
    trainer.add_observer(early_stop);

    trainer.train(5);
}

void example_modern_event_system() {
    LOG_INFO("=== Example 2: Modern Event System ===");

    // Define event types
    Event<std::string, int> on_request_completed;  // params: request content, status code
    Event<> on_server_shutdown;                    // parameterless event

    // Subscriber 1: Logger
    auto log_id = on_request_completed.subscribe([](const std::string& req, int status) {
        LOG_INFO_FMT("  [Logger] Request: " << req << " -> Status: " << status);
    });

    // Subscriber 2: Monitor
    on_request_completed.subscribe([](const std::string& /*req*/, int status) {
        if (status >= 500) {
            LOG_ERROR_FMT("  [Monitor] Server error! Status: " << status);
        }
    });

    // Subscribe to shutdown event
    on_server_shutdown.subscribe([]() { LOG_INFO("  [Cleanup] Closing database connections..."); });
    on_server_shutdown.subscribe([]() { LOG_INFO("  [Cleanup] Saving cache to disk..."); });

    // Simulate event emission
    on_request_completed.emit("GET /api/models", 200);
    on_request_completed.emit("POST /api/predict", 500);

    // Unsubscribe logger
    on_request_completed.unsubscribe(log_id);
    LOG_INFO("  (Logger unsubscribed)");
    on_request_completed.emit("GET /api/health", 200);

    // Emit shutdown event
    LOG_INFO("  Server shutting down...");
    on_server_shutdown.emit();
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Observer Pattern");
    LOG_INFO("========================================");

    example_classic_observer();
    example_modern_event_system();

    LOG_INFO("========================================");
    LOG_INFO("  All examples complete!");
    LOG_INFO("========================================");

    return 0;
}
