#pragma once
#include <Core/Base.hpp>
#include <concepts>
#include <cstring>
#include <memory_resource>

namespace quinte
{
    struct Memory final
    {
        inline static void Set(void* dst, uint8_t value, size_t byteCount)
        {
            memset(dst, value, byteCount);
        }

        template<class T>
        requires std::is_trivially_copyable_v<T> inline static void Copy(T* pDestination, const T* pSource, size_t elementCount)
        {
            memcpy(pDestination, pSource, elementCount * sizeof(T));
        }

        inline static std::pmr::memory_resource* GetDefaultAllocator()
        {
            return std::pmr::get_default_resource();
        }
    };
} // namespace quinte
