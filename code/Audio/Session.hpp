#pragma once
#include <Audio/AudioEngineEvents.hpp>
#include <Audio/Tracks/TrackList.hpp>
#include <Core/Core.hpp>
#include <Core/EventBus.hpp>
#include <Core/Interface.hpp>

namespace quinte
{
    class PortManager;


    class Session final
        : public Interface<Session>::Registrar
        , public EventBus<AudioEngineEvents>::Handler
    {
        friend class ExecutionGraph;

        memory::unique_ptr<PortManager> m_pPortManager;

        StereoPorts m_MasterInputPorts;
        StereoPorts m_MasterOutputPorts;
        Rc<Track> m_pMasterTrack;

        TrackList m_TrackList;

        void OnAudioStreamStarted() override;
        void OnAudioStreamStopped() override;

    public:
        Session();
        ~Session();

        Track* CreateTrack(audio::DataType inputDataType = audio::DataType::Audio,
                           audio::DataType outputDataType = audio::DataType::Audio);

        inline TrackList& GetTrackList()
        {
            return m_TrackList;
        }
    };
} // namespace quinte
