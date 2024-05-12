#pragma once
#include <Audio/Base.hpp>
#include <Core/FixedVector.hpp>

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
            RecordingOnly = 1 << 2,

            All = StereoLeft | StereoRight,
        };

        QU_ENUM_BIT_OPERATORS(PortFlags);


        struct PortDesc final
        {
            PortKind Kind;
            DataDirection Direction;
            PortFlags Flags;
        };


        struct PortHandle : TypedHandle<PortHandle, uint32_t>
        {
        };
    } // namespace audio


    class BaseBufferView;
    class Track;


    class Port
    {
        friend class PortManager;

        SmallVector<audio::PortHandle, 2> m_Sources;
        SmallVector<audio::PortHandle, 2> m_Destinations;
        void* m_pOwner;
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

        uint32_t Release();

        [[nodiscard]] inline bool IsInput() const
        {
            return (m_Desc.Direction & audio::DataDirection::Input) == audio::DataDirection::Input;
        }

        [[nodiscard]] inline bool IsOutput() const
        {
            return (m_Desc.Direction & audio::DataDirection::Output) == audio::DataDirection::Output;
        }

        [[nodiscard]] inline audio::PortHandle GetHandle() const
        {
            return m_Handle;
        }

        [[nodiscard]] inline const audio::PortDesc& GetDesc() const
        {
            return m_Desc;
        }

        [[nodiscard]] inline audio::DataType GetDataType() const
        {
            return m_DataType;
        }

        [[nodiscard]] inline std::span<const audio::PortHandle> GetSources() const
        {
            return m_Sources;
        }

        [[nodiscard]] inline std::span<const audio::PortHandle> GetDestinations() const
        {
            return m_Destinations;
        }

        [[nodiscard]] inline Track* TryGetTrack() const
        {
            if (m_Desc.Kind == audio::PortKind::Track)
                return static_cast<Track*>(m_pOwner);
            return nullptr;
        }

        [[nodiscard]] virtual BaseBufferView* GetBufferView() = 0;
        [[nodiscard]] virtual const BaseBufferView* GetBufferView() const = 0;

        virtual void AllocateBuffer() = 0;
    };
} // namespace quinte
