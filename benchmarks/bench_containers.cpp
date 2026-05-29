/**
 * @file bench_containers.cpp
 * @brief STL container performance benchmarks
 *
 * Compares insertion, lookup, and iteration performance across different containers.
 */

#include <common/logger.h>
#include <common/timer.h>

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

const int N = 1000000;

void bench_insertion() {
    LOG_INFO("=== Insertion Performance (N=" + std::to_string(N) + ") ===");

    {
        SCOPED_TIMER("vector push_back");
        std::vector<int> v;
        for (int i = 0; i < N; ++i)
            v.push_back(i);
    }
    {
        SCOPED_TIMER("vector (reserved) push_back");
        std::vector<int> v;
        v.reserve(static_cast<size_t>(N));
        for (int i = 0; i < N; ++i)
            v.push_back(i);
    }
    {
        SCOPED_TIMER("deque push_back");
        std::deque<int> d;
        for (int i = 0; i < N; ++i)
            d.push_back(i);
    }
    {
        SCOPED_TIMER("list push_back");
        std::list<int> l;
        for (int i = 0; i < N; ++i)
            l.push_back(i);
    }
    {
        SCOPED_TIMER("set insert");
        std::set<int> s;
        for (int i = 0; i < N; ++i)
            s.insert(i);
    }
    {
        SCOPED_TIMER("unordered_set insert");
        std::unordered_set<int> s;
        for (int i = 0; i < N; ++i)
            s.insert(i);
    }
}

void bench_lookup() {
    LOG_INFO("=== Lookup Performance ===");

    std::vector<int> vec(static_cast<size_t>(N));
    std::iota(vec.begin(), vec.end(), 0);
    std::set<int> ordered_set(vec.begin(), vec.end());
    std::unordered_set<int> hash_set(vec.begin(), vec.end());

    std::mt19937 gen(42);
    std::vector<int> queries(10000);
    for (auto& q : queries)
        q = static_cast<int>(gen() % static_cast<unsigned>(N));

    {
        SCOPED_TIMER("vector (sorted) binary_search x10000");
        volatile int found = 0;
        for (int q : queries) {
            found += static_cast<int>(std::binary_search(vec.begin(), vec.end(), q));
        }
        (void)found;
    }
    {
        SCOPED_TIMER("set find x10000");
        volatile int found = 0;
        for (int q : queries) {
            found += static_cast<int>(ordered_set.count(q));
        }
        (void)found;
    }
    {
        SCOPED_TIMER("unordered_set find x10000");
        volatile int found = 0;
        for (int q : queries) {
            found += static_cast<int>(hash_set.count(q));
        }
        (void)found;
    }
}

void bench_iteration() {
    LOG_INFO("=== Iteration Performance ===");

    std::vector<int> vec(static_cast<size_t>(N));
    std::iota(vec.begin(), vec.end(), 0);
    std::list<int> lst(vec.begin(), vec.end());

    {
        SCOPED_TIMER("vector iteration sum");
        volatile long long sum = 0;
        for (int v : vec)
            sum += v;
        (void)sum;
    }
    {
        SCOPED_TIMER("list iteration sum");
        volatile long long sum = 0;
        for (int v : lst)
            sum += v;
        (void)sum;
    }
}

int main() {
    LOG_INFO("========================================");
    LOG_INFO("  STL Container Benchmarks");
    LOG_INFO("========================================");

    bench_insertion();
    bench_lookup();
    bench_iteration();

    LOG_INFO("========================================");
    LOG_INFO("  Benchmarks complete");
    LOG_INFO("========================================");

    return 0;
}
