# C++ Advanced Learning Roadmap

## Suggested Learning Order

Follow this sequence. Each topic has a corresponding reference implementation under `reference/`:

### Phase 1: Memory Management (Foundational)
1. **RAII** → `reference/memory_management/raii.cpp`
2. **Smart Pointers** → `reference/memory_management/smart_pointers.cpp`
3. **Move Semantics** → `reference/memory_management/move_semantics.cpp`

### Phase 2: Multithreading & Concurrency
4. **Thread Basics** → `reference/concurrency/thread_basics.cpp`
5. **Mutex & Locks** → `reference/concurrency/mutex_and_lock.cpp`
6. **Condition Variables** → `reference/concurrency/condition_variable.cpp`
7. **Atomic Operations** → `reference/concurrency/atomic_operations.cpp`
8. **Thread Pool** → `reference/concurrency/thread_pool.cpp`
9. **Async & Future** → `reference/concurrency/async_and_future.cpp`

### Phase 3: Design Patterns
10. **Singleton** → `reference/design_patterns/singleton.cpp`
11. **Factory** → `reference/design_patterns/factory.cpp`
12. **Observer** → `reference/design_patterns/observer.cpp`
13. **Strategy** → `reference/design_patterns/strategy.cpp`

### Phase 4: Modern C++ Features
14. **Lambda & Functional** → `reference/modern_cpp/lambda_and_functional.cpp`
15. **Templates & Concepts** → `reference/modern_cpp/templates_and_concepts.cpp`
16. **Coroutines** → `reference/modern_cpp/coroutines.cpp`

### Phase 5: Metaprogramming
17. **Type Traits & SFINAE** → `reference/metaprogramming/type_traits_and_sfinae.cpp`

### Phase 6: Performance Optimization
18. **Cache-Friendly Programming** → `reference/performance/cache_friendly.cpp`

---

## Study Method

1. Read the reference code and comments under `reference/`
2. Create your own implementation under `src/` in the corresponding directory
3. Build & run, compare output
4. Modify and experiment
5. Use benchmarks to verify performance differences

## Mapping to AI/ML Applications

| C++ Topic | AI/ML Application |
| --------- | ----------------- |
| Thread Pool | Concurrent inference serving |
| Move Semantics | Efficient large tensor transfer |
| Smart Pointers | Model lifecycle management |
| Singleton | Global model registry |
| Factory Pattern | Multi-model / multi-backend creation |
| Atomics | Lock-free statistics counters |
| Cache Optimization | Inference engine performance |
| Templates | Generic tensor implementation |
| Coroutines | Streaming token generation |
