#include <Audio/Sources/BufferAudioSource.hpp>

namespace quinte
{
    uint64_t BufferAudioSource::ReadImpl(float* pDestination, uint64_t firstSampleIndex, uint64_t sampleCount)
    {
        const uint64_t actualSampleCount = Min(firstSampleIndex + sampleCount, m_Length) - firstSampleIndex;
        memory::Copy(pDestination, m_pBuffer->Data(), actualSampleCount);
        return actualSampleCount;
    }


    uint64_t BufferAudioSource::WriteImpl(const float* pSource, uint64_t firstSampleIndex, uint64_t sampleCount)
    {
        const uint64_t actualSampleCount = Min(firstSampleIndex + sampleCount, m_Length) - firstSampleIndex;
        m_pBuffer->Read(pSource, firstSampleIndex, actualSampleCount);
        return actualSampleCount;
    }
} // namespace quinte
