#pragma once
#include <Audio/Base.hpp>
#include <Audio/Buffers/AudioBufferView.hpp>
#include <Core/Memory/MemoryPool.hpp>
#include <Core/Interface.hpp>
#include <unordered_map>

namespace quinte
{
    namespace audio
    {
        enum class PortKind : uint8_t
        {
            Hardware,
            Track,
            Plugin,
        };


        enum class PortFlags : uint8_t
        {
            None = 0,

            StereoLeft = 1 << 0,
            StereoRight = 1 << 1,

            All = StereoLeft | StereoRight,
        };


        struct PortDesc final
        {
            PortKind Kind;
            DataDirection Direction;
            PortFlags Flags;
        };


        struct PortHandle : TypedHandle<PortHandle, uint32_t>
        {
        };


        struct PortConnection final
        {
            PortHandle Source;
            PortHandle Destination;
        };
    } // namespace audio


    class Port;


    class PortManager final : public Interface<PortManager>::Registrar
    {
        friend class Port;
        friend class AudioPort;

        MemoryPool m_AudioPortPool;
        MemoryPool m_AudioBufferPool;

        size_t m_AudioBufferSize;
        std::pmr::unordered_map<uint32_t, Port*, Hasher<uint32_t>> m_PortHandleMap;

        uint32_t m_currentPortIndex = 0;

        inline uint32_t GetUniquePortID()
        {
            return m_currentPortIndex++;
        }

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

        AudioPort* NewAudioPort(const audio::PortDesc& desc);

        inline Port* FindPortByHandle(audio::PortHandle handle) const
        {
            const auto it = m_PortHandleMap.find(handle.Value);
            if (it != m_PortHandleMap.end())
                return it->second;

            return nullptr;
        }
    };
} // namespace quinte
