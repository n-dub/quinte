#pragma once
#include <Audio/Sources/Source.hpp>

namespace quinte
{
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

        virtual uint64_t ReadImpl(float* pDestination, uint64_t firstSampleIndex, uint64_t sampleCount) = 0;
        virtual uint64_t WriteImpl(const float* pSource, uint64_t firstSampleIndex, uint64_t sampleCount) = 0;

    public:
        [[nodiscard]] inline uint64_t GetLength() const
        {
            return m_Length;
        }

        [[nodiscard]] inline uint64_t Read(float* pDestination, uint64_t firstSampleIndex, uint64_t sampleCount)
        {
            const std::lock_guard lock{ m_Mutex };
            return ReadImpl(pDestination, firstSampleIndex, sampleCount)
                + ReadImpl(pDestination + sampleCount, firstSampleIndex, sampleCount);
        }

        [[nodiscard]] inline uint64_t Write(const float* pSource, uint64_t firstSampleIndex, uint64_t sampleCount)
        {
            QU_Unused(pSource);
            QU_Unused(firstSampleIndex);
            QU_Unused(sampleCount);
            QU_AssertMsg(false, "not implemented");
            return 0;
        }
    };
} // namespace quinte
