#pragma once
#include <Audio/Buffers/Buffer.hpp>

namespace quinte
{
    class AudioBuffer final : public BaseBuffer
    {
        float* m_pData = nullptr;

    public:
        AudioBuffer(uint64_t size);
        ~AudioBuffer() override;

        void Resize(uint64_t size) override;
        void Clear(uint64_t offset, uint64_t length) override;
        void Clear() override;

        void Read(const BaseBuffer* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length) override;
        void Read(const float* pSource, uint64_t destOffset, uint64_t length);

        void Mix(const BaseBuffer* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length) override;
        void Mix(const float* pSource, uint64_t destOffset, uint64_t length);

        void MixWithGain(const BaseBuffer* pSourceBuffer, float gain, uint64_t srcOffset, uint64_t destOffset, uint64_t length);
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
