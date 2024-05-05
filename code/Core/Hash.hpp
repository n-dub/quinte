#pragma once
#include <concepts>
#include <string_view>

namespace quinte
{
    namespace detail
    {
        // Our hash functions are based on WyHash: https://github.com/wangyi-fudan/wyhash
        // WyHash is released into public domain.
        // Here is a gist with my compile-time implementation: https://gist.github.com/n-dub/56acb5aa3665fefd9dd6ca21c2748886
        //
        // The code below assumes little-endian and implements only the most basic functionality of the original
        // library: string hashes and uint64_t hashes.
        //
        // The reason why i rewrote it instead of just adding existing header is that we need to compute hashes of
        // types at compile type (see Core/Interface.hpp).
        //


        constexpr uint64_t kDefaultSecret[] = {
            UINT64_C(0x2d358dccaa6c78a5), UINT64_C(0x8bb84b93962eacc9), UINT64_C(0x4b33a62ed433d4a3), UINT64_C(0x4d5a2da51de1aa47)
        };

        inline constexpr uint64_t WyRead4(const char* p)
        {
            return static_cast<uint64_t>(p[0]) | (static_cast<uint64_t>(p[1]) << 8) | (static_cast<uint64_t>(p[2]) << 16)
                | (static_cast<uint64_t>(p[3]) << 24);
        }

        inline constexpr uint64_t WyRead8(const char* p)
        {
            return static_cast<uint64_t>(p[0]) | (static_cast<uint64_t>(p[1]) << 8) | (static_cast<uint64_t>(p[2]) << 16)
                | (static_cast<uint64_t>(p[3]) << 24) | (static_cast<uint64_t>(p[4]) << 32) | (static_cast<uint64_t>(p[5]) << 40)
                | (static_cast<uint64_t>(p[6]) << 48) | (static_cast<uint64_t>(p[7]) << 56);
        }

        inline constexpr uint64_t WyRead3(const char* p, size_t k)
        {
            return ((static_cast<uint64_t>(p[0])) << 16) | ((static_cast<uint64_t>(p[k >> 1])) << 8) | p[k - 1];
        }

        inline constexpr uint64_t WyMix(uint64_t A, uint64_t B)
        {
            __uint128_t r = A;
            r *= B;
            A = static_cast<uint64_t>(r);
            B = static_cast<uint64_t>(r >> 64);
            return A ^ B;
        }

        inline constexpr uint64_t WyHash64(uint64_t A, uint64_t B)
        {
            A ^= UINT64_C(0x2d358dccaa6c78a5);
            B ^= UINT64_C(0x8bb84b93962eacc9);

            __uint128_t r = A;
            r *= B;
            A = static_cast<uint64_t>(r);
            B = static_cast<uint64_t>(r >> 64);

            return WyMix(A ^ UINT64_C(0x2d358dccaa6c78a5), B ^ UINT64_C(0x8bb84b93962eacc9));
        }

        // This is useful for function signatures when compiling on Windows. `std::string_view` is a template class
        // (`std::basic_string_view< ... >`) which makes it difficult to retrieve typename from a template function
        // signature since it has multiple templates.
        struct SVWrapper final
        {
            std::string_view value;
        };

        //! \brief Remove leading and trailing spaces from a string view.
        inline constexpr std::string_view TrimTypeName(std::string_view name)
        {
            name.remove_prefix(name.find_first_not_of(' '));
            name.remove_suffix(name.length() - name.find_last_not_of(' ') - 1);
            return name;
        }

        template<class T>
        inline constexpr SVWrapper TypeNameImpl()
        {
#if FE_COMPILER_MSVC
            std::string_view fn = __FUNCSIG__;
            fn.remove_prefix(fn.find_first_of("<") + 1);
            fn.remove_suffix(fn.length() - fn.find_last_of(">"));
#else
            std::string_view fn = __PRETTY_FUNCTION__;
            fn.remove_prefix(fn.find_first_of('=') + 1);
            fn.remove_suffix(fn.length() - fn.find_last_of(']'));
#endif
            return SVWrapper{ TrimTypeName(fn) };
        }
    } // namespace detail


    inline constexpr uint64_t DefaultHash(const char* p, size_t len) noexcept
    {
        uint64_t seed = detail::kDefaultSecret[0], a, b;
        if (len <= 16) [[likely]]
        {
            if (len >= 4) [[likely]]
            {
                a = (detail::WyRead4(p) << 32) | detail::WyRead4(p + ((len >> 3) << 2));
                b = (detail::WyRead4(p + len - 4) << 32) | detail::WyRead4(p + len - 4 - ((len >> 3) << 2));
            }
            else if (len > 0) [[likely]]
            {
                a = detail::WyRead3(p, len);
                b = 0;
            }
            else
            {
                a = b = 0;
            }
        }
        else
        {
            size_t i = len;
            if (i > 48) [[likely]]
            {
                uint64_t see1 = seed, see2 = seed;
                do
                {
                    seed = detail::WyMix(detail::WyRead8(p) ^ detail::kDefaultSecret[1], detail::WyRead8(p + 8) ^ seed);
                    see1 = detail::WyMix(detail::WyRead8(p + 16) ^ detail::kDefaultSecret[2], detail::WyRead8(p + 24) ^ see1);
                    see2 = detail::WyMix(detail::WyRead8(p + 32) ^ detail::kDefaultSecret[3], detail::WyRead8(p + 40) ^ see2);
                    p += 48;
                    i -= 48;
                }
                while (i > 48);
                seed ^= see1 ^ see2;
            }
            while (i > 16)
            {
                seed = detail::WyMix(detail::WyRead8(p) ^ detail::kDefaultSecret[1], detail::WyRead8(p + 8) ^ seed);
                i -= 16;
                p += 16;
            }
            a = detail::WyRead8(p + i - 16);
            b = detail::WyRead8(p + i - 8);
        }

        return detail::WyMix(detail::kDefaultSecret[1] ^ len, detail::WyMix(a ^ detail::kDefaultSecret[1], b ^ seed));
    }


    inline constexpr uint64_t DefaultHash(std::string_view str) noexcept
    {
        return DefaultHash(str.data(), str.length());
    }


    inline constexpr uint64_t DefaultHash(std::integral auto number) noexcept
    {
        return detail::WyHash64(static_cast<uint64_t>(number), detail::kDefaultSecret[0]);
    }


    inline uint64_t DefaultHash(const void* pointer) noexcept
    {
        return detail::WyHash64(reinterpret_cast<uint64_t>(pointer), detail::kDefaultSecret[0]);
    }


    template<class T>
    inline constexpr std::string_view TypeName = detail::TypeNameImpl<T>().value;


    template<class T>
    inline constexpr uint64_t TypeNameHash = DefaultHash(TypeName<T>);


    template<class T>
    struct Hasher final
    {
    };

    template<std::integral T>
    struct Hasher<T> final
    {
        inline uint64_t operator()(const T& value) const noexcept
        {
            return DefaultHash(value);
        }
    };

    template<>
    struct Hasher<const char*> final
    {
        inline uint64_t operator()(const char* value) const noexcept
        {
            return DefaultHash(value, strlen(value));
        }
    };

    template<class T>
    struct Hasher<T*> final
    {
        inline uint64_t operator()(T* value) const noexcept
        {
            return DefaultHash(static_cast<void*>(value));
        }
    };

    template<class T>
    struct Hasher<const T*> final
    {
        inline uint64_t operator()(const T* value) const noexcept
        {
            return DefaultHash(static_cast<void*>(value));
        }
    };


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
