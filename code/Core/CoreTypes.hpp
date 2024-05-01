#pragma once
#include <Core/Base.hpp>

namespace quinte
{
    struct Int24 final
    {
        static constexpr uint32_t kByteCount = 3;

        uint8_t Data[kByteCount];

        inline Int24() = default;

        inline Int24(int32_t value)
        {
            Data[0] = static_cast<uint8_t>((value & 0x0000ff) >> 0);
            Data[1] = static_cast<uint8_t>((value & 0x00ff00) >> 8);
            Data[2] = static_cast<uint8_t>((value & 0xff0000) >> 16);
        }

        inline explicit Int24(float value)
            : Int24(static_cast<int32_t>(value))
        {
        }

        inline explicit Int24(double value)
            : Int24(static_cast<int32_t>(value))
        {
        }

        inline explicit Int24(int64_t value)
            : Int24(static_cast<int32_t>(value))
        {
        }

        inline Int24& operator=(int32_t value)
        {
            Data[0] = static_cast<uint8_t>((value & 0x0000ff) >> 0);
            Data[1] = static_cast<uint8_t>((value & 0x00ff00) >> 8);
            Data[2] = static_cast<uint8_t>((value & 0xff0000) >> 16);
            return *this;
        }

        inline explicit operator float() const
        {
            return static_cast<float>(static_cast<int32_t>(*this));
        }

        inline explicit operator double() const
        {
            return static_cast<double>(static_cast<int32_t>(*this));
        }

        inline explicit operator int8_t() const
        {
            return static_cast<int8_t>(static_cast<int32_t>(*this));
        }

        inline explicit operator int16_t() const
        {
            return static_cast<int16_t>(static_cast<int32_t>(*this));
        }

        inline explicit operator int32_t() const
        {
            const int32_t result = Data[0] | (Data[1] << 8) | (Data[2] << 16);
            return result & 0x800000 ? result | ~0xffffff : result;
        }

        inline explicit operator int64_t() const
        {
            return static_cast<int32_t>(*this);
        }

        inline friend auto operator<=>(Int24 lhs, Int24 rhs)
        {
            return static_cast<int32_t>(lhs) <=> static_cast<int32_t>(rhs);
        }

        // clang-format off
        inline friend Int24 operator+(Int24 lhs, Int24 rhs) { return static_cast<int32_t>(lhs) + static_cast<int32_t>(rhs); }
        inline friend Int24 operator-(Int24 lhs, Int24 rhs) { return static_cast<int32_t>(lhs) - static_cast<int32_t>(rhs); }
        inline friend Int24 operator*(Int24 lhs, Int24 rhs) { return static_cast<int32_t>(lhs) * static_cast<int32_t>(rhs); }
        inline friend Int24 operator/(Int24 lhs, Int24 rhs) { return static_cast<int32_t>(lhs) / static_cast<int32_t>(rhs); }
        inline friend Int24 operator&(Int24 lhs, Int24 rhs) { return static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs); }
        inline friend Int24 operator|(Int24 lhs, Int24 rhs) { return static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs); }
        inline friend Int24 operator^(Int24 lhs, Int24 rhs) { return static_cast<int32_t>(lhs) ^ static_cast<int32_t>(rhs); }
        inline friend Int24 operator<<(Int24 lhs, Int24 rhs) { return static_cast<int32_t>(lhs) << static_cast<int32_t>(rhs); }
        inline friend Int24 operator>>(Int24 lhs, Int24 rhs) { return static_cast<int32_t>(lhs) >> static_cast<int32_t>(rhs); }

        inline Int24& operator+=(Int24 other) { return *this = *this + other; }
        inline Int24& operator-=(Int24 other) { return *this = *this - other; }
        inline Int24& operator*=(Int24 other) { return *this = *this * other; }
        inline Int24& operator/=(Int24 other) { return *this = *this / other; }
        inline Int24& operator&=(Int24 other) { return *this = *this & other; }
        inline Int24& operator|=(Int24 other) { return *this = *this | other; }
        inline Int24& operator^=(Int24 other) { return *this = *this ^ other; }
        inline Int24& operator<<=(Int24 other) { return *this = *this << other; }
        inline Int24& operator>>=(Int24 other) { return *this = *this >> other; }
        // clang-format on
    };

    using int24_t = Int24;
} // namespace quinte
