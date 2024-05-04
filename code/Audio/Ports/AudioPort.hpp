#pragma once
#include <Audio/Buffers/AudioBufferView.hpp>
#include <Audio/Ports/Port.hpp>

namespace quinte
{
    class AudioPort final : public Port
    {
        AudioBufferView m_BufferView;

    public:
        inline AudioPort(const audio::PortDesc& desc)
            : Port(desc, audio::DataType::Audio)
        {
        }

        inline ~AudioPort() override
        {
            if (m_BufferView.Data())
                Session::Get()->GetPortManager()->DeallocateAudioBuffer(m_BufferView);
        }

        inline AudioBufferView* GetBufferView() override
        {
            return &m_BufferView;
        }

        inline const AudioBufferView* GetBufferView() const override
        {
            return &m_BufferView;
        }

        inline void AllocateBuffer() override
        {
            m_BufferView = Session::Get()->GetPortManager()->AllocateAudioBuffer();
        }
    };


    struct StereoPorts final
    {
        Rc<AudioPort> Left;
        Rc<AudioPort> Right;

        inline static StereoPorts Create(const audio::PortDesc& desc)
        {
            PortManager* pPortManager = Session::Get()->GetPortManager();

            StereoPorts result;
            result.Left = pPortManager->NewAudioPort(desc);
            result.Right = pPortManager->NewAudioPort(desc);
            return result;
        }
    };
} // namespace quinte
