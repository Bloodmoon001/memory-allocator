cmake_minimum_required(VERSION 3.20)
project(MemoryAllocator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# vcpkg toolchain
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# --- Библиотека ---
add_library(alloc INTERFACE)
target_include_directories(alloc INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

# --- Демо ---
add_executable(alloc_demo apps/main.cpp)
target_link_libraries(alloc_demo PRIVATE alloc)

# --- Тесты ---
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)

    add_executable(alloc_tests tests/test_allocator.cpp)
    target_link_libraries(alloc_tests PRIVATE alloc GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(alloc_tests)
endif()

# --- Бенчмарки ---
option(BUILD_BENCHMARKS "Build benchmarks" ON)
if(BUILD_BENCHMARKS)
    find_package(benchmark CONFIG REQUIRED)

    add_executable(alloc_bench benchmarks/bench_allocator.cpp)
    target_link_libraries(alloc_bench PRIVATE alloc benchmark::benchmark)
endif()

# Все exe в bin/
set_target_properties(alloc_demo alloc_tests alloc_bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)