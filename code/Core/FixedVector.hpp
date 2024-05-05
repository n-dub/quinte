#pragma once
#include <Core/Core.hpp>
#include <gch/small_vector.hpp>

namespace quinte
{
#if __INTELLISENSE__
    // for some reason intellisense can't handle this

    template<class T, size_t TCapacity = gch::default_buffer_size_v<std::allocator<T>>>
    using SmallVector = gch::small_vector<T, TCapacity>;
#else
    template<class T, size_t TCapacity = gch::default_buffer_size_v<memory::StdDefaultAllocator<T, uint32_t>>>
    using SmallVector = gch::small_vector<T, TCapacity, memory::StdDefaultAllocator<T, uint32_t>>;
#endif


    template<class T, size_t TCapacity>
    class FixedVector final
    {
        T m_Data[TCapacity];
        uint32_t m_Size;

    public:
        inline FixedVector()
            : m_Size(0)
        {
        }

        inline FixedVector(size_t n, const T& x)
            : m_Size(n)
        {
            for (uint32_t i = 0; i < n; ++i)
                new (&m_Data[i]) T(x);
        }

        inline FixedVector(std::initializer_list<T> list)
        {
            QU_Assert(list.size() <= capacity());
            for (const T& value : list)
                push_back(value);
        }

        inline FixedVector(const FixedVector& other)
            : m_Size(other.m_Size)
        {
            memory::Copy(m_Data, other.m_Data, m_Size);
        }

        inline FixedVector& operator=(const FixedVector& other)
        {
            clear();
            m_Size = other.m_Size;
            memory::Copy(m_Data, other.m_Data, m_Size);
            return *this;
        }

        inline FixedVector(FixedVector&& other) noexcept
            : m_Size(other.m_Size)
        {
            other.m_Size = 0;
            for (uint32_t i = 0; i < m_Size; ++i)
                new (&m_Data[i]) T(std::move(other.m_Data[i]));
        }

        inline FixedVector& operator=(FixedVector&& other) noexcept
        {
            clear();
            m_Size = other.m_Size;
            other.m_Size = 0;
            for (uint32_t i = 0; i < m_Size; ++i)
                new (&m_Data[i]) T(std::move(other.m_Data[i]));
            return *this;
        }

        inline T& operator[](size_t index) noexcept
        {
            QU_Assert(index < size());
            return m_Data[index];
        }

        inline const T& operator[](size_t index) const noexcept
        {
            QU_Assert(index < size());
            return m_Data[index];
        }

        inline T& push_back(const T& value)
        {
            QU_Assert(m_Size < capacity());
            new (&m_Data[m_Size++]) T(value);
            return m_Data[m_Size - 1];
        }

        inline T& push_back(T&& value)
        {
            QU_Assert(m_Size < capacity());
            new (&m_Data[m_Size++]) T(std::move(value));
            return m_Data[m_Size - 1];
        }

        inline void pop_back()
        {
            QU_Assert(m_Size > 0);
            m_Data[--m_Size].~T();
        }

        inline void clear()
        {
            while (m_Size)
                pop_back();
        }

        [[nodiscard]] inline size_t size() const noexcept
        {
            return m_Size;
        }

        [[nodiscard]] inline size_t capacity() const noexcept
        {
            return TCapacity;
        }

        [[nodiscard]] inline T* begin() noexcept
        {
            return m_Data;
        }

        [[nodiscard]] inline T* end() noexcept
        {
            return m_Data + m_Size;
        }

        [[nodiscard]] inline const T* begin() const noexcept
        {
            return m_Data;
        }

        [[nodiscard]] inline const T* end() const noexcept
        {
            return m_Data + m_Size;
        }
    };
} // namespace quinte
