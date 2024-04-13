#pragma once
#include <Core/Base.hpp>
#include <concepts>
#include <cstring>
#include <memory>
#include <memory_resource>

namespace quinte::memory
{
    inline constexpr size_t kDefaultAlignment = 16;


    inline constexpr size_t operator""_KB(size_t bytes)
    {
        return bytes * 1024;
    }


    inline constexpr size_t operator""_MB(size_t bytes)
    {
        return bytes * 1024 * 1024;
    }


    inline constexpr size_t operator""_GB(size_t bytes)
    {
        return bytes * 1024 * 1024 * 1024;
    }


    namespace detail
    {
        template<class T>
        inline constexpr size_t kDefaultAllocationAlignment = Max(alignof(T), kDefaultAlignment);
    }


    namespace platform
    {
        //! \brief The size of virtual memory page.
        inline static constexpr size_t kVirtualPageSize = 4_KB;

        //! \brief The virtual allocation granularity.
        inline static constexpr size_t kVirtualAllocationGranularity = 64_KB;


        //! \brief Call platform-specific function to allocate virtual memory directly from the OS.
        void* Allocate(size_t byteSize);


        //! \brief Deallocate memory allocated via memory::platform::Allocate().
        void Deallocate(void* pointer, size_t byteSize);
    } // namespace platform


    //! \brief Set byteCount bytes of memory at pDestination to value.
    inline static void Set(void* pDestination, uint8_t value, size_t byteCount)
    {
        memset(pDestination, value, byteCount);
    }


    //! \brief Copy memory from pSource to pDestination.
    template<class T>
    inline static void Copy(T* pDestination, const T* pSource, size_t elementCount)
    {
        if (std::is_trivially_copyable_v<T>)
        {
            memcpy(pDestination, pSource, elementCount * sizeof(T));
        }
        else
        {
            for (uint32_t i = 0; i < elementCount; ++i)
                new (&pDestination[i]) T(pSource[i]);
        }
    }


    //! \brief Create a new object of type T using the provided allocator.
    //!
    //! \param pAllocator - The allocator to use.
    //! \param args       - The arguments to call the constructor of T with.
    //!
    //! \tparam T          - The type of the object to allocate.
    //! \tparam TAllocator - The type of the provided allocator.
    //!
    //! \return The allocated object.
    template<class T, class TAllocator, class... TArgs>
    inline T* New(TAllocator* pAllocator, TArgs&&... args)
    requires std::constructible_from<T, TArgs...>
    {
        return new (pAllocator->allocate(sizeof(T), detail::kDefaultAllocationAlignment<T>)) T(std::forward<TArgs>(args)...);
    }


    //! \brief Create a new array using the provided allocator.
    //!
    //! \param pAllocator - The allocator to use.
    //! \param count      - The size of the array to allocate.
    //!
    //! \tparam T - The type of elements in the array to allocate.
    //! \tparam TAllocator - The type of the provided allocator.
    //!
    //! \return The allocated array.
    template<class T, class TAllocator>
    inline T* NewArray(TAllocator* pAllocator, size_t count)
    {
        return new (pAllocator->allocate(sizeof(T) * count, detail::kDefaultAllocationAlignment<T>)) T[count];
    }


    //! \brief Create a new object of type T using the default allocator.
    //!
    //! \param args - The arguments to call the constructor of T with.
    //!
    //! \tparam T - The type of the object to allocate.
    //!
    //! \return The allocated object.
    template<class T, class... TArgs>
    inline T* DefaultNew(TArgs&&... args)
    requires std::constructible_from<T, TArgs...>
    {
        return new (std::pmr::get_default_resource()->allocate(sizeof(T), detail::kDefaultAllocationAlignment<T>))
            T(std::forward<TArgs>(args)...);
    }


    //! \brief Create a new array using the default allocator.
    //!
    //! \param count - The size of the array to allocate.
    //!
    //! \tparam T - The type of elements in the array to allocate.
    //!
    //! \return The allocated array.
    template<class T>
    inline T* DefaultNewArray(size_t count)
    {
        return new (std::pmr::get_default_resource()->allocate(sizeof(T) * count, detail::kDefaultAllocationAlignment<T>))
            T[count];
    }


    //! \brief Delete an object previously created via memory::New().
    //!
    //! \param pAllocator - The allocator to use.
    //! \param pointer    - The pointer to the object to delete previously returned by memory::New().
    //! \param byteSize   - The size of the object to delete.
    //!
    //! \tparam T          - The type of the object to delete.
    //! \tparam TAllocator - The type of the provided allocator.
    template<class T, class TAllocator>
    inline void Delete(TAllocator* pAllocator, T* pointer, size_t byteSize)
    {
        pointer->~T();
        pAllocator->deallocate(pointer, byteSize, detail::kDefaultAllocationAlignment<T>);
    }


    //! \brief Delete an array previously allocated via memory::NewArray().
    //!
    //! \param pAllocator - The allocator to use.
    //! \param pointer    - The pointer to the array to delete previously returned by memory::NewArray().
    //! \param count      - The size of the array to delete.
    //!
    //! \tparam T          - The type of elements in the array to delete.
    //! \tparam TAllocator - The type of the provided allocator.
    template<class T, class TAllocator>
    inline void DeleteArray(TAllocator* pAllocator, T* pointer, size_t count)
    {
        for (uint32_t i = 0; i < count; ++i)
            pointer[i].~T();

        pAllocator->deallocate(pointer, sizeof(T) * count, detail::kDefaultAllocationAlignment<T>);
    }


    //! \brief Delete an object previously created via memory::DefaultNew().
    //!
    //! \param pointer  - The pointer to the object to delete previously returned by memory::DefaultNew().
    //! \param byteSize - The size of the object to delete.
    //!
    //! \tparam T - The type of the object to delete.
    template<class T>
    inline void DefaultDelete(T* pointer, size_t byteSize)
    {
        pointer->~T();
        std::pmr::get_default_resource()->deallocate(pointer, byteSize, detail::kDefaultAllocationAlignment<T>);
    }


    //! \brief Delete an array previously allocated via memory::DefaultNewArray().
    //!
    //! \param pointer - The pointer to the array to delete previously returned by memory::NewArray().
    //! \param count   - The size of the array to delete.
    //!
    //! \tparam T - The type of elements in the array to delete.
    template<class T>
    inline void DefaultDeleteArray(T* pointer, size_t count)
    {
        for (uint32_t i = 0; i < count; ++i)
            pointer[i].~T();

        std::pmr::get_default_resource()->deallocate(pointer, sizeof(T) * count, detail::kDefaultAllocationAlignment<T>);
    }


    template<class T>
    concept IsReleasable = requires(T t) { t.Release(); };


    //! \brief Check the provided pointer for nullptr, call T::Release() and reset it to nullptr.
    template<class T>
    requires IsReleasable<T>
    inline void SafeRelease(T*& ptr)
    {
        if (ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }


    template<class T>
    struct DefaultDeleter final
    {
        constexpr DefaultDeleter() noexcept = default;

        template<class T2>
        requires std::convertible_to<T2*, T*>
        inline DefaultDeleter(const DefaultDeleter<T2>&) noexcept
        {
        }

        inline void operator()(T* ptr) const noexcept
        {
            DefaultDelete(ptr, 0);
        }
    };


    template<class T>
    struct DefaultDeleterNoDealloc final
    {
        constexpr DefaultDeleterNoDealloc() noexcept = default;

        template<class T2>
        requires std::convertible_to<T2*, T*>
        inline DefaultDeleterNoDealloc(const DefaultDeleterNoDealloc<T2>&) noexcept
        {
        }

        inline void operator()(T* ptr) const noexcept
        {
            ptr->~T();
        }
    };


    template<class T>
    requires std::is_trivial_v<T>
    struct DefaultDeleter<T[]>
    {
        constexpr DefaultDeleter() noexcept = default;

        template<class T2>
        requires std::convertible_to<T2 (*)[], T (*)[]>
        inline DefaultDeleter(const DefaultDeleter<T2[]>&) noexcept
        {
        }

        template<class T2>
        requires std::convertible_to<T2 (*)[], T (*)[]>
        inline void operator()(T2* ptr) const noexcept
        {
            DefaultDeleteArray(ptr, 0);
        }
    };


    template<class T>
    using unique_ptr = std::unique_ptr<T, DefaultDeleter<T>>;


    template<class T>
    requires(!std::is_array_v<T>)
    using unique_temp_ptr = std::unique_ptr<T, DefaultDeleterNoDealloc<T>>;


    template<class T, class... TArgs>
    requires(!std::is_array_v<T>)
    [[nodiscard]] unique_ptr<T> make_unique(TArgs&&... args)
    {
        return unique_ptr<T>(DefaultNew<T>(std::forward<TArgs>(args)...));
    }

} // namespace quinte::memory
