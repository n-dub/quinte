#pragma once
#include <Core/StringBase.hpp>
#include <Core/Unicode.hpp>
#include <cassert>
#include <codecvt>
#include <locale>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace quinte
{
    //! \brief A slice of String.
    class StringSlice final
    {
        const TChar* m_Data;
        size_t m_Size;

    public:
        using Iterator = detail::StrIterator;

        inline constexpr StringSlice() noexcept
            : m_Data(nullptr)
            , m_Size(0)
        {
        }

        inline constexpr StringSlice(const TChar* data, size_t size) noexcept
            : m_Data(data)
            , m_Size(size)
        {
        }

        inline constexpr StringSlice(const std::string_view& stringView) noexcept
            : m_Data(stringView.data())
            , m_Size(stringView.size())
        {
        }

        inline constexpr StringSlice(const TChar* data) noexcept
            : m_Data(data)
            , m_Size(data == nullptr ? 0 : std::char_traits<TChar>::length(data))
        {
        }

        template<size_t S>
        inline constexpr StringSlice(const TChar (&data)[S]) noexcept
            : m_Data(data)
            , m_Size(S)
        {
        }

        inline StringSlice(Iterator begin, Iterator end) noexcept
            : m_Data(begin.m_Iter)
            , m_Size(end.m_Iter - begin.m_Iter)
        {
        }

        [[nodiscard]] inline constexpr const TChar* Data() const noexcept
        {
            return m_Data;
        }

        [[nodiscard]] inline constexpr size_t Size() const noexcept
        {
            return m_Size;
        }

        // O(N)
        [[nodiscard]] inline size_t Length() const noexcept
        {
            return utf8::Length(Data(), Size());
        }

        inline StringSlice operator()(size_t beginIndex, size_t endIndex) const
        {
            auto begin = Data();
            auto end = Data();
            utf8::Advance(begin, beginIndex);
            utf8::Advance(end, endIndex);
            return StringSlice(begin, end - begin);
        }

        // O(1)
        [[nodiscard]] inline TChar ByteAt(size_t index) const
        {
            QU_AssertMsg(index < Size(), "Invalid index");
            return Data()[index];
        }

        // O(N)
        [[nodiscard]] inline TCodepoint CodePointAt(size_t index) const
        {
            return Str::CodepointAt(Data(), Size(), index);
        }

        [[nodiscard]] inline Iterator FindFirstOf(Iterator start, TCodepoint search) const noexcept
        {
            const size_t size = Size();
            const TChar* data = Data();
            QU_Assert(start.m_Iter >= data && start.m_Iter <= data + size);

            const size_t searchSize = data + size - start.m_Iter;
            return Str::FindFirstOf(start.m_Iter, searchSize, search);
        }

        [[nodiscard]] inline Iterator FindFirstOf(TCodepoint search) const noexcept
        {
            return FindFirstOf(begin(), search);
        }

        [[nodiscard]] inline Iterator FindLastOf(TCodepoint search) const noexcept
        {
            return Str::FindLastOf(Data(), Size(), search);
        }

        [[nodiscard]] inline bool Contains(TCodepoint search) const noexcept
        {
            return FindFirstOf(search) != end();
        }

        [[nodiscard]] inline bool StartsWith(StringSlice prefix, bool caseSensitive = true) const noexcept
        {
            if (prefix.Size() > Size())
                return false;

            return utf8::AreEqual(Data(), prefix.Data(), prefix.Size(), prefix.Size(), caseSensitive);
        }

        [[nodiscard]] inline bool EndsWith(StringSlice suffix, bool caseSensitive = true) const noexcept
        {
            if (suffix.Size() > Size())
                return false;

            return utf8::AreEqual(Data() + Size() - suffix.Size(), suffix.Data(), suffix.Size(), suffix.Size(), caseSensitive);
        }

        [[nodiscard]] inline std::pmr::vector<StringSlice> Split(TCodepoint c = ' ',
                                                                 std::pmr::memory_resource* allocator = nullptr) const
        {
            if (allocator == nullptr)
                allocator = std::pmr::get_default_resource();

            std::pmr::vector<StringSlice> result{ allocator };
            Split(result, c);
            return result;
        }

        inline void Split(std::pmr::vector<StringSlice>& result, TCodepoint c = ' ') const
        {
            auto current = begin();
            while (current != end())
            {
                auto cPos = FindFirstOf(current, c);
                result.emplace_back(current.m_Iter, cPos.m_Iter - current.m_Iter);
                current = cPos;
                if (current != end())
                    ++current;
            }
        }

        [[nodiscard]] inline std::pmr::vector<StringSlice> SplitLines(std::pmr::memory_resource* allocator = nullptr) const
        {
            if (allocator == nullptr)
                allocator = std::pmr::get_default_resource();

            std::pmr::vector<StringSlice> result{ allocator };
            SplitLines(result);
            return result;
        }

        inline void SplitLines(std::pmr::vector<StringSlice>& result) const
        {
            auto current = begin();
            while (current != end())
            {
                auto cPos = FindFirstOf(current, '\n');
                auto line = StringSlice{ current.m_Iter, static_cast<size_t>(cPos.m_Iter - current.m_Iter) }.StripRight("\r");
                result.emplace_back(line);
                current = cPos;
                if (current != end())
                    ++current;
            }
        }

        [[nodiscard]] inline StringSlice StripLeft(StringSlice chars = "\n\r\t ") const noexcept
        {
            return { Iterator{ Str::StripLeft(Data(), Size(), chars.Data(), chars.Size()) }, end() };
        }

        [[nodiscard]] inline StringSlice StripRight(StringSlice chars = "\n\r\t ") const noexcept
        {
            return { begin(), Iterator{ Str::StripRight(Data(), Size(), chars.Data(), chars.Size()) } };
        }

        [[nodiscard]] inline StringSlice Strip(StringSlice chars = "\n\r\t ") const noexcept
        {
            return StripLeft(chars).StripRight(chars);
        }

        [[nodiscard]] inline int Compare(const StringSlice& other) const noexcept
        {
            return utf8::Compare(Data(), other.Data(), Size(), other.Size());
        }

        [[nodiscard]] inline bool IsEqualTo(const StringSlice& other, bool caseSensitive = true) const noexcept
        {
            return utf8::AreEqual(Data(), other.Data(), Size(), other.Size(), caseSensitive);
        }

        [[nodiscard]] inline Iterator begin() const noexcept
        {
            return Iterator(Data());
        }

        [[nodiscard]] inline Iterator end() const noexcept
        {
            return Iterator(Data() + Size());
        }
    };

    inline bool operator==(const StringSlice& lhs, const StringSlice& rhs) noexcept
    {
        return lhs.Size() == rhs.Size() && lhs.Compare(rhs) == 0;
    }

    inline bool operator!=(const StringSlice& lhs, const StringSlice& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline bool operator<(const StringSlice& lhs, const StringSlice& rhs) noexcept
    {
        return lhs.Compare(rhs) < 0;
    }

    inline bool operator>(const StringSlice& lhs, const StringSlice& rhs) noexcept
    {
        return lhs.Compare(rhs) > 0;
    }

    inline bool operator<=(const StringSlice& lhs, const StringSlice& rhs) noexcept
    {
        return lhs.Size() == rhs.Size() && lhs.Compare(rhs) <= 0;
    }

    inline bool operator>=(const StringSlice& lhs, const StringSlice& rhs) noexcept
    {
        return lhs.Size() == rhs.Size() && lhs.Compare(rhs) >= 0;
    }


    template<>
    struct Hasher<StringSlice> final
    {
        inline uint64_t operator()(StringSlice str) const noexcept
        {
            return DefaultHash(str.Data(), str.Size());
        }
    };
} // namespace quinte
