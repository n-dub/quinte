#pragma once
#include <Core/Base.hpp>
#include <Core/CoreMath.hpp>
#include <Core/CoreTypes.hpp>
#include <Core/Memory.hpp>
#include <cassert>
#include <concepts>
#include <format>
#include <span>
#include <string_view>
#include <type_traits>


#define QU_Unused(expr) (void)expr

#if QU_DEBUG
#    define QU_Assert(expr) assert(expr)
#    define QU_AssertMsg(expr, msg) assert(expr)
#else
#    define QU_Assert(expr) QU_Unused(expr)
#    define QU_AssertMsg(expr, msg) QU_Unused(expr)
#endif


namespace quinte
{
    //! \brief Find ceiling of x divided by y.
    template<std::unsigned_integral T1, std::unsigned_integral T2>
    inline auto CeilDivide(T1 x, T2 y) -> decltype(x / y)
    {
        return (x + y - 1) / y;
    }


    //! \brief Align up an integer.
    //!
    //! \param x     - Value to align.
    //! \param align - Alignment to use.
    template<std::unsigned_integral T, std::unsigned_integral TAlign = T>
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
    template<uint32_t TValue, std::unsigned_integral T>
    inline constexpr T AlignUp(T x)
    {
        return (x + (TValue - 1)) & ~(TValue - 1);
    }


    //! \brief Align down an integer.
    //!
    //! \param x     - Value to align.
    //! \param align - Alignment to use.
    template<std::unsigned_integral T, std::unsigned_integral TAlign = T>
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
    template<uint32_t TValue, std::unsigned_integral T>
    inline constexpr T AlignDown(T x)
    {
        return (x & ~(TValue - 1));
    }


    //! \brief Create a bitmask.
    //!
    //! \param bitCount  - The number of ones in the created mask.
    //! \param leftShift - The number of zeros to the right of the created mask.
    template<std::unsigned_integral T>
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


    //! \brief A simple type used to discard out parameters from external functions.
    template<std::semiregular T>
    struct Discard final
    {
        T Value;

        [[nodiscard]] inline T* operator&()
        {
            return &Value;
        }
    };


    template<class T, std::integral TValue, TValue TInvalidValue = std::numeric_limits<TValue>::max()>
    struct TypedHandle
    {
        using BaseType = TValue;
        inline static constexpr TValue kInvalidValue = TInvalidValue;

        TValue Value = kInvalidValue;

        inline void Reset() noexcept
        {
            Value = kInvalidValue;
        }

        inline explicit operator TValue() const noexcept
        {
            return Value;
        }

        inline explicit operator bool() const noexcept
        {
            return Value != kInvalidValue;
        }

        inline friend auto operator<=>(const TypedHandle&, const TypedHandle&) = default;
    };


    class NoCopy
    {
        NoCopy(const NoCopy&) = delete;
        NoCopy& operator=(const NoCopy&) = delete;

    public:
        NoCopy() = default;
    };


    class NoMove
    {
        NoMove(NoMove&&) = delete;
        NoMove& operator=(NoMove&&) = delete;

    public:
        NoMove() = default;
    };


    class NoCopyMove
    {
        NoCopyMove(const NoCopyMove&) = delete;
        NoCopyMove& operator=(const NoCopyMove&) = delete;
        NoCopyMove(NoCopyMove&&) = delete;
        NoCopyMove& operator=(NoCopyMove&&) = delete;

    public:
        NoCopyMove() = default;
    };


    namespace detail
    {
        template<std::invocable TFunc>
        class DeferImpl final : public NoCopyMove
        {
            TFunc m_Func;

        public:
            template<typename T>
            inline DeferImpl(T&& func)
                : m_Func(std::forward<T>(func))
            {
            }

            inline ~DeferImpl()
            {
                m_Func();
            }
        };

        struct DeferOperatorImplType final
        {
            template<std::invocable F>
            inline DeferImpl<F> operator+=(F&& f)
            {
                return DeferImpl<F>(std::forward<F>(f));
            }
        };
    } // namespace detail

#define QU_Defer const auto QU_UNIQUE_NAME(DeferredObject) = detail::DeferOperatorImplType{} += [&]


    template<class T>
    requires std::is_enum_v<T>
    inline constexpr auto enum_cast(T value) -> std::underlying_type_t<T>
    {
        return static_cast<std::underlying_type_t<T>>(value);
    }
} // namespace quinte


#define QU_ENUM_BIT_OPERATORS(Name)                                                                                              \
    inline constexpr Name operator|(Name a, Name b)                                                                              \
    {                                                                                                                            \
        return static_cast<Name>(::quinte::enum_cast(a) | ::quinte::enum_cast(b));                                               \
    }                                                                                                                            \
    inline constexpr Name& operator|=(Name& a, Name b)                                                                           \
    {                                                                                                                            \
        return a = a | b;                                                                                                        \
    }                                                                                                                            \
    inline constexpr Name operator&(Name a, Name b)                                                                              \
    {                                                                                                                            \
        return static_cast<Name>(::quinte::enum_cast(a) & ::quinte::enum_cast(b));                                               \
    }                                                                                                                            \
    inline constexpr Name& operator&=(Name& a, Name b)                                                                           \
    {                                                                                                                            \
        return a = a & b;                                                                                                        \
    }                                                                                                                            \
    inline constexpr Name operator^(Name a, Name b)                                                                              \
    {                                                                                                                            \
        return static_cast<Name>(::quinte::enum_cast(a) ^ ::quinte::enum_cast(b));                                               \
    }                                                                                                                            \
    inline constexpr Name& operator^=(Name& a, Name b)                                                                           \
    {                                                                                                                            \
        return a = a ^ b;                                                                                                        \
    }
