#include <Audio/Ports/AudioPort.hpp>
#include <Audio/Ports/PortManager.hpp>

namespace quinte
{
    uint32_t Port::Release()
    {
        const uint32_t refCount = --m_RefCount;
        if (refCount == 0)
            Interface<PortManager>::Get()->DeletePort(this);
        return refCount;
    }


    AudioPort::~AudioPort()
    {
        if (m_BufferView.Data())
            Interface<PortManager>::Get()->DeallocateAudioBuffer(m_BufferView);
    }


    void AudioPort::AllocateBuffer()
    {
        QU_AssertDebug(m_BufferView.Data() == nullptr);
        m_BufferView = Interface<PortManager>::Get()->AllocateAudioBuffer();
    }


    StereoPorts StereoPorts::Create(const audio::PortDesc& desc)
    {
        PortManager* pPortManager = Interface<PortManager>::Get();

        StereoPorts result;
        result.Left = pPortManager->NewAudioPort(desc);
        result.Right = pPortManager->NewAudioPort(desc);
        return result;
    }
} // namespace quinte
