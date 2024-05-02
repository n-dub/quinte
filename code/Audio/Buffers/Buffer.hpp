#pragma once
#include <Core/Core.hpp>

namespace quinte
{
    class BaseBuffer : public memory::RefCountedObjectBase
    {
    protected:
        uint64_t m_Capacity : 48;
        uint64_t m_Silent : 1;
        uint64_t m_Owning : 1;
        uint64_t m_Written : 1;

        inline BaseBuffer(uint64_t capacity, bool owning)
            : m_Capacity(capacity)
            , m_Silent(true)
            , m_Owning(owning)
            , m_Written(false)
        {
        }

    public:
        inline static constexpr size_t kDataAlignment = memory::kCacheLineSize;


        virtual ~BaseBuffer() = default;

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

        //! \brief Reallocate the internal storage if the current capacity is not sufficient.
        virtual void Resize(uint64_t size) = 0;

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
        virtual void Read(const BaseBuffer* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length) = 0;

        //! \brief Mix (add) buffer's data with the contents of another buffer.
        //!
        //! The offsets and the length is measured in number of samples for audio buffers.
        //!
        //! \param pSourceBuffer - The buffer to read from.
        //! \param srcOffset - Offset in the source buffer to read from.
        //! \param destOffset - Offset in this buffer.
        //! \param length - The length of the data to be written.
        virtual void Mix(const BaseBuffer* pSourceBuffer, uint64_t srcOffset, uint64_t destOffset, uint64_t length) = 0;
    };
} // namespace quinte
