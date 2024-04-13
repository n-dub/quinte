#pragma once
#include <Core/Core.hpp>

namespace quinte
{
    class AudioRingBuffer final : public NoCopyMove
    {
        uint8_t* m_pBuffer = nullptr;
        uint32_t m_BufferByteSize = 0;
        uint32_t m_BufferSize = 0;
        uint32_t m_InIndex = 0;
        uint32_t m_OutIndex = 0;

    public:
        inline ~AudioRingBuffer()
        {
            if (m_pBuffer)
                memory::DefaultDeleteArray(m_pBuffer, m_BufferByteSize);

            m_pBuffer = nullptr;
        }

        inline void Initialize(uint32_t bufferSize, uint32_t formatSize)
        {
            if (m_pBuffer)
                memory::DefaultDeleteArray(m_pBuffer, m_BufferByteSize);

            m_BufferByteSize = bufferSize * formatSize;
            m_pBuffer = memory::DefaultNewArray<uint8_t>(m_BufferByteSize);
            m_BufferSize = bufferSize;
            m_InIndex = 0;
            m_OutIndex = 0;
        }

        inline bool Push(const uint8_t* pSourceBuffer, uint32_t bufferSize, uint32_t formatSize)
        {
            if (bufferSize == 0 || bufferSize > m_BufferSize)
                return false;

            const uint32_t inIndexEnd = m_InIndex + bufferSize;

            uint32_t relOutIndex = m_OutIndex;
            if (relOutIndex < m_InIndex && inIndexEnd >= m_BufferSize)
                relOutIndex += m_BufferSize;

            if (m_InIndex < relOutIndex && inIndexEnd >= relOutIndex)
                return false;

            int32_t fromZeroSize = m_InIndex + bufferSize - m_BufferSize;
            fromZeroSize = fromZeroSize < 0 ? 0 : fromZeroSize;

            const int32_t fromInSize = bufferSize - fromZeroSize;

            memcpy(m_pBuffer + m_InIndex * formatSize, pSourceBuffer, static_cast<size_t>(fromInSize) * formatSize);
            memcpy(m_pBuffer, pSourceBuffer + fromInSize * formatSize, static_cast<size_t>(fromZeroSize) * formatSize);

            m_InIndex += bufferSize;
            m_InIndex %= m_BufferSize;
            return true;
        }

        inline bool Pull(uint8_t* pBuffer, uint32_t bufferSize, uint32_t formatSize)
        {
            if (bufferSize == 0 || bufferSize > m_BufferSize)
                return false;

            const uint32_t outIndexEnd = m_OutIndex + bufferSize;

            uint32_t relInIndex = m_InIndex;
            if (relInIndex < m_OutIndex && outIndexEnd >= m_BufferSize)
                relInIndex += m_BufferSize;

            if (m_OutIndex <= relInIndex && outIndexEnd > relInIndex)
                return false;

            int32_t fromZeroSize = m_OutIndex + bufferSize - m_BufferSize;
            fromZeroSize = fromZeroSize < 0 ? 0 : fromZeroSize;

            const int32_t fromOutSize = bufferSize - fromZeroSize;

            memcpy(pBuffer, m_pBuffer + m_OutIndex * formatSize, static_cast<size_t>(fromOutSize) * formatSize);
            memcpy(pBuffer + fromOutSize * formatSize, m_pBuffer, static_cast<size_t>(fromZeroSize) * formatSize);

            m_OutIndex += bufferSize;
            m_OutIndex %= m_BufferSize;
            return true;
        }
    };
} // namespace quinte
