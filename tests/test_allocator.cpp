#include <gtest/gtest.h>
#include "alloc/PoolAllocator.h"
#include <string>
#include <cstdint>

using alloc::PoolAllocator;

// ============================================================
// Базовые проверки
// ============================================================

TEST(PoolAllocator, StartsEmpty) {
    PoolAllocator<int, 10> pool;
    EXPECT_EQ(pool.capacity(), 10u);
    EXPECT_EQ(pool.used(), 0u);
    EXPECT_EQ(pool.available(), 10u);
    EXPECT_TRUE(pool.empty());
    EXPECT_FALSE(pool.full());
}

TEST(PoolAllocator, AllocateSingle) {
    PoolAllocator<int, 3> pool;
    int* p = pool.allocate();
    ASSERT_NE(p, nullptr);
    *p = 42;
    EXPECT_EQ(*p, 42);
    EXPECT_EQ(pool.used(), 1u);
    EXPECT_EQ(pool.available(), 2u);
}

TEST(PoolAllocator, AllocateMultiple) {
    PoolAllocator<int, 5> pool;
    int* ptrs[5];
    for (int i = 0; i < 5; ++i) {
        ptrs[i] = pool.allocate();
        ASSERT_NE(ptrs[i], nullptr);
        *ptrs[i] = i * 10;
    }
    EXPECT_TRUE(pool.full());
    EXPECT_EQ(pool.used(), 5u);

    // Проверим, что значения не пересекаются
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(*ptrs[i], i * 10);
    }
}

TEST(PoolAllocator, OverflowReturnsNullptr) {
    PoolAllocator<int, 2> pool;
    EXPECT_NE(pool.allocate(), nullptr);
    EXPECT_NE(pool.allocate(), nullptr);
    EXPECT_EQ(pool.allocate(), nullptr);   // третий — отказ
    EXPECT_TRUE(pool.full());
}

TEST(PoolAllocator, DeallocateReturnsSlot) {
    PoolAllocator<int, 2> pool;
    int* a = pool.allocate();
    int* b = pool.allocate();
    EXPECT_EQ(pool.allocate(), nullptr);

    pool.deallocate(a);
    EXPECT_EQ(pool.used(), 1u);

    int* c = pool.allocate();
    EXPECT_NE(c, nullptr);
    EXPECT_TRUE(pool.full());

    pool.deallocate(b);
    pool.deallocate(c);
    EXPECT_TRUE(pool.empty());
}

TEST(PoolAllocator, DeallocateNullIsSafe) {
    PoolAllocator<int, 3> pool;
    pool.deallocate(nullptr);   // не должно падать
    EXPECT_TRUE(pool.empty());
}

TEST(PoolAllocator, ReusesFreedSlot) {
    PoolAllocator<int, 1> pool;
    int* a = pool.allocate();
    pool.deallocate(a);
    int* b = pool.allocate();
    // В пуле один слот, значит b должен совпасть с a
    EXPECT_EQ(a, b);
}

// ============================================================
// create / destroy — нетривиальные типы
// ============================================================

TEST(PoolAllocator, CreateWithArgs) {
    PoolAllocator<std::string, 2> pool;
    auto* s = pool.create(5, 'x');   // вызывает std::string(5, 'x') = "xxxxx"
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(*s, "xxxxx");
    EXPECT_EQ(pool.used(), 1u);
    pool.destroy(s);
}

// Вспомогательный счётчик для проверки вызова деструкторов.
// Определяем ВНЕ теста, потому что локальные классы не могут иметь статических полей.
namespace {
    struct Counter {
        static int alive;
        Counter() { ++alive; }
        ~Counter() { --alive; }
    };
    int Counter::alive = 0;
}

TEST(PoolAllocator, DestroyCallsDestructor) {
    Counter::alive = 0;   // сброс на всякий случай

    {
        PoolAllocator<Counter, 3> pool;
        auto* a = pool.create();
        auto* b = pool.create();
        EXPECT_EQ(Counter::alive, 2);

        pool.destroy(a);
        EXPECT_EQ(Counter::alive, 1);
        pool.destroy(b);
        EXPECT_EQ(Counter::alive, 0);
    }
    EXPECT_EQ(Counter::alive, 0);
}


TEST(PoolAllocator, CreateOverflowReturnsNullptr) {
    PoolAllocator<std::string, 1> pool;
    auto* a = pool.create("first");
    auto* b = pool.create("second");
    EXPECT_NE(a, nullptr);
    EXPECT_EQ(b, nullptr);
    pool.destroy(a);
}

// ============================================================
// owns()
// ============================================================

TEST(PoolAllocator, OwnsPointerInsidePool) {
    PoolAllocator<int, 4> pool;
    int* p = pool.allocate();
    EXPECT_TRUE(pool.owns(p));
    pool.deallocate(p);
}

TEST(PoolAllocator, OwnsReturnsFalseForExternalPointer) {
    PoolAllocator<int, 4> pool;
    int stackVar = 0;
    EXPECT_FALSE(pool.owns(&stackVar));
    EXPECT_FALSE(pool.owns(nullptr));
}

TEST(PoolAllocator, OwnsReturnsFalseForOtherPool) {
    PoolAllocator<int, 4> pool1;
    PoolAllocator<int, 4> pool2;
    int* p = pool1.allocate();
    EXPECT_TRUE(pool1.owns(p));
    EXPECT_FALSE(pool2.owns(p));
    pool1.deallocate(p);
}

// ============================================================
// reset()
// ============================================================

TEST(PoolAllocator, ResetFreesAllSlots) {
    PoolAllocator<int, 4> pool;
    pool.allocate();
    pool.allocate();
    pool.allocate();
    EXPECT_EQ(pool.used(), 3u);

    pool.reset();
    EXPECT_EQ(pool.used(), 0u);
    EXPECT_TRUE(pool.empty());

    // После reset все слоты должны быть доступны заново
    for (int i = 0; i < 4; ++i) {
        EXPECT_NE(pool.allocate(), nullptr);
    }
    EXPECT_TRUE(pool.full());
}

// ============================================================
// Выравнивание — критично для аллокатора
// ============================================================

TEST(PoolAllocator, AlignedForDouble) {
    PoolAllocator<double, 4> pool;
    for (int i = 0; i < 4; ++i) {
        double* p = pool.allocate();
        ASSERT_NE(p, nullptr);
        auto addr = reinterpret_cast<std::uintptr_t>(p);
        EXPECT_EQ(addr % alignof(double), 0u) << "Double is misaligned!";
    }
}

TEST(PoolAllocator, AlignedForLargeStruct) {
    struct alignas(32) Big {
        char data[64];
    };
    PoolAllocator<Big, 4> pool;
    for (int i = 0; i < 4; ++i) {
        Big* p = pool.allocate();
        ASSERT_NE(p, nullptr);
        auto addr = reinterpret_cast<std::uintptr_t>(p);
        EXPECT_EQ(addr % 32u, 0u) << "alignas(32) struct is misaligned!";
    }
}

TEST(PoolAllocator, SmallTypeStillWorks) {
    // char меньше, чем FreeNode* — слот должен вмещать и то, и другое
    PoolAllocator<char, 4> pool;
    char* a = pool.allocate();
    char* b = pool.allocate();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    *a = 'x';
    *b = 'y';
    EXPECT_EQ(*a, 'x');
    EXPECT_EQ(*b, 'y');
}