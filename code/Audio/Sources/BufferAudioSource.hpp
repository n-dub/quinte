#pragma once
#include <Audio/Buffers/AudioBuffer.hpp>
#include <Audio/Sources/AudioSource.hpp>

namespace quinte
{
    class BufferAudioSource final : public AudioSource
    {
        Rc<AudioBuffer> m_pBuffer;

    protected:
        uint64_t ReadImpl(AudioBufferView* pDestination, uint64_t firstSampleIndex, uint64_t dstOffset,
                          uint64_t sampleCount) override;

    public:
        inline BufferAudioSource(AudioBuffer* pSourceBuffer)
            : AudioSource(pSourceBuffer->GetCapacity(), 1)
            , m_pBuffer(pSourceBuffer)
        {
        }
    };
} // namespace quinte
