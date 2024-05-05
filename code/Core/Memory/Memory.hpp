#pragma once
#include <Core/CoreMath.hpp>
#include <concepts>
#include <cstring>
#include <memory>
#include <memory_resource>

namespace quinte::memory
{
    inline constexpr size_t kDefaultAlignment = 16;
    inline constexpr size_t kCacheLineSize = 64;


    namespace detail
    {
        template<class T>
        inline constexpr size_t kDefaultAllocationAlignment = Max(alignof(T), kDefaultAlignment);
    }


    namespace platform
    {
        //! \brief The size of virtual memory page.
        inline static constexpr size_t kVirtualPageSize = 4 * 1024;

        //! \brief The virtual allocation granularity.
        inline static constexpr size_t kVirtualAllocationGranularity = 64 * 1024;


        //! \brief Call platform-specific function to allocate virtual memory directly from the OS.
        void* Allocate(size_t byteSize);


        //! \brief Deallocate memory allocated via memory::platform::Allocate().
        void Deallocate(void* pointer, size_t byteSize);
    } // namespace platform


    //! \brief An allocate that allocates virtual memory directly from the OS.
    class VirtualMemoryResource final : public std::pmr::memory_resource
    {
        static VirtualMemoryResource s_Instance;

    public:
        inline void* do_allocate(size_t byteSize, size_t) override
        {
            return platform::Allocate(byteSize);
        }


        inline void do_deallocate(void* p, size_t byteSize, size_t) override
        {
            platform::Deallocate(p, byteSize);
        }


        inline bool do_is_equal(const memory_resource&) const noexcept override
        {
            return false;
        }

        inline static VirtualMemoryResource* Get()
        {
            return &s_Instance;
        }
    };

    inline VirtualMemoryResource VirtualMemoryResource::s_Instance;


    //! \brief Set byteCount bytes of memory at pDestination to zero.
    template<std::default_initializable T>
    inline static void Zero(T* pDestination, size_t elementCount)
    {
        if constexpr (std::is_floating_point_v<T> || std::is_integral_v<T>)
        {
            memset(pDestination, 0, elementCount * sizeof(T));
        }
        else
        {
            for (uint32_t i = 0; i < elementCount; ++i)
                new (&pDestination[i]) T;
        }
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


    namespace detail
    {
        void* DefaultAlloc(size_t byteSize);
        void* DefaultAlloc(size_t byteSize, size_t byteAlignment);
        void DefaultFree(void* ptr);
    } // namespace detail


    //! \brief Allocate memory aligned by kDefaultAlignment.
    //!
    //! This is a low-level function, it doesn't account for sizeof(T) or alignof(T).
    //! It doesn't call constructors and doesn't initialize the memory.
    //! Prefer using DefaultNew<T> if you need to allocate an object.
    //!
    //! \param byteSize Size of the memory to allocate in bytes.
    template<class T>
    inline T* DefaultAlloc(size_t byteSize)
    {
        return static_cast<T*>(detail::DefaultAlloc(byteSize));
    }


    //! \brief Allocate memory aligned by byteAlignment.
    //!
    //! This is a low-level function, it doesn't account for sizeof(T) or alignof(T).
    //! It doesn't call constructors and doesn't initialize the memory.
    //! Prefer using DefaultNew<T> if you need to allocate an object.
    //!
    //! \param byteSize Size of the memory to allocate in bytes.
    //! \param byteAlignment Alignment of the memory to allocate in bytes.
    template<class T>
    inline T* DefaultAlloc(size_t byteSize, size_t byteAlignment)
    {
        return static_cast<T*>(detail::DefaultAlloc(byteSize, byteAlignment));
    }


    //! \brief Free memory allocated by DefaultAlloc<T>.
    //!
    //! This is a low-level function, it doesn't call destructors.
    //! To free memory allocated by DefaultNew<T> and destroy the object use DefaultDelete<T>.
    inline void DefaultFree(void* ptr)
    {
        detail::DefaultFree(ptr);
    }


    //! \brief Just a helper to allocate memory using std::pmr::memory_resource and cast to T*
    template<class T>
    inline T* Alloc(std::pmr::memory_resource* pAllocator, size_t byteSize, size_t byteAlignment = kDefaultAlignment)
    {
        return static_cast<T*>(pAllocator->allocate(byteSize, byteAlignment));
    }


    //! \brief Free memory allocated by DefaultAlloc<T> and set the pointer to null.
    //!
    //! This is a low-level function, it doesn't call destructors.
    //! To free memory allocated by DefaultNew<T> and destroy the object use DefaultDelete<T>.
    template<class T>
    inline void SafeFree(T*& ptr)
    {
        if (ptr)
        {
            detail::DefaultFree(ptr);
            ptr = nullptr;
        }
    }


    template<class T>
    inline void SafeFree(std::pmr::memory_resource* pAllocator, T*& ptr, size_t byteSize = 0,
                         size_t byteAlignment = kDefaultAlignment)
    {
        if (ptr)
        {
            pAllocator->deallocate(ptr, byteSize, byteAlignment);
            ptr = nullptr;
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
    requires std::constructible_from<T, TArgs...>
    inline T* New(TAllocator* pAllocator, TArgs&&... args)
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
    template<std::default_initializable T, class TAllocator>
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
    requires std::constructible_from<T, TArgs...>
    inline T* DefaultNew(TArgs&&... args)
    {
        return new (detail::DefaultAlloc(sizeof(T), detail::kDefaultAllocationAlignment<T>)) T(std::forward<TArgs>(args)...);
    }


    //! \brief Create a new array using the default allocator.
    //!
    //! \param count - The size of the array to allocate.
    //!
    //! \tparam T - The type of elements in the array to allocate.
    //!
    //! \return The allocated array.
    template<std::default_initializable T>
    inline T* DefaultNewArray(size_t count)
    {
        return new (detail::DefaultAlloc(sizeof(T) * count, detail::kDefaultAllocationAlignment<T>)) T[count];
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
    inline void DefaultDelete(T* pointer)
    {
        pointer->~T();
        detail::DefaultFree(pointer);
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

        detail::DefaultFree(pointer);
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
            ptr->~T();
            DefaultFree(ptr);
        }
    };


    template<class T>
    struct DefaultDeleterNoFree final
    {
        constexpr DefaultDeleterNoFree() noexcept = default;

        template<class T2>
        requires std::convertible_to<T2*, T*>
        inline DefaultDeleterNoFree(const DefaultDeleterNoFree<T2>&) noexcept
        {
        }

        inline void operator()(T* ptr) const noexcept
        {
            ptr->~T();
        }
    };


    template<class T>
    requires std::is_trivially_destructible_v<T>
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
            DefaultFree(ptr);
        }
    };


    template<class T>
    using unique_ptr = std::unique_ptr<T, DefaultDeleter<T>>;


    template<class T>
    requires(!std::is_array_v<T>)
    using unique_temp_ptr = std::unique_ptr<T, DefaultDeleterNoFree<T>>;


    template<class T, class... TArgs>
    requires(!std::is_array_v<T>)
    [[nodiscard]] unique_ptr<T> make_unique(TArgs&&... args)
    {
        return unique_ptr<T>(DefaultNew<T>(std::forward<TArgs>(args)...));
    }


    template<class T, class TSizeType = size_t>
    class StdDefaultAllocator final
    {
    public:
        using value_type = T;
        using size_type = TSizeType;

        constexpr StdDefaultAllocator() noexcept {}
        inline constexpr StdDefaultAllocator(const StdDefaultAllocator&) noexcept {}

        template<class U>
        inline constexpr StdDefaultAllocator(const StdDefaultAllocator<U, TSizeType>&) noexcept
        {
        }

        [[nodiscard]] inline T* allocate(size_t n) const
        {
            return static_cast<T*>(detail::DefaultAlloc(n));
        }

        inline void deallocate(T* ptr, size_t) const
        {
            detail::DefaultFree(ptr);
        }

        template<class U>
        inline bool operator==(const StdDefaultAllocator<U, TSizeType>&) noexcept
        {
            return true;
        }

        template<class U>
        inline bool operator!=(const StdDefaultAllocator<U, TSizeType>&) noexcept
        {
            return false;
        }
    };
} // namespace quinte::memory
