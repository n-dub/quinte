#include <Audio/Engine.hpp>
#include <Audio/Ports/AudioPort.hpp>
#include <Audio/Ports/PortManager.hpp>

namespace quinte
{
    PortManager::PortManager()
        : m_AudioPortPool(sizeof(AudioPort), 64)
        , m_AudioBufferSize(Interface<AudioEngine>::Get()->GetAPI()->GetAudioBufferSize())
    {
        // TODO:
        //   m_AudioBufferPool doesn't really allocate nicely aligned bufferSize * 4 * 64 pages,
        //   but adds another 16 bytes for page metadata. We can do better in such scenarios:
        //   let's somehow store the metadata separately and allocate large pages aligned to
        //   virtual memory granularity directly from the OS. This should reduce some overhead.
        //
        m_AudioBufferPool.Initialize(m_AudioBufferSize * sizeof(float), 64);

        const audio::PortDesc hwPortsDesc{
            .Kind = audio::PortKind::Hardware,
            .Direction = audio::DataDirection::Output,
            .Flags = audio::PortFlags::RecordingOnly,
        };

        IAudioAPI* pAPI = Interface<AudioEngine>::Get()->GetAPI();
        const audio::DeviceDesc inputDeviceDesc = pAPI->GetDeviceDesc(pAPI->GetSelectedInputDevice());

        PortManager* pPortManager = Interface<PortManager>::Get();
        m_HardwarePorts.resize(inputDeviceDesc.InputChannelCount);
        for (uint32_t portIndex = 0; portIndex < m_HardwarePorts.size(); ++portIndex)
        {
            m_HardwarePorts[portIndex] = pPortManager->NewAudioPort(hwPortsDesc);
            m_HardwarePorts[portIndex]->AllocateBuffer();
        }

        const audio::PortDesc monPortsDesc{ .Kind = audio::PortKind::Hardware, .Direction = audio::DataDirection::Input };
        m_MonitorPorts = StereoPorts::Create(monPortsDesc);
        m_MonitorPorts.Left->AllocateBuffer();
        m_MonitorPorts.Right->AllocateBuffer();
    }


    std::span<const Rc<AudioPort>> PortManager::GetHardwarePorts() const
    {
        return m_HardwarePorts;
    }


    const StereoPorts& PortManager::GetMonitorPorts() const
    {
        return m_MonitorPorts;
    }


    AudioPort* PortManager::NewAudioPort(const audio::PortDesc& desc)
    {
        AudioPort* pResult = memory::New<AudioPort>(&m_AudioPortPool, desc);

        const uint32_t portID = m_CurrentPortIndex++;
        m_PortHandleMap[portID] = pResult;
        pResult->m_Handle = { portID };
        return pResult;
    }


    void PortManager::ConnectPorts(Port* pSource, Port* pDestination)
    {
        QU_AssertDebug(pSource->IsOutput() && pDestination->IsInput());
        if (FindIndex(pSource->m_Destinations, pDestination->m_Handle) != InvalidIndex)
        {
            QU_AssertDebug(FindIndex(pDestination->m_Sources, pSource->m_Handle) != InvalidIndex);
            return;
        }

        QU_AssertDebug(FindIndex(pSource->m_Sources, pDestination->m_Handle) == InvalidIndex);
        QU_AssertDebug(FindIndex(pDestination->m_Destinations, pSource->m_Handle) == InvalidIndex);

        pSource->m_Destinations.push_back(pDestination->m_Handle);
        pDestination->m_Sources.push_back(pSource->m_Handle);
    }


    void PortManager::DeletePort(Port* pPort)
    {
        const audio::PortHandle portHandle = pPort->m_Handle;
        for (const audio::PortHandle source : pPort->m_Sources)
        {
            Port* pSource = FindPortByHandle(source);
            const size_t index = FindIndex(pSource->m_Destinations, portHandle);
            QU_AssertDebug(index != InvalidIndex);
            pSource->m_Destinations[index] = pSource->m_Destinations.back();
            pSource->m_Destinations.pop_back();
        }

        for (const audio::PortHandle destination : pPort->m_Destinations)
        {
            Port* pDestination = FindPortByHandle(destination);
            const size_t index = FindIndex(pDestination->m_Sources, portHandle);
            QU_AssertDebug(index != InvalidIndex);
            pDestination->m_Sources[index] = pDestination->m_Sources.back();
            pDestination->m_Sources.pop_back();
        }

        m_PortHandleMap.erase(portHandle.Value);
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
