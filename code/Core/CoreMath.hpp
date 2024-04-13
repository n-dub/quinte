#pragma once
#include <Core/Base.hpp>
#include <cmath>
#include <concepts>
#include <gcem.hpp>
#include <limits>
#include <tuple>

namespace quinte
{
    template<class T>
    requires std::integral<T> || std::floating_point<T>
    inline constexpr T Max(const T& a, const T& b)
    {
        return a < b ? b : a;
    }


    template<class T>
    requires std::integral<T> || std::floating_point<T>
    inline constexpr T Min(const T& a, const T& b)
    {
        return b < a ? b : a;
    }


    template<class TValue, class TClamp = TValue>
    requires std::integral<TValue> && std::integral<TClamp>
        || std::floating_point<TValue> && std::floating_point<TClamp>
        inline constexpr TValue Clamp(const TValue& value, const TClamp& min, const TClamp& max)
    {
        return Min(max, Max(min, value));
    }


    template<class T>
    requires std::floating_point<T>
    inline constexpr T Lerp(T a, T b, T t)
    {
        return a + t * (b - a);
    }


    template<class T>
    requires std::floating_point<T>
    inline constexpr T ApproxEqual(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
    {
        return a < b ? (b - a) < epsilon : (a - b) < epsilon;
    }


    namespace math
    {
        template<class T>
        requires std::floating_point<T>
        inline constexpr T Pow(T base, T exp)
        {
            if (std::is_constant_evaluated())
                return gcem::pow(base, exp);
            else
                return std::pow(base, exp);
        }


        template<class T>
        requires std::floating_point<T>
        inline constexpr T Log(T x)
        {
            if (std::is_constant_evaluated())
                return gcem::log(x);
            else
                return std::log(x);
        }


        template<class T>
        requires std::floating_point<T>
        inline constexpr T Log10(T x)
        {
            if (std::is_constant_evaluated())
                return gcem::log10(x);
            else
                return std::log10(x);
        }


        template<class T>
        requires std::floating_point<T>
        inline constexpr std::pair<T, T> SinCos(T x)
        {
            if (std::is_constant_evaluated())
                return { gcem::sin(x), gcem::cos(x) };
            else
                return { std::sin(x), std::cos(x) };
        }
    } // namespace math


    namespace audio
    {
        inline constexpr float ConvertAmplitudeToFader(float value)
        {
            if (value <= 0.00001f)
                return std::numeric_limits<float>::epsilon();

            if (ApproxEqual(value, 1.0f))
                value = 1.0f + std::numeric_limits<float>::epsilon();

            constexpr float kCoeff1 = 192.0f * math::Log(2.0f);
            constexpr float kCoeff2 = math::Pow(math::Log(2.0f), 8.0f) * math::Pow(198.0f, 8.0f);
            return math::Pow(6.0f * math::Log(value) + kCoeff1, 8.0f) / kCoeff2;
        }


        inline constexpr float ConvertFaderToAmplitude(float value)
        {
            return math::Pow(2.0f, (1.0f / 6.0f) * (-192.0f + 198.0f * math::Pow(value, 1.0f / 8.0f)));
        }


        inline constexpr float ConvertAmplitudeToDBFS(float value)
        {
            return 20.0f * math::Log10(value);
        }


        inline constexpr float ConvertDBFSToAmplitude(float value)
        {
            return math::Pow(10.0f, value / 20.0f);
        }


        inline constexpr float ConvertDBFSToFader(float value)
        {
            return ConvertAmplitudeToFader(ConvertDBFSToAmplitude(value));
        }
    } // namespace audio


    namespace colors
    {
        inline constexpr uint32_t kCol32RShift = 0;
        inline constexpr uint32_t kCol32GShift = 8;
        inline constexpr uint32_t kCol32BShift = 16;
        inline constexpr uint32_t kCol32AShift = 24;

        inline constexpr uint32_t kCol32RMask = 0xFFu << kCol32RShift;
        inline constexpr uint32_t kCol32GMask = 0xFFu << kCol32GShift;
        inline constexpr uint32_t kCol32BMask = 0xFFu << kCol32BShift;
        inline constexpr uint32_t kCol32AMask = 0xFFu << kCol32AShift;


        inline constexpr uint32_t CreateU32(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            return (static_cast<uint32_t>(r) << kCol32RShift) | (static_cast<uint32_t>(g) << kCol32GShift)
                | (static_cast<uint32_t>(b) << kCol32BShift) | (static_cast<uint32_t>(a) << kCol32AShift);
        }


        inline constexpr uint8_t GetR(uint32_t col)
        {
            return (col >> kCol32RShift) & 0xFF;
        }


        inline constexpr uint32_t SetR(uint32_t col, uint8_t val)
        {
            return (col & ~kCol32RMask) + (static_cast<uint32_t>(val) << kCol32RShift);
        }


        inline constexpr uint8_t GetG(uint32_t col)
        {
            return (col >> kCol32GShift) & 0xFF;
        }


        inline constexpr uint32_t SetG(uint32_t col, uint8_t val)
        {
            return (col & ~kCol32GMask) + (static_cast<uint32_t>(val) << kCol32GShift);
        }


        inline constexpr uint8_t GetB(uint32_t col)
        {
            return (col >> kCol32BShift) & 0xFF;
        }


        inline constexpr uint32_t SetB(uint32_t col, uint8_t val)
        {
            return (col & ~kCol32BMask) + (static_cast<uint32_t>(val) << kCol32BShift);
        }


        inline constexpr uint8_t GetA(uint32_t col)
        {
            return (col >> kCol32AShift) & 0xFF;
        }


        inline constexpr uint32_t SetA(uint32_t col, uint8_t val)
        {
            return (col & ~kCol32AMask) + (static_cast<uint32_t>(val) << kCol32AShift);
        }


        inline constexpr uint32_t Dim(uint32_t col, float t)
        {
            const uint8_t a = GetA(col);
            const uint8_t r = static_cast<uint8_t>(GetR(col) * t);
            const uint8_t g = static_cast<uint8_t>(GetG(col) * t);
            const uint8_t b = static_cast<uint8_t>(GetB(col) * t);
            return CreateU32(r, g, b, a);
        }
    } // namespace colors
} // namespace quinte
