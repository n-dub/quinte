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
        memory::unique_ptr<PortManager> m_pPortManager;
        TrackList m_TrackList;

        void OnAudioStreamStarted() override;
        void OnAudioStreamStopped() override;

    public:
        Session();
        ~Session();

        inline TrackList& GetTrackList()
        {
            return m_TrackList;
        }
    };
} // namespace quinte
