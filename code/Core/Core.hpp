#pragma once
#include <Core/Base.hpp>
#include <Core/Memory.hpp>
#include <cassert>
#include <concepts>
#include <span>
#include <string_view>
#include <type_traits>


#define QUINTE_Unused(expr) (void)expr
#define QUINTE_Assert(expr) assert(expr)
#define QUINTE_AssertMsg(expr, msg) assert(expr)


namespace quinte
{
    inline constexpr size_t kDefaultAlignment = 16;

    //! \brief Align up an integer.
    //!
    //! \param x     - Value to align.
    //! \param align - Alignment to use.
    template<class T, class TAlign = T>
    inline T AlignUp(T x, TAlign align)
    {
        auto alignT = static_cast<T>(align);
        return (x + (alignT - 1u)) & ~(alignT - 1u);
    }


    //! \brief Align up a pointer.
    //!
    //! \param x     - Value to align.
    //! \param align - Alignment to use.
    template<class T>
    inline T* AlignUpPtr(const T* x, size_t align)
    {
        return reinterpret_cast<T*>(AlignUp(reinterpret_cast<size_t>(x), align));
    }


    //! \brief Align up an integer.
    //!
    //! \param x - Value to align.
    //! \tparam TValue - Alignment to use.
    template<uint32_t TValue, class T>
    inline constexpr T AlignUp(T x)
    {
        return (x + (TValue - 1)) & ~(TValue - 1);
    }


    //! \brief Align down an integer.
    //!
    //! \param x     - Value to align.
    //! \param align - Alignment to use.
    template<class T, class TAlign = T>
    inline T AlignDown(T x, TAlign align)
    {
        return (x & ~(align - 1));
    }


    //! \brief Align down a pointer.
    //!
    //! \param x - Value to align.
    //! \param align - Alignment to use.
    template<class T>
    inline constexpr T* AlignDownPtr(const T* x, size_t align)
    {
        return reinterpret_cast<T*>(AlignDown(reinterpret_cast<size_t>(x), align));
    }


    //! \brief Align down an integer.
    //!
    //! \param x - Value to align.
    //! \tparam TValue - Alignment to use.
    template<uint32_t TValue, class T>
    inline constexpr T AlignDown(T x)
    {
        return (x & ~(TValue - 1));
    }


    //! \brief Create a bitmask.
    //!
    //! \param bitCount  - The number of ones in the created mask.
    //! \param leftShift - The number of zeros to the right of the created mask.
    template<class T>
    inline constexpr T MakeMask(T bitCount, T leftShift)
    {
        auto typeBitCount = sizeof(T) * 8;
        auto mask = bitCount == typeBitCount ? static_cast<T>(-1) : ((1 << bitCount) - 1);
        return static_cast<T>(mask << leftShift);
    }


    inline void HashCombine(size_t& /* seed */) {}


    //! \brief Combine hashes of specified values with seed.
    //!
    //! \tparam Args - Types of values.
    //!
    //! \param [in,out] seed - Initial hash value to combine with.
    //! \param [in]     args - The values to calculate hash of.
    template<typename T, typename... Args>
    inline void HashCombine(size_t& seed, const T& value, const Args&... args)
    {
        std::hash<T> hasher;
        seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        HashCombine(seed, args...);
    }
} // namespace quinte
