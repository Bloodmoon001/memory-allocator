#include <benchmark/benchmark.h>
#include "alloc/PoolAllocator.h"
#include <cstdlib>
#include <string>

// ============================================================
// Сравнение: PoolAllocator vs malloc/free для int
// ============================================================

static void BM_Pool_Int(benchmark::State& state) {
    alloc::PoolAllocator<int, 1024> pool;
    for (auto _ : state) {
        for (int i = 0; i < 1024; ++i) {
            int* p = pool.allocate();
            benchmark::DoNotOptimize(*p = i);
            pool.deallocate(p);
        }
    }
}
BENCHMARK(BM_Pool_Int);

static void BM_Malloc_Int(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < 1024; ++i) {
            int* p = static_cast<int*>(std::malloc(sizeof(int)));
            benchmark::DoNotOptimize(*p = i);
            std::free(p);
        }
    }
}
BENCHMARK(BM_Malloc_Int);

static void BM_New_Int(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < 1024; ++i) {
            int* p = new int(i);
            benchmark::DoNotOptimize(*p);
            delete p;
        }
    }
}
BENCHMARK(BM_New_Int);

// ============================================================
// Сравнение с нетривиальным типом (std::string)
// ============================================================

static void BM_Pool_String(benchmark::State& state) {
    alloc::PoolAllocator<std::string, 256> pool;
    for (auto _ : state) {
        for (int i = 0; i < 256; ++i) {
            auto* p = pool.create("hello world, benchmark string");
            benchmark::DoNotOptimize(p->size());
            pool.destroy(p);
        }
    }
}
BENCHMARK(BM_Pool_String);

static void BM_New_String(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < 256; ++i) {
            auto* p = new std::string("hello world, benchmark string");
            benchmark::DoNotOptimize(p->size());
            delete p;
        }
    }
}
BENCHMARK(BM_New_String);

// ============================================================
// Массовая аллокация: сколько времени на N штук
// ============================================================

static void BM_Pool_Bulk(benchmark::State& state) {
    const std::size_t count = state.range(0);
    alloc::PoolAllocator<int, 100000> pool;
    for (auto _ : state) {
        for (std::size_t i = 0; i < count; ++i) {
            pool.allocate();
        }
        pool.reset();
    }
    state.SetItemsProcessed(state.iterations() * count);
}
BENCHMARK(BM_Pool_Bulk)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Malloc_Bulk(benchmark::State& state) {
    const std::size_t count = state.range(0);
    std::vector<int*> ptrs(count);
    for (auto _ : state) {
        for (std::size_t i = 0; i < count; ++i) {
            ptrs[i] = static_cast<int*>(std::malloc(sizeof(int)));
        }
        for (std::size_t i = 0; i < count; ++i) {
            std::free(ptrs[i]);
        }
    }
    state.SetItemsProcessed(state.iterations() * count);
}
BENCHMARK(BM_Malloc_Bulk)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();