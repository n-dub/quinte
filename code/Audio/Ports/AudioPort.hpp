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

        ~AudioPort() override;

        inline AudioBufferView* GetBufferView() override
        {
            return &m_BufferView;
        }

        inline const AudioBufferView* GetBufferView() const override
        {
            return &m_BufferView;
        }

        void AllocateBuffer() override;
    };


    struct StereoPorts final
    {
        Rc<AudioPort> Left;
        Rc<AudioPort> Right;

        static StereoPorts Create(const audio::PortDesc& desc);
    };
} // namespace quinte
