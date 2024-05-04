#pragma once
#include <Audio/Base.hpp>

namespace quinte
{
    class BaseBufferView
    {
    protected:
        uint64_t m_Capacity : 48;
        uint64_t m_DataType : 2;
        uint64_t m_Silent : 1;
        uint64_t m_Written : 1;

        inline BaseBufferView(audio::DataType dataType)
            : m_Capacity(0)
            , m_DataType(enum_cast(dataType))
            , m_Silent(true)
            , m_Written(false)
        {
        }

    public:
        inline static constexpr size_t kDataAlignment = memory::kCacheLineSize;

        //! \brief Get buffer's maximum capacity.
        //!
        //! Measured in number of samples for audio buffers.
        inline uint64_t GetCapacity() const
        {
            return m_Capacity;
        }

        //! \brief Check if the buffer's silent flag is set.
        inline bool IsSilent() const
        {
            return m_Silent;
        }

        //! \brief Check if the buffer's written flag is set.
        inline bool IsWritten() const
        {
            return m_Written;
        }

        //! \brief Clear buffer data in the range [offset, offset + length)
        virtual void Clear(uint64_t offset, uint64_t length) = 0;

        //! \brief Clear the entire buffer.
        virtual void Clear() = 0;

        //! \brief Read (overwrite) buffer's data with the contents of another buffer.
        //!
        //! The offsets and the length is measured in number of samples for audio buffers.
        //!
        //! \param pSourceBuffer - The buffer to read from.
        //! \param srcOffset - Offset in the source buffer to read from.
        //! \param destOffset - Offset in this buffer.
        //! \param length - The length of the data to be written.
        virtual void Read(const BaseBufferView* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length) = 0;

        //! \brief Mix (add) buffer's data with the contents of another buffer.
        //!
        //! The offsets and the length is measured in number of samples for audio buffers.
        //!
        //! \param pSourceBuffer - The buffer to read from.
        //! \param srcOffset - Offset in the source buffer to read from.
        //! \param destOffset - Offset in this buffer.
        //! \param length - The length of the data to be written.
        virtual void Mix(const BaseBufferView* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length) = 0;
    };
} // namespace quinte
