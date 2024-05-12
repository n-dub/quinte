#pragma once
#include <Audio/Base.hpp>
#include <Audio/Buffers/AudioBufferView.hpp>
#include <Audio/Ports/AudioPort.hpp>
#include <Core/Interface.hpp>
#include <Core/Memory/MemoryPool.hpp>
#include <unordered_map>

namespace quinte
{
    class PortManager final : public Interface<PortManager>::Registrar
    {
        friend class Port;
        friend class AudioPort;

        MemoryPool m_AudioPortPool;
        MemoryPool m_AudioBufferPool;

        size_t m_AudioBufferSize;
        std::pmr::unordered_map<uint32_t, Port*, Hasher<uint32_t>> m_PortHandleMap;

        uint32_t m_CurrentPortIndex = 0;

        SmallVector<Rc<AudioPort>> m_HardwarePorts;
        StereoPorts m_MonitorPorts;

        void DeletePort(Port* pPort);

        inline AudioBufferView AllocateAudioBuffer()
        {
            return { memory::NewArray<float>(&m_AudioBufferPool, m_AudioBufferSize), m_AudioBufferSize };
        }

        inline void DeallocateAudioBuffer(AudioBufferView& audioBufferView)
        {
            memory::DeleteArray(&m_AudioBufferPool, audioBufferView.Data(), m_AudioBufferSize);
            audioBufferView = {};
        }

    public:
        PortManager();
        ~PortManager() = default;

        std::span<const Rc<AudioPort>> GetHardwarePorts() const;
        const StereoPorts& GetMonitorPorts() const;

        AudioPort* NewAudioPort(const audio::PortDesc& desc);

        void ConnectPorts(Port* pSource, Port* pDestination);

        inline void ConnectPorts(audio::PortHandle source, Port* pDestination)
        {
            ConnectPorts(FindPortByHandle(source), pDestination);
        }

        inline void ConnectPorts(Port* pSource, audio::PortHandle destination)
        {
            ConnectPorts(pSource, FindPortByHandle(destination));
        }

        inline void ConnectPorts(audio::PortHandle source, audio::PortHandle destination)
        {
            ConnectPorts(FindPortByHandle(source), FindPortByHandle(destination));
        }

        inline Port* FindPortByHandle(audio::PortHandle handle) const
        {
            const auto it = m_PortHandleMap.find(handle.Value);
            if (it != m_PortHandleMap.end())
                return it->second;

            return nullptr;
        }
    };
} // namespace quinte
