#include <Audio/Buffers/AudioBuffer.hpp>
#include <Audio/Buffers/AudioBufferCommon.hpp>
#include <Audio/Buffers/AudioBufferView.hpp>

namespace quinte
{
    AudioBufferView::AudioBufferView(float* pData, uint64_t size)
        : BaseBufferView(audio::DataType::Audio)
    {
        if (size > 0)
        {
            m_pData = pData;
            m_Capacity = size;
            Clear();
            m_Silent = false;
        }
    }


    AudioBufferView::AudioBufferView(AudioBuffer* pAudioBuffer)
        : BaseBufferView(audio::DataType::Audio)
    {
        m_Capacity = pAudioBuffer->GetCapacity();
        m_Silent = pAudioBuffer->IsSilent();
        m_Written = pAudioBuffer->IsWritten();
        m_pData = pAudioBuffer->Data();
    }


    void AudioBufferView::Clear(uint64_t offset, uint64_t length)
    {
        if (!m_Silent)
        {
            QU_Assert(m_Capacity > 0);
            QU_Assert(offset + length <= m_Capacity);
            memset(m_pData + offset, 0, length * sizeof(float));
            m_Silent = length == m_Capacity;
        }

        m_Written = true;
    }


    void AudioBufferView::Clear()
    {
        Clear(0, m_Capacity);
    }


    void AudioBufferView::Read(const BaseBufferView* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length)
    {
        QU_Assert(this != pSourceBuffer);
        QU_Assert(destOffset + length <= m_Capacity);
        QU_Assert(srcOffset + length <= pSourceBuffer->GetCapacity());

        const float* pSource = static_cast<const AudioBufferView*>(pSourceBuffer)->m_pData;
        float* pDestination = m_pData;

        if (pSourceBuffer->IsSilent())
        {
            memory::Zero(pDestination + destOffset, length);
            if (m_Silent || (destOffset == 0 && length == m_Capacity))
                m_Silent = true;
        }
        else
        {
            memory::Copy(pDestination + destOffset, pSource + srcOffset, length);
        }

        m_Written = true;
    }


    void AudioBufferView::Read(const float* pSource, uint64_t destOffset, uint64_t length)
    {
        QU_Assert(pSource && length > 0);
        QU_Assert(m_Capacity >= destOffset + length);

        memory::Copy(m_pData + destOffset, pSource, length);
        m_Silent = false;
        m_Written = true;
    }


    void AudioBufferView::Mix(const BaseBufferView* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length)
    {
        QU_Assert(this != pSourceBuffer);
        QU_Assert(destOffset + length <= m_Capacity);
        QU_Assert(srcOffset + length <= pSourceBuffer->GetCapacity());

        if (pSourceBuffer->IsSilent())
            return;

        const float* pSource = static_cast<const AudioBufferView*>(pSourceBuffer)->m_pData;
        float* pDestination = m_pData;

        detail::MixBuffersImpl(pDestination + destOffset, pSource + srcOffset, length);
        m_Silent = pSourceBuffer->IsSilent() && IsSilent();
        m_Written = true;
    }


    void AudioBufferView::Mix(const float* pSource, uint64_t destOffset, uint64_t length)
    {
        QU_Assert(pSource && length > 0);
        QU_Assert(m_Capacity >= destOffset + length);

        detail::MixBuffersImpl(m_pData + destOffset, pSource, length);
        m_Silent = false;
        m_Written = true;
    }


    void AudioBufferView::MixWithGain(const BaseBufferView* pSourceBuffer, float gain, uint64_t srcOffset, uint64_t destOffset,
                                      uint64_t length)
    {
        QU_Assert(this != pSourceBuffer);
        QU_Assert(destOffset + length <= m_Capacity);
        QU_Assert(srcOffset + length <= pSourceBuffer->GetCapacity());

        if (pSourceBuffer->IsSilent() || gain == 0.0f)
        {
            Clear(destOffset, length);
            return;
        }

        const float* pSource = static_cast<const AudioBufferView*>(pSourceBuffer)->m_pData;
        float* pDestination = m_pData;

        detail::MixBuffersImpl(pDestination + destOffset, pSource + srcOffset, gain, length);
        m_Silent = false;
        m_Written = true;
    }


    void AudioBufferView::MixWithGain(const float* pSource, float gain, uint64_t destOffset, uint64_t length)
    {
        QU_Assert(pSource && length > 0);
        QU_Assert(m_Capacity >= destOffset + length);

        if (gain == 0.0f)
        {
            Clear(destOffset, length);
            return;
        }

        detail::MixBuffersImpl(m_pData + destOffset, pSource, gain, length);
        m_Silent = false;
        m_Written = true;
    }


    void AudioBufferView::ApplyGain(float gain, uint64_t offset, uint64_t length)
    {
        if (gain == 0.0f)
        {
            memory::Zero(m_pData + offset, length);
            if (offset == 0 && length == m_Capacity)
                m_Silent = true;
            return;
        }

        for (uint64_t sampleIndex = 0; sampleIndex < length; ++sampleIndex)
        {
            m_pData[sampleIndex + offset] *= gain;
        }
    }
} // namespace quinte
