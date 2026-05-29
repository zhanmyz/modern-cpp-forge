/**
 * @file cache_friendly.cpp
 * @brief Cache-Friendly Programming - Leveraging CPU cache for performance
 *
 * [Learning Objectives]
 * 1. Understand how CPU caches work
 * 2. Learn data layout optimization (SoA vs AoS)
 * 3. Master techniques to avoid false sharing
 *
 * [Core Concept - Analogy]
 * CPU cache is like your desk:
 * - Memory = bookshelf (large but slow to access)
 * - L1 cache = desk (small but fast to access)
 * - When fetching a book from shelf, nearby books come too (cache line = 64 bytes)
 * - If the next book needed is on the desk → cache hit (fast!)
 * - If not → cache miss (slow!)
 *
 * Key: keep related data together (contiguous memory), regular access patterns
 *
 * [Applications in AI/ML]
 * - Matrix multiplication tiling strategies
 * - Memory pool design
 * - Tensor data layout (NCHW vs NHWC)
 * - KV Cache layout in attention mechanisms
 */

#include <common/logger.h>
#include <common/timer.h>

#include <algorithm>
#include <array>
#include <random>
#include <vector>

// ============================================================================
// Example 1: Row vs Column Traversal (Cache Line Effect)
// ============================================================================
void example_row_vs_column() {
    LOG_INFO("=== Example 1: Row vs Column Traversal ===");

    const int N = 4096;
    std::vector<std::vector<int>> matrix(static_cast<size_t>(N),
                                         std::vector<int>(static_cast<size_t>(N), 1));

    // Row-major traversal (cache-friendly - sequential memory access)
    {
        SCOPED_TIMER("Row-major traversal");
        volatile long long sum = 0;
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                sum += matrix[static_cast<size_t>(i)][static_cast<size_t>(j)];
            }
        }
        (void)sum;
    }

    // Column-major traversal (cache-unfriendly - strided memory access)
    {
        SCOPED_TIMER("Column-major traversal");
        volatile long long sum = 0;
        for (int j = 0; j < N; ++j) {
            for (int i = 0; i < N; ++i) {
                sum += matrix[static_cast<size_t>(i)][static_cast<size_t>(j)];
            }
        }
        (void)sum;
    }

    LOG_INFO("  Row-major is typically 3-10x faster due to cache line locality!");
}

// ============================================================================
// Example 2: AoS vs SoA (Array of Structures vs Structure of Arrays)
// ============================================================================

// AoS (Array of Structures) - Traditional approach
struct ParticleAoS {
    float x, y, z;     // Position
    float vx, vy, vz;  // Velocity
    float mass;        // Mass
    int type;          // Type (rarely used)
};

// SoA (Structure of Arrays) - Cache-friendly approach
struct ParticlesSoA {
    std::vector<float> x, y, z;     // All particle x-coordinates together
    std::vector<float> vx, vy, vz;  // All particle velocities together
    std::vector<float> mass;

    void resize(size_t n) {
        x.resize(n);
        y.resize(n);
        z.resize(n);
        vx.resize(n);
        vy.resize(n);
        vz.resize(n);
        mass.resize(n);
    }
};

void example_aos_vs_soa() {
    LOG_INFO("=== Example 2: AoS vs SoA ===");

    const size_t N = 1000000;  // 1M particles

    // AoS: Update all particle positions
    {
        std::vector<ParticleAoS> particles(N);
        for (auto& p : particles) {
            p.x = 1.0f;
            p.vx = 0.1f;
        }

        SCOPED_TIMER("AoS update positions");
        for (size_t i = 0; i < N; ++i) {
            // Access particles[i].x and particles[i].vx
            // Each struct is 32 bytes, only 2 floats (8 bytes) used
            // Cache utilization: 8/64 = 12.5%
            particles[i].x += particles[i].vx;
        }
    }

    // SoA: Update all particle positions
    {
        ParticlesSoA particles;
        particles.resize(N);
        std::fill(particles.x.begin(), particles.x.end(), 1.0f);
        std::fill(particles.vx.begin(), particles.vx.end(), 0.1f);

        SCOPED_TIMER("SoA update positions");
        for (size_t i = 0; i < N; ++i) {
            // x[i] and x[i+1] are contiguous in memory
            // Cache utilization: ~100%
            particles.x[i] += particles.vx[i];
        }
    }

    LOG_INFO("  SoA is typically 2-4x faster when accessing only some fields!");
    LOG_INFO("  Tensors in AI frameworks are the ultimate expression of SoA");
}

// ============================================================================
// Example 3: Data Prefetch and Access Patterns
// ============================================================================
void example_access_pattern() {
    LOG_INFO("=== Example 3: Sequential vs Random Access ===");

    const size_t N = 10000000;  // 10M
    std::vector<int> data(N);
    std::iota(data.begin(), data.end(), 0);

    // Prepare random indices
    std::vector<size_t> random_indices(N);
    std::iota(random_indices.begin(), random_indices.end(), 0);
    std::mt19937 gen(42);
    std::shuffle(random_indices.begin(), random_indices.end(), gen);

    // Sequential access (cache-friendly)
    {
        SCOPED_TIMER("Sequential access");
        volatile long long sum = 0;
        for (size_t i = 0; i < N; ++i) {
            sum += data[i];  // Sequential access
        }
        (void)sum;
    }

    // Random access (cache-unfriendly)
    {
        SCOPED_TIMER("Random access");
        volatile long long sum = 0;
        for (size_t i = 0; i < N; ++i) {
            sum += data[random_indices[i]];  // Strided access
        }
        (void)sum;
    }

    LOG_INFO("  Sequential access is typically 5-20x faster!");
}

// ============================================================================
// Example 4: False Sharing Problem
// ============================================================================
struct alignas(64) PaddedCounter {
    int value = 0;
    // alignas(64) Ensure each counter occupies its own cache line
    // Prevent counters from different threads sharing a cache line
};

void example_false_sharing() {
    LOG_INFO("=== Example 4: False Sharing ===");
    LOG_INFO("  When multiple threads modify different variables in the same cache line,");
    LOG_INFO("  the cache line ping-pongs between CPU cores, degrading performance");
    LOG_INFO("  Solution: use alignas(64) to pad to independent cache lines");
    LOG_INFO_FMT("  PaddedCounter size: " << sizeof(PaddedCounter) << " bytes (= cache line size)");
}

// ============================================================================
// Example 5: Loop Tiling - Matrix Multiplication Optimization
// ============================================================================
void example_tiling() {
    LOG_INFO("=== Example 5: Loop Tiling (Matrix Multiplication) ===");

    const int N = 512;
    std::vector<float> A(static_cast<size_t>(N * N), 1.0f);
    std::vector<float> B(static_cast<size_t>(N * N), 1.0f);
    std::vector<float> C(static_cast<size_t>(N * N), 0.0f);

    // Naive implementation (cache-unfriendly)
    {
        std::fill(C.begin(), C.end(), 0.0f);
        SCOPED_TIMER("Naive matrix multiply");
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                float sum = 0;
                for (int k = 0; k < N; ++k) {
                    sum += A[static_cast<size_t>(i * N + k)] *
                           B[static_cast<size_t>(k * N + j)];  // B's column access is cache-unfriendly
                }
                C[static_cast<size_t>(i * N + j)] = sum;
            }
        }
    }

    // Tiled implementation (cache-friendly)
    {
        std::fill(C.begin(), C.end(), 0.0f);
        const int BLOCK = 32;  // Block size, typically chosen to fit in L1 cache

        SCOPED_TIMER("Tiled matrix multiply");
        for (int i0 = 0; i0 < N; i0 += BLOCK) {
            for (int j0 = 0; j0 < N; j0 += BLOCK) {
                for (int k0 = 0; k0 < N; k0 += BLOCK) {
                    // Process one tile (fits entirely in cache)
                    for (int i = i0; i < std::min(i0 + BLOCK, N); ++i) {
                        for (int k = k0; k < std::min(k0 + BLOCK, N); ++k) {
                            float a_val = A[static_cast<size_t>(i * N + k)];
                            for (int j = j0; j < std::min(j0 + BLOCK, N); ++j) {
                                C[static_cast<size_t>(i * N + j)] +=
                                    a_val * B[static_cast<size_t>(k * N + j)];
                            }
                        }
                    }
                }
            }
        }
    }

    LOG_INFO("  Tiling ensures each block fits in L1 cache, reducing cache misses");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    LOG_INFO("========================================");
    LOG_INFO("  Cache-Friendly Programming");
    LOG_INFO("========================================");

    example_row_vs_column();
    example_aos_vs_soa();
    example_access_pattern();
    example_false_sharing();
    example_tiling();

    LOG_INFO("========================================");
    LOG_INFO("  Core performance optimization principles:");
    LOG_INFO("  1. Sequential access > Random access");
    LOG_INFO("  2. SoA > AoS (when accessing only some fields)");
    LOG_INFO("  3. Avoid false sharing");
    LOG_INFO("  4. Tile large data processing");
    LOG_INFO("========================================");

    return 0;
}
