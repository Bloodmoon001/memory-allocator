#include "alloc/PoolAllocator.h"
#include <iostream>
#include <string>

int main() {
    // Простой пример: пул из 5 int-ов
    alloc::PoolAllocator<int, 5> pool;

    std::cout << "=== Basic int pool ===\n";
    std::cout << "Capacity: " << pool.capacity()
        << ", used: " << pool.used() << "\n";

    int* a = pool.allocate();
    int* b = pool.allocate();
    *a = 10;
    *b = 20;
    std::cout << "a=" << *a << ", b=" << *b
        << ", used=" << pool.used() << "\n";

    pool.deallocate(a);
    std::cout << "After dealloc(a): used=" << pool.used() << "\n";

    int* c = pool.allocate();  // должен переиспользовать слот a
    *c = 30;
    std::cout << "c=" << *c << ", used=" << pool.used() << "\n";

    // Пул с нетривиальным типом
    std::cout << "\n=== std::string pool ===\n";
    alloc::PoolAllocator<std::string, 3> strPool;

    auto* s1 = strPool.create("hello");
    auto* s2 = strPool.create("world");
    auto* s3 = strPool.create(5, 'x');   // "xxxxx"
    std::cout << *s1 << " " << *s2 << " " << *s3 << "\n";
    std::cout << "used: " << strPool.used() << "/" << strPool.capacity() << "\n";

    // Проверка на переполнение
    auto* s4 = strPool.create("overflow");
    std::cout << "s4 (should be nullptr): " << (s4 == nullptr ? "nullptr" : "valid") << "\n";

    // Уничтожение объектов — обязательно, иначе утечка!
    strPool.destroy(s1);
    strPool.destroy(s2);
    strPool.destroy(s3);
    std::cout << "After destroy: used=" << strPool.used() << "\n";

    // Проверка owns()
    std::cout << "\nowns(s1)? " << (strPool.owns(s1) ? "yes" : "no") << "\n";
    int stackVar = 42;
    std::cout << "owns(&stackVar)? "
        << (strPool.owns(reinterpret_cast<std::string*>(&stackVar)) ? "yes" : "no")
        << "\n";

    return 0;
}