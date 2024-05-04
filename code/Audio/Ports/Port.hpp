#pragma once
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Session.hpp>

namespace quinte
{
    class BaseBufferView;


    class Port
    {
        friend class PortManager;

        audio::PortHandle m_Handle;
        std::atomic<uint32_t> m_RefCount;
        audio::PortDesc m_Desc;
        audio::DataType m_DataType;

    protected:
        inline Port(const audio::PortDesc& desc, audio::DataType dataType)
            : m_Desc(desc)
            , m_DataType(dataType)
        {
        }

    public:
        virtual ~Port() = default;

        inline uint32_t AddRef()
        {
            return ++m_RefCount;
        }

        inline uint32_t Release()
        {
            const uint32_t refCount = --m_RefCount;
            if (refCount == 0)
                Session::Get()->GetPortManager()->DeletePort(this);
            return refCount;
        }

        virtual BaseBufferView* GetBufferView() = 0;
        virtual const BaseBufferView* GetBufferView() const = 0;

        virtual void AllocateBuffer() = 0;
    };
} // namespace quinte
