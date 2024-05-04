#include <Audio/Engine.hpp>
#include <Audio/Ports/AudioPort.hpp>
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Session.hpp>

namespace quinte
{
    PortManager::PortManager()
        : m_AudioPortPool(sizeof(AudioPort), 64)
        , m_AudioBufferSize(Session::Get()->GetAudioEngine()->GetAPI()->GetAudioBufferSize())
    {
        // TODO:
        //   1. Session::Get()->GetAudioEngine()->GetAPI()->GetAudioBufferSize() looks ugly.
        //      Also too much indirection/jumping around random pointers, probably too expensive
        //      to do this frequently.
        //
        //   2. m_AudioBufferPool doesn't really allocate nicely aligned bufferSize * 4 * 64 pages,
        //      but adds another 16 bytes for page metadata. We can do better in such scenarios:
        //      let's somehow store the metadata separately and allocate large pages aligned to
        //      virtual memory granularity directly from the OS. This should reduce some overhead.
        //
        m_AudioBufferPool.Initialize(m_AudioBufferSize * sizeof(float), 64);
    }


    AudioPort* PortManager::NewAudioPort(const audio::PortDesc& desc)
    {
        AudioPort* pResult = memory::New<AudioPort>(&m_AudioPortPool, desc);

        const uint32_t portID = GetUniquePortID();
        m_PortHandleMap[portID] = pResult;
        pResult->m_Handle = { portID };
        return pResult;
    }


    void PortManager::DeletePort(Port* pPort)
    {
        const uint32_t portHandle = pPort->m_Handle.Value;
        m_PortHandleMap.erase(portHandle);
        switch (pPort->m_DataType)
        {
        case audio::DataType::Audio:
            memory::Delete(&m_AudioPortPool, pPort, sizeof(AudioPort));
            break;
        default:
            QU_AssertMsg(false, "Not implemented");
            return;
        }
    }
} // namespace quinte
