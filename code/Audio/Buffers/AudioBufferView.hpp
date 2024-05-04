#pragma once
#include <Audio/Buffers/BufferView.hpp>

namespace quinte
{
    class AudioBuffer;


    class AudioBufferView final : public BaseBufferView
    {
        float* m_pData = nullptr;

    public:
        inline AudioBufferView()
            : BaseBufferView(audio::DataType::Audio)
        {
        }

        AudioBufferView(float* pData, uint64_t size);
        AudioBufferView(AudioBuffer* pAudioBuffer);

        void Clear(uint64_t offset, uint64_t length) override;
        void Clear() override;

        void Read(const BaseBufferView* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length) override;
        void Read(const float* pSource, uint64_t destOffset, uint64_t length);

        void Mix(const BaseBufferView* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length) override;
        void Mix(const float* pSource, uint64_t destOffset, uint64_t length);

        void MixWithGain(const BaseBufferView* pSourceBuffer, float gain, uint64_t srcOffset, uint64_t destOffset,
                         uint64_t length);
        void MixWithGain(const float* pSource, float gain, uint64_t destOffset, uint64_t length);

        void ApplyGain(float gain, uint64_t offset, uint64_t length);

        [[nodiscard]] inline float* Data()
        {
            return std::assume_aligned<kDataAlignment>(m_pData);
        }

        [[nodiscard]] inline const float* Data() const
        {
            return std::assume_aligned<kDataAlignment>(m_pData);
        }
    };
} // namespace quinte
