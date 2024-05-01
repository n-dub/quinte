#include <Audio/Backend/WASAPI.hpp>
#include <Audio/Engine.hpp>

namespace quinte
{
    audio::ResultCode AudioEngine::InitializeAPI(audio::APIKind apiKind)
    {
        switch (apiKind)
        {
        case audio::APIKind::WASAPI:
            m_Impl = memory::make_unique<AudioBackendWASAPI>();
            return m_Impl->UpdateDeviceList();
        default:
            return audio::ResultCode::FailUnsupportedAPI;
        }
    }
} // namespace quinte
