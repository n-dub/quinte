#pragma once
#include <Audio/Buffers/BufferView.hpp>

namespace quinte::detail
{
    inline void MixBuffersImpl(float* QU_RESTRICT pDestination, const float* QU_RESTRICT pSource, uint64_t sampleCount)
    {
        //
        // I decided not to write custom SIMD optimized algorithms for operations with buffers since Clang can
        // do this for me, especially in simple cases like this one. And it also adapts to the ISA.
        //
        // Using __restrict here greatly reduces the amount of code generated: https://godbolt.org/z/xhjM13Yo8
        // So we should keep in mind that we can't mix a buffer with itself and use asserts to ensure proper usage.
        //

        for (uint64_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            pDestination[sampleIndex] += pSource[sampleIndex];
        }
    }


    inline void MixBuffersImpl(float* QU_RESTRICT pDestination, const float* QU_RESTRICT pSource, float gain,
                               uint64_t sampleCount)
    {
        for (uint64_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            pDestination[sampleIndex] += pSource[sampleIndex] * gain;
        }
    }
} // namespace quinte::detail
