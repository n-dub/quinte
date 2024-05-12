#include <Audio/Backend/WASAPI.hpp>
#include <Audio/Engine.hpp>
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Transport.hpp>
#include <Graph/ExecutionGraph.hpp>

namespace quinte
{
    audio::CallbackResult AudioEngine::AudioCallbackImpl(void* pOutputBuffer, void* pInputBuffer, uint32_t frameCount,
                                                         double streamTime, audio::StreamStatus status, void* pUserData)
    {
        return static_cast<AudioEngine*>(pUserData)->AudioCallback(pOutputBuffer, pInputBuffer, frameCount, streamTime, status);
    }


    audio::CallbackResult AudioEngine::AudioCallback(void* pOutputBuffer, void* pInputBuffer, uint32_t frameCount,
                                                     double streamTime, audio::StreamStatus status)
    {
        QU_Unused(streamTime);
        QU_Unused(status);

        if (!m_Running)
        {
            memory::Zero(static_cast<float*>(pOutputBuffer), frameCount * 2);
            return audio::CallbackResult::OK;
        }

        Transport* pTransport = Interface<Transport>::Get();
        PortManager* pPortManager = Interface<PortManager>::Get();
        pTransport->m_Playhead = pTransport->m_PlayheadRequest;
        pTransport->m_Recording = pTransport->m_RecordingRequested;

        audio::PlayState playState = pTransport->m_PlayState;
        while (playState == audio::PlayState::PauseRequested || playState == audio::PlayState::RollRequested)
        {
            if (playState == audio::PlayState::PauseRequested)
            {
                if (pTransport->m_PlayState.compare_exchange_weak(playState, audio::PlayState::Paused))
                    break;
            }
            if (playState == audio::PlayState::RollRequested)
            {
                if (pTransport->m_PlayState.compare_exchange_weak(playState, audio::PlayState::Rolling))
                    break;
            }
        }

        QU_Defer
        {
            if (pTransport->IsActuallyRolling())
                pTransport->AddToPlayhead(frameCount);
        };

        const StereoPorts& monitorPorts = pPortManager->GetMonitorPorts();
        monitorPorts.Left->GetBufferView()->Clear();
        monitorPorts.Right->GetBufferView()->Clear();

        const audio::EngineProcessInfo processInfo{ .StartTime = pTransport->m_Playhead, .LocalRange = { 0, frameCount } };
        m_Graph->Run(processInfo);

        const std::span<const Rc<AudioPort>> hardwarePorts = pPortManager->GetHardwarePorts();
        for (uint32_t channelIndex = 0; channelIndex < hardwarePorts.size(); ++channelIndex)
        {
            AudioBufferView* pHardwareInput = hardwarePorts[channelIndex]->GetBufferView();
            pHardwareInput->Read(static_cast<const float*>(pInputBuffer) + frameCount * channelIndex, 0, frameCount);
        }

        AudioBufferView* pML = monitorPorts.Left->GetBufferView();
        AudioBufferView* pMR = monitorPorts.Right->GetBufferView();

        memory::Copy(static_cast<float*>(pOutputBuffer), pML->Data(), frameCount);
        memory::Copy(static_cast<float*>(pOutputBuffer) + frameCount, pMR->Data(), frameCount);

        return audio::CallbackResult::OK;
    }


    AudioEngine::AudioEngine() = default;
    AudioEngine::~AudioEngine() = default;


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


    audio::ResultCode AudioEngine::Start(const audio::EngineStartInfo& startInfo)
    {
        QU_AssertDebug(m_Impl->GetState() == audio::StreamState::Closed);

        audio::StreamDesc outputDesc{};
        outputDesc.DeviceID = startInfo.OutputDevice;
        outputDesc.ChannelCount = 2;

        const audio::DeviceDesc& inputDeviceDesc = m_Impl->GetDeviceDesc(startInfo.InputDevice);

        audio::StreamDesc inputDesc{};
        inputDesc.DeviceID = startInfo.InputDevice;
        inputDesc.ChannelCount = inputDeviceDesc.InputChannelCount;

        audio::StreamOpenInfo openInfo{};
        openInfo.pOutputDesc = &outputDesc;
        openInfo.pInputDesc = &inputDesc;
        openInfo.Format = audio::Format::Float32;
        openInfo.BufferFrameCount = startInfo.BufferSize;
        openInfo.Callback = &AudioCallbackImpl;
        openInfo.pUserData = this;

        const audio::ResultCode openResult = m_Impl->OpenStream(openInfo);
        if (audio::Failed(openResult))
            return openResult;

        const audio::ResultCode startResult = m_Impl->StartStream();
        if (audio::Failed(startResult))
            return startResult;

        EventBus<AudioEngineEvents>::SendEvent(&AudioEngineEvents::OnAudioStreamStarted);

        m_Graph = memory::make_unique<ExecutionGraph>();
        m_Graph->Build();

        m_Running.store(true);

        return startResult;
    }


    void AudioEngine::Stop()
    {
        if (m_Impl->GetState() == audio::StreamState::Closed)
            return;

        m_Running.store(false);
        m_Impl->CloseStream();
        m_Graph.reset();
        EventBus<AudioEngineEvents>::SendEvent(&AudioEngineEvents::OnAudioStreamStopped);
    }
} // namespace quinte
