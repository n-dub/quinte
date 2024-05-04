#pragma once
#include <Core/Core.hpp>

namespace quinte
{
    class PortManager;
    class AudioEngine;


    class Session final
    {
        memory::unique_ptr<AudioEngine> m_pAudioEngine;
        memory::unique_ptr<PortManager> m_pPortManager;

    public:
        Session();
        ~Session();

        void OnStreamStarted();

        AudioEngine* GetAudioEngine() const;
        PortManager* GetPortManager() const;

        static Session* LoadEmpty();
        static void Unload();

        static Session* Get();
    };
} // namespace quinte
