#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>
#include <cassert>

namespace alloc {

    template <typename T, std::size_t N>
    class PoolAllocator {
    public:
        PoolAllocator() noexcept;
        ~PoolAllocator() noexcept = default;

        // Запрещаем копирование и перемещение
        PoolAllocator(const PoolAllocator&) = delete;
        PoolAllocator& operator=(const PoolAllocator&) = delete;
        PoolAllocator(PoolAllocator&&) = delete;
        PoolAllocator& operator=(PoolAllocator&&) = delete;

        T* allocate() noexcept;
        void deallocate(T* ptr) noexcept;

        template <typename... Args>
        T* create(Args&&... args);

        void destroy(T* ptr) noexcept;

        std::size_t capacity()  const noexcept { return N; }
        std::size_t used()      const noexcept { return used_; }
        std::size_t available() const noexcept { return N - used_; }
        bool        full()      const noexcept { return used_ == N; }
        bool        empty()     const noexcept { return used_ == 0; }

        bool owns(const T* ptr) const noexcept;
        void reset() noexcept;

    private:
        // Узел free-list. Хранится в свободных слотах.
        struct FreeNode { FreeNode* next; };

        // Слот должен вмещать либо T, либо FreeNode — что больше.
        static constexpr std::size_t SLOT_SIZE =
            sizeof(T) > sizeof(FreeNode) ? sizeof(T) : sizeof(FreeNode);

        // Слот должен быть выровнен под T и под FreeNode.
        static constexpr std::size_t SLOT_ALIGN =
            alignof(T) > alignof(FreeNode) ? alignof(T) : alignof(FreeNode);

        struct alignas(SLOT_ALIGN) Slot {
            std::byte storage[SLOT_SIZE];
        };

        Slot slots_[N];
        FreeNode* freeList_ = nullptr;
        std::size_t used_ = 0;
    };

    // ============================================================
    // Реализация
    // ============================================================

    template <typename T, std::size_t N>
    PoolAllocator<T, N>::PoolAllocator() noexcept {
        for (std::size_t i = 0; i + 1 < N; ++i) {
            auto* node = reinterpret_cast<FreeNode*>(slots_[i].storage);
            node->next = reinterpret_cast<FreeNode*>(slots_[i + 1].storage);
        }
        auto* last = reinterpret_cast<FreeNode*>(slots_[N - 1].storage);
        last->next = nullptr;

        freeList_ = reinterpret_cast<FreeNode*>(slots_[0].storage);
        used_ = 0;
    }

    template <typename T, std::size_t N>
    T* PoolAllocator<T, N>::allocate() noexcept {
        if (!freeList_) return nullptr;

        FreeNode* node = freeList_;
        freeList_ = node->next;
        ++used_;
        return reinterpret_cast<T*>(node);
    }

    template <typename T, std::size_t N>
    void PoolAllocator<T, N>::deallocate(T* ptr) noexcept {
        if (!ptr) return;
        assert(owns(ptr) && "Pointer does not belong to this pool");

        auto* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = freeList_;
        freeList_ = node;
        --used_;
    }

    template <typename T, std::size_t N>
    template <typename... Args>
    T* PoolAllocator<T, N>::create(Args&&... args) {
        T* ptr = allocate();
        if (!ptr) return nullptr;
        return new (ptr) T(std::forward<Args>(args)...);
    }

    template <typename T, std::size_t N>
    void PoolAllocator<T, N>::destroy(T* ptr) noexcept {
        if (!ptr) return;
        ptr->~T();
        deallocate(ptr);
    }

    template <typename T, std::size_t N>
    bool PoolAllocator<T, N>::owns(const T* ptr) const noexcept {
        if (!ptr) return false;
        const auto* p = reinterpret_cast<const std::byte*>(ptr);
        const auto* begin = reinterpret_cast<const std::byte*>(slots_);
        const auto* end = reinterpret_cast<const std::byte*>(slots_ + N);
        return p >= begin && p < end;
    }

    template <typename T, std::size_t N>
    void PoolAllocator<T, N>::reset() noexcept {
        for (std::size_t i = 0; i + 1 < N; ++i) {
            auto* node = reinterpret_cast<FreeNode*>(slots_[i].storage);
            node->next = reinterpret_cast<FreeNode*>(slots_[i + 1].storage);
        }
        auto* last = reinterpret_cast<FreeNode*>(slots_[N - 1].storage);
        last->next = nullptr;

        freeList_ = reinterpret_cast<FreeNode*>(slots_[0].storage);
        used_ = 0;
    }

} // namespace alloc