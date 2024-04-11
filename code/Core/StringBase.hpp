#pragma once
#include <Core/Core.hpp>
#include <Core/Unicode.hpp>
#include <format>

namespace quinte
{
    using utf8::TChar;
    using utf8::TCharTraits;
    using utf8::TCodepoint;
    using utf8::TCodepointTraits;

    class Str;
    class String;
    class StringSlice;

    template<size_t TCapacity>
    class FixedString;

    namespace detail
    {
        class StrIterator final
        {
            friend class quinte::Str;
            friend class quinte::String;
            friend class quinte::StringSlice;

            template<size_t TCapacity>
            friend class quinte::FixedString;

            const TChar* m_Iter;

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = TCodepoint;
            using pointer = const TCodepoint*;
            using reference = const TCodepoint&;

            inline StrIterator(const TChar* iter)
                : m_Iter(iter)
            {
            }

            inline value_type operator*() const
            {
                return utf8::PeekDecode(m_Iter);
            }

            inline StrIterator& operator++()
            {
                utf8::Decode(m_Iter);
                return *this;
            }

            inline StrIterator operator++(int)
            {
                StrIterator t = *this;
                ++(*this);
                return t;
            }

            inline StrIterator& operator--()
            {
                utf8::DecodePrior(m_Iter);
                return *this;
            }

            inline StrIterator operator--(int)
            {
                StrIterator t = *this;
                utf8::DecodePrior(m_Iter);
                return t;
            }

            inline friend StrIterator operator+(StrIterator lhs, int32_t rhs)
            {
                if (rhs > 0)
                {
                    while (rhs--)
                        ++lhs;
                }
                else
                {
                    while (rhs--)
                        --lhs;
                }

                return lhs;
            }

            inline friend StrIterator operator-(const StrIterator& lhs, int32_t rhs)
            {
                return lhs + (-rhs);
            }

            inline friend bool operator==(const StrIterator& a, const StrIterator& b)
            {
                return a.m_Iter == b.m_Iter;
            };

            inline friend bool operator!=(const StrIterator& a, const StrIterator& b)
            {
                return a.m_Iter != b.m_Iter;
            };
        };
    } // namespace detail


    class Str final
    {
        using Iter = detail::StrIterator;

    public:
        [[nodiscard]] inline static size_t ByteLength(const TChar* str) noexcept
        {
            return strlen(str);
        }

        [[nodiscard]] inline static size_t Length(const TChar* str, size_t byteSize) noexcept
        {
            return utf8::Length(str, byteSize);
        }

        [[nodiscard]] inline static size_t Length(const TChar* str) noexcept
        {
            return utf8::Length(str, ByteLength(str));
        }

        [[nodiscard]] inline static int32_t ByteCompare(const TChar* lhs, const TChar* rhs) noexcept
        {
            return strcmp(lhs, rhs);
        }

        [[nodiscard]] inline static int32_t Compare(const TChar* lhs, const TChar* rhs, size_t length1, size_t length2) noexcept
        {
            return utf8::Compare(lhs, rhs, length1, length2);
        }

        [[nodiscard]] inline static int32_t Compare(const TChar* lhs, const TChar* rhs) noexcept
        {
            return Compare(lhs, rhs, ByteLength(lhs), ByteLength(rhs));
        }

        [[nodiscard]] inline static TCodepoint CodepointAt(const TChar* str, size_t size, size_t index)
        {
            const TChar* begin = str;
            const TChar* end = begin + size + 1;
            size_t cpIndex = 0;
            for (auto iter = begin; iter != end; utf8::Decode(iter), ++cpIndex)
            {
                if (cpIndex == index)
                    return utf8::PeekDecode(iter);
            }

            QUINTE_Assert(0);
            return 0;
        }

        [[nodiscard]] inline static const TChar* FindFirstOf(const TChar* str, size_t size, TCodepoint search) noexcept
        {
            const Iter e{ str + size };
            for (Iter iter = str; iter != e; ++iter)
            {
                if (*iter == search)
                {
                    return iter.m_Iter;
                }
            }

            return e.m_Iter;
        }

        [[nodiscard]] inline static const TChar* FindLastOf(const TChar* str, size_t size, TCodepoint search) noexcept
        {
            const Iter e{ str + size };
            Iter result = e;
            for (Iter iter = str; iter != e; ++iter)
            {
                if (*iter == search)
                {
                    result = iter;
                }
            }

            return result.m_Iter;
        }

        [[nodiscard]] inline static bool Contains(const TChar* str, size_t size, TCodepoint search) noexcept
        {
            return FindFirstOf(str, size, search) != str + size;
        }

        [[nodiscard]] inline static const TChar* StripLeft(const TChar* str, size_t size, const TChar* chars,
                                                           size_t charsSize) noexcept
        {
            const Iter endIter{ str + size };
            Iter result{ str };
            for (Iter iter = str; iter != endIter; ++iter)
            {
                if (!Contains(chars, charsSize, *iter))
                {
                    break;
                }

                result = iter;
                ++result;
            }

            return result.m_Iter;
        }

        [[nodiscard]] inline static const TChar* StripRight(const TChar* str, size_t size, const TChar* chars,
                                                            size_t charsSize) noexcept
        {
            const Iter beginIter{ str };
            Iter result{ str + size };
            Iter iter = result;
            --iter;
            for (; iter != beginIter; --iter)
            {
                if (!Contains(chars, charsSize, *iter))
                {
                    break;
                }

                result = iter;
            }

            return result.m_Iter;
        }
    };
} // namespace quinte
