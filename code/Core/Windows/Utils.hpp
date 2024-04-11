#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <Core/StringSlice.hpp>

namespace quinte::windows
{
    template<size_t TSize>
    struct WideString
    {
        WCHAR Data[TSize + 1];

        inline WideString(StringSlice str)
        {
            int count = MultiByteToWideChar(CP_UTF8, 0, str.Data(), str.Size(), nullptr, 0);
            QUINTE_Assert(count >= 0 && count <= TSize);

            MultiByteToWideChar(CP_UTF8, 0, str.Data(), str.Size(), Data, count);
            Data[count] = 0;
        }
    };

    using WideStr = WideString<512>;
    using WidePath = WideString<MAX_PATH>;
} // namespace quinte::windows
