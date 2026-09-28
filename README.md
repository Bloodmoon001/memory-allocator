# Pool Memory Allocator

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg)](https://cmake.org/)
[![Tests](https://img.shields.io/badge/tests-18%20passed-brightgreen.svg)](#tests)
[![Benchmark](https://img.shields.io/badge/speedup-10x%20vs%20malloc-orange.svg)](#benchmarks)

A high-performance pool memory allocator in C++17, up to **10x faster** than `malloc`/`free` on micro-allocations.

Высокопроизводительный пул-аллокатор на C++17, до **10 раз быстрее** стандартного `malloc`/`free` на микро-аллокациях.

---

## 🇬🇧 English

### About
This project implements a **pool allocator** — a memory allocator that pre-allocates a fixed-size block and hands out equal-sized slots in O(1). It's a header-only library that can be dropped into any C++17 project.

The allocator demonstrates:
* Manual memory management with placement new and explicit destructor calls
* Alignment handling via `alignas` and compile-time computed slot sizes
* An intrusive free-list that stores links **inside free slots** (zero metadata overhead)
* Performance comparison with `malloc` via Google Benchmark

### Features
* **O(1) allocation and deallocation** — no searching, no fragmentation
* **Zero metadata overhead** — free-list links live inside free slots
* **Correct alignment** — works with `double`, `alignas(N)` types, small types like `char`
* **RAII-friendly API** — `create<T>(args...)` / `destroy()` handles constructor/destructor
* **Safe ownership check** — `owns(ptr)` verifies a pointer belongs to the pool
* **Header-only** — just `#include "alloc/PoolAllocator.h"`
* **18 unit tests** (Google Test) + **11 benchmarks** (Google Benchmark)

### Project Structure
```
memory-allocator/
├── include/alloc/PoolAllocator.h    # Public API
├── apps/main.cpp                    # Demo executable
├── tests/test_allocator.cpp         # Google Test
├── benchmarks/bench_allocator.cpp   # Google Benchmark
└── CMakeLists.txt
```

### Benchmark Results

Run on Intel i5-12450H (12 cores @ 2.6 GHz), Windows 11, MSVC 2022, `x64-Release`:

| Benchmark | Pool Allocator | `malloc`/`new` | Speedup |
|---|---:|---:|---:|
| Single `int` allocate+free | **2 731 ns** | 29 067 ns | **10.6×** |
| `std::string` create+destroy | **7 240 ns** | 19 826 ns | **2.7×** |
| Bulk 10 000 allocations | **54 860 ns** | 433 902 ns | **7.9×** |
| Throughput | **188 M ops/s** | 23 M ops/s | **8.1×** |

See `benchmarks/` for the full test suite.

### Getting Started

#### Prerequisites
* Visual Studio 2022 (with "Desktop development with C++")
* CMake 3.20+
* vcpkg

#### Build & Run
1. Install dependencies:
   ```bash
   vcpkg install gtest:x64-windows
   vcpkg install benchmark:x64-windows
   ```
2. Clone:
   ```bash
   git clone https://github.com/Bloodmoon001/memory-allocator.git
   cd memory-allocator
   ```
3. Open the folder in Visual Studio (`File` → `Open` → `Folder`).
4. Select `x64-Release` configuration, press `F5` to run the demo.

### Usage Example
```cpp
#include "alloc/PoolAllocator.h"

int main() {
    // Pool of 1024 strings
    alloc::PoolAllocator<std::string, 1024> pool;

    auto* s1 = pool.create("hello");
    auto* s2 = pool.create(5, 'x');   // std::string(5, 'x')

    std::cout << *s1 << " " << *s2 << "\n";   // hello xxxxx
    std::cout << pool.used() << "/" << pool.capacity() << "\n";   // 2/1024

    pool.destroy(s1);
    pool.destroy(s2);
    return 0;
}
```

### Tests
```bash
./bin/alloc_tests.exe
```
Expected: `[  PASSED  ] 18 tests.`

### License
MIT License. See `LICENSE`.

---

## 🇷🇺 Русский

### О проекте
Этот проект реализует **пул-аллокатор** — аллокатор памяти, который заранее выделяет блок фиксированного размера и раздаёт из него слоты за O(1). Это header-only библиотека, которая подключается в любой C++17 проект.

Проект демонстрирует:
* Ручное управление памятью через placement new и явные вызовы деструкторов
* Работу с выравниванием через `alignas` и вычисляемые на этапе компиляции размеры слотов
* Интрузивный free-list, хранящий ссылки **внутри свободных слотов** (нулевые накладные расходы)
* Сравнение производительности с `malloc` через Google Benchmark

### Возможности
* **O(1) выделение и освобождение** — без поиска, без фрагментации
* **Нулевые накладные расходы** — ссылки free-list живут внутри свободных слотов
* **Корректное выравнивание** — работает с `double`, `alignas(N)`-типами, мелкими типами (`char`)
* **RAII-дружественный API** — `create<T>(args...)` / `destroy()` вызывают конструктор/деструктор
* **Безопасная проверка владения** — `owns(ptr)` проверяет, что указатель принадлежит пулу
* **Header-only** — достаточно `#include "alloc/PoolAllocator.h"`
* **18 unit-тестов** (Google Test) + **11 бенчмарков** (Google Benchmark)

### Структура проекта
```
memory-allocator/
├── include/alloc/PoolAllocator.h    # Публичный API
├── apps/main.cpp                    # Демо
├── tests/test_allocator.cpp         # Google Test
├── benchmarks/bench_allocator.cpp   # Google Benchmark
└── CMakeLists.txt
```

### Результаты бенчмарков

Прогон на Intel i5-12450H (12 ядер @ 2.6 ГГц), Windows 11, MSVC 2022, `x64-Release`:

| Бенчмарк | Пул-аллокатор | `malloc`/`new` | Ускорение |
|---|---:|---:|---:|
| Один `int` alloc+free | **2 731 нс** | 29 067 нс | **в 10.6 раз** |
| `std::string` create+destroy | **7 240 нс** | 19 826 нс | **в 2.7 раза** |
| Массовые 10 000 аллокаций | **54 860 нс** | 433 902 нс | **в 7.9 раз** |
| Пропускная способность | **188 млн/с** | 23 млн/с | **в 8.1 раз** |

Полный набор бенчмарков — в папке `benchmarks/`.

### Сборка и запуск

#### Требования
* Visual Studio 2022 (с рабочей нагрузкой «Разработка классических приложений на C++»)
* CMake 3.20+
* vcpkg

#### Сборка
1. Установите зависимости:
   ```bash
   vcpkg install gtest:x64-windows
   vcpkg install benchmark:x64-windows
   ```
2. Склонируйте репозиторий:
   ```bash
   git clone https://github.com/Bloodmoon001/memory-allocator.git
   cd memory-allocator
   ```
3. Откройте папку в Visual Studio (`Файл` → `Открыть` → `Папка`).
4. Выберите конфигурацию `x64-Release` и нажмите `F5`.

### Пример использования
```cpp
#include "alloc/PoolAllocator.h"

int main() {
    alloc::PoolAllocator<std::string, 1024> pool;

    auto* s1 = pool.create("hello");
    auto* s2 = pool.create(5, 'x');

    std::cout << *s1 << " " << *s2 << "\n";   // hello xxxxx
    std::cout << pool.used() << "/" << pool.capacity() << "\n";   // 2/1024

    pool.destroy(s1);
    pool.destroy(s2);
    return 0;
}
```

### Тесты
```bash
./bin/alloc_tests.exe
```
Ожидаемый вывод: `[  PASSED  ] 18 tests.`

### Лицензия
MIT License. См. файл `LICENSE`.

---