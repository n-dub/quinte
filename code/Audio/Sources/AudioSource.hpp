#pragma once
#include <Audio/Sources/Source.hpp>

namespace quinte
{
    class AudioBufferView;


    class AudioSource : public BaseSource
    {
    protected:
        uint64_t m_Length : 48;
        uint64_t m_ChannelCount : 16;

        inline AudioSource(uint64_t length, uint32_t channelCount)
            : BaseSource(audio::DataType::Audio)
            , m_Length(length)
            , m_ChannelCount(channelCount)
        {
            // TODO: stereo sources
            QU_AssertMsg(channelCount == 1, "not implemented");
        }

        virtual uint64_t ReadImpl(AudioBufferView* pDestination, uint64_t firstSampleIndex, uint64_t dstOffset,
                                  uint64_t sampleCount) = 0;

    public:
        [[nodiscard]] inline uint64_t GetLength() const
        {
            return m_Length;
        }

        [[nodiscard]] inline uint64_t Read(AudioBufferView* pDestination, uint64_t firstSampleIndex, uint64_t dstOffset,
                                           uint64_t sampleCount, uint32_t channelIndex)
        {
            const std::lock_guard lock{ m_Mutex };
            if (channelIndex == 0)
                return ReadImpl(pDestination, firstSampleIndex, dstOffset, sampleCount);
            return 0;
        }
    };
} // namespace quinte
