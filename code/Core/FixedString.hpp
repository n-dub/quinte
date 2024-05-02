#pragma once
#include <Core/StringSlice.hpp>

namespace quinte
{
    //! \brief String class that never allocates and uses UTF-8 encoding.
    template<size_t TCapacity>
    class FixedString final
    {
        static_assert(TCapacity <= (1 << 24));

        TChar m_Data[TCapacity];
        uint32_t m_Zero : 8;
        uint32_t m_Size : 24;

        template<size_t TFmtCapacity>
        friend class FixedFmt;

    public:
        inline static constexpr size_t Cap = TCapacity;

        using Iterator = detail::StrIterator;

        inline FixedString() noexcept
            : m_Zero(0)
            , m_Size(0)
        {
            *m_Data = 0;
        }

        inline FixedString(size_t length, TChar value) noexcept
            : m_Zero(0)
        {
            QU_Assert(length <= TCapacity);
            memory::Set(m_Data, value, length);
            m_Size = static_cast<uint32_t>(length);
            m_Data[m_Size] = 0;
        }

        inline FixedString(const TChar* str, size_t byteSize) noexcept
            : m_Zero(0)
        {
            QU_Assert(byteSize <= TCapacity);
            memory::Copy(m_Data, str, byteSize);
            m_Size = static_cast<uint32_t>(byteSize);
            m_Data[m_Size] = 0;
        }

        inline FixedString(StringSlice slice) noexcept
            : FixedString(slice.Data(), slice.Size())
        {
        }

        inline FixedString(const TChar* str) noexcept
            : FixedString(str, TCharTraits::length(str))
        {
        }

        inline FixedString(Iterator begin, Iterator end) noexcept
            : FixedString(begin.m_Iter, end.m_Iter - begin.m_Iter)
        {
        }

        [[nodiscard]] inline const TChar* Data() const noexcept
        {
            return m_Data;
        }

        [[nodiscard]] inline TChar* Data() noexcept
        {
            return m_Data;
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
            auto begin = Data();
            auto end = begin + Size() + 1;
            size_t cpIndex = 0;
            for (auto iter = begin; iter != end; utf8::Decode(iter), ++cpIndex)
            {
                if (cpIndex == index)
                    return utf8::PeekDecode(iter);
            }

            QU_AssertMsg(0, "Invalid index");
            return 0;
        }

        // O(1)
        [[nodiscard]] inline size_t Size() const noexcept
        {
            return m_Size;
        }

        [[nodiscard]] inline bool Empty() const noexcept
        {
            return Size() == 0;
        }

        // O(1)
        [[nodiscard]] inline size_t Capacity() const noexcept
        {
            return TCapacity;
        }

        // O(N)
        [[nodiscard]] inline size_t Length() const noexcept
        {
            auto ptr = Data();
            return utf8::Length(ptr, Size());
        }

        inline operator StringSlice() const noexcept
        {
            return { Data(), Size() };
        }

        inline StringSlice operator()(size_t beginIndex, size_t endIndex) const
        {
            auto begin = Data();
            auto end = Data();
            utf8::Advance(begin, beginIndex);
            utf8::Advance(end, endIndex);
            return StringSlice(begin, end - begin);
        }

        [[nodiscard]] inline StringSlice ASCIISubstring(size_t beginIndex, size_t endIndex) const
        {
            auto begin = Data() + beginIndex;
            auto end = Data() + endIndex;
            return StringSlice(begin, end - begin);
        }

        inline void Clear() noexcept
        {
            m_Size = 0;
            *m_Data = 0;
        }

        inline FixedString& Append(const TChar* str, size_t count)
        {
            QU_AssertMsg(count == 0 || str != nullptr, "Couldn't append more than 0 chars from a null string");
            QU_Assert(count + Size() <= Capacity());
            if (count == 0)
                return *this;

            memory::Copy(Data() + Size(), str, count);
            m_Size += static_cast<uint32_t>(count);
            m_Data[m_Size] = 0;
            return *this;
        }

        inline FixedString& Append(StringSlice str)
        {
            return Append(str.Data(), str.Size());
        }

        inline FixedString& Append(TChar cp)
        {
            QU_Assert(Size() < Capacity());
        }

        inline FixedString& Append(const TChar* str)
        {
            return Append(str, std::char_traits<TChar>::length(str));
        }

        inline void Resize(size_t length, TChar value) noexcept
        {
            QU_Assert(length <= TCapacity);
            memset(m_Data, value, length);
            m_Size = static_cast<uint32_t>(length);
            m_Data[m_Size] = 0;
        }

        inline FixedString& operator+=(StringSlice str)
        {
            return Append(str);
        }

        inline FixedString& operator/=(const StringSlice& str)
        {
            Append('/');
            return Append(str);
        }

        inline friend FixedString operator+(const FixedString& lhs, StringSlice rhs)
        {
            FixedString t{ lhs };
            t += rhs;
            return t;
        }

        inline friend FixedString operator/(const FixedString& lhs, StringSlice rhs)
        {
            FixedString t{ lhs };
            t /= rhs;
            return t;
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

        [[nodiscard]] inline std::pmr::vector<StringSlice> Split(TCodepoint c = ' ',
                                                                 std::pmr::memory_resource* allocator = nullptr) const
        {
            std::pmr::vector<StringSlice> result{ allocator };
            Split(result, c);
            return result;
        }

        inline void Split(std::pmr::vector<StringSlice>& result, TCodepoint c = ' ') const
        {
            StringSlice(Data(), Size()).Split(result, c);
        }

        [[nodiscard]] inline std::pmr::vector<StringSlice> SplitLines(std::pmr::memory_resource* allocator = nullptr) const
        {
            std::pmr::vector<StringSlice> result{ allocator };
            SplitLines(result);
            return result;
        }

        inline void SplitLines(std::pmr::vector<StringSlice>& result) const
        {
            StringSlice(Data(), Size()).SplitLines(result);
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

        [[nodiscard]] inline int32_t Compare(const StringSlice& other) const noexcept
        {
            return utf8::Compare(Data(), other.Data(), Size(), other.Size());
        }

        [[nodiscard]] inline bool IsEqualTo(const StringSlice& other, bool caseSensitive = true) const noexcept
        {
            return utf8::AreEqual(Data(), other.Data(), Size(), other.Size(), caseSensitive);
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

        [[nodiscard]] inline static FixedString Join(const StringSlice& separator, const std::span<StringSlice>& strings)
        {
            FixedString result;
            for (size_t i = 0; i < strings.size(); ++i)
            {
                result.Append(strings[i]);
                if (i != strings.size() - 1)
                    result.Append(separator);
            }

            return result;
        }

        [[nodiscard]] inline Iterator begin() const noexcept
        {
            auto ptr = Data();
            return Iterator(ptr);
        }

        [[nodiscard]] inline Iterator end() const noexcept
        {
            auto ptr = Data();
            auto size = Size();
            return Iterator(ptr + size);
        }
    };


    template<size_t TCapacity>
    class FixedFmt final
    {
        FixedString<TCapacity> m_Data;

    public:
        template<class... TArgs>
        inline FixedFmt(std::string_view fmt, TArgs&&... args)
        {
            TChar* ptr = std::vformat_to(m_Data.Data(), fmt, std::make_format_args(std::forward<TArgs>(args)...));
            m_Data.m_Size = static_cast<uint32_t>(ptr - m_Data.Data());
            QU_Assert(m_Data.m_Size <= TCapacity);
            *ptr = '\0';
        }

        [[nodiscard]] inline const FixedString<TCapacity>& Get() const noexcept
        {
            return m_Data;
        }

        [[nodiscard]] inline const TChar* Data() const noexcept
        {
            return m_Data.Data();
        }

        [[nodiscard]] inline const TChar* DataEnd() const noexcept
        {
            return m_Data.Data() + m_Data.m_Size;
        }

        inline operator StringSlice() const noexcept
        {
            return { m_Data.Data(), m_Data.Size() };
        }

        [[nodiscard]] inline detail::StrIterator begin() const noexcept
        {
            return m_Data.begin();
        }

        [[nodiscard]] inline detail::StrIterator end() const noexcept
        {
            return m_Data.end();
        }
    };


    using FixStr32 = FixedString<32>;
    using FixStr64 = FixedString<64>;
    using FixStr128 = FixedString<128>;
    using FixStr256 = FixedString<256>;
    using FixStr512 = FixedString<512>;

    using FixFmt32 = FixedFmt<32>;
    using FixFmt64 = FixedFmt<64>;
    using FixFmt128 = FixedFmt<128>;
    using FixFmt256 = FixedFmt<256>;
    using FixFmt512 = FixedFmt<512>;
} // namespace quinte

namespace std
{
    template<size_t TCapacity>
    struct hash<quinte::FixedString<TCapacity>>
    {
        inline size_t operator()(const quinte::FixedString<TCapacity>& str) const noexcept
        {
            std::hash<std::string_view> hasher;
            return hasher(std::string_view(str.Data(), str.Size()));
        }
    };
} // namespace std
