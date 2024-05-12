#include <Audio/Buffers/AudioBufferView.hpp>
#include <Audio/Sources/BufferAudioSource.hpp>

namespace quinte
{
    uint64_t BufferAudioSource::ReadImpl(AudioBufferView* pDestination, uint64_t firstSampleIndex, uint64_t dstOffset,
                                         uint64_t sampleCount)
    {
        const uint64_t actualSampleCount = Min(firstSampleIndex + sampleCount, m_Length) - firstSampleIndex;
        pDestination->Read(m_pBuffer->Data() + firstSampleIndex, dstOffset, actualSampleCount);
        return actualSampleCount;
    }
} // namespace quinte
