#pragma once
#include <Audio/Base.hpp>
#include <Core/Threading.hpp>

namespace quinte
{
    class BaseSource : public memory::RefCountedObjectBase
    {
        audio::DataType m_DataType;

    protected:
        threading::Mutex m_Mutex;

        inline BaseSource(audio::DataType dataType)
            : m_DataType(dataType)
        {
        }

    public:
        [[nodiscard]] inline audio::DataType GetDataType() const
        {
            return m_DataType;
        }
    };
} // namespace quinte
