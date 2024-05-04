#include <Application/Application.hpp>
#include <UI/Icons.hpp>
#include <UI/Widgets/Track.hpp>
#include <numbers>

namespace quinte
{
    [[maybe_unused]] static audio::CallbackResult AudioCallbackDist(void* pOutputBuffer, void* pInputBuffer, uint32_t frameCount,
                                                                    double streamTime, audio::StreamStatus status, void*)
    {
        QU_Unused(streamTime);
        QU_Unused(status);

        const uint32_t channelCount = 2;
        float* outBuffer = static_cast<float*>(pOutputBuffer);
        const float* inBuffer = static_cast<float*>(pInputBuffer);
        const float drive = 300.0f;

        for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex)
        {
            float sample = 0.0f;
            for (uint32_t channelIndex = 0; channelIndex < channelCount; ++channelIndex)
            {
                const float* inChannel = inBuffer + channelIndex * frameCount;
                sample += inChannel[frameIndex] / channelCount;
            }

            for (uint32_t channelIndex = 0; channelIndex < channelCount; ++channelIndex)
            {
                float* outChannel = outBuffer + channelIndex * frameCount;
                outChannel[frameIndex] = (2.0f / std::numbers::pi_v<float>)*atan(drive * sample);
            }
        }

        return audio::CallbackResult::OK;
    }


    Application::Application()
        : VulkanApplication("Quinte")
    {
        Session::LoadEmpty();
        Session::Get()->GetAudioEngine()->InitializeAPI(audio::APIKind::WASAPI);
    }


    Application::~Application()
    {
        Session::Unload();
    }


    void Application::DrawUI()
    {
        using namespace ImGui;

        static bool s_ShowDemo = false;
        if (BeginMainMenuBar())
        {
            if (BeginMenu("File"))
            {
                MenuItem("Save", "Ctrl+S");
                MenuItem("Open", "Ctrl+O");
                if (MenuItem("Show Demo"))
                    s_ShowDemo = true;
                EndMenu();
            }

            EndMainMenuBar();
        }

        ImGuiViewport* pViewport = GetMainViewport();
        DockSpaceOverViewport(pViewport);

        if (s_ShowDemo)
            ShowDemoWindow();

        if (Begin("Audio Device"))
        {
            TextUnformatted("Test " NF_FA_ARROW_RIGHT " " NF_COD_ACCOUNT " " NF_FA_MEMORY);
            TextUnformatted("API Kind: WASAPI");

            const IAudioAPI* pAPI = Session::Get()->GetAudioEngine()->GetAPI();
            const auto& devices = pAPI->GetDevices();

            const audio::DeviceDesc* pSelectedInDevice =
                m_SelectedInputDeviceIndex >= devices.size() ? nullptr : &devices[m_SelectedInputDeviceIndex];
            if (BeginCombo("Input Device", pSelectedInDevice ? pSelectedInDevice->Name.Data() : "Select Input Device"))
            {
                for (uint32_t deviceIndex = 0; deviceIndex < devices.size(); ++deviceIndex)
                {
                    const audio::DeviceDesc& device = devices[deviceIndex];
                    if (device.InputChannelCount == 0)
                        continue;

                    if (Selectable(device.Name.Data(), m_SelectedInputDeviceIndex == deviceIndex))
                    {
                        m_SelectedInputDeviceIndex = deviceIndex;
                    }

                    if (IsItemHovered(ImGuiHoveredFlags_ForTooltip))
                    {
                        BeginTooltip();
                        Text("ID: %d", device.ID.Value);
                        Text("Name: %s", device.Name.Data());
                        Text("OutputChannelCount: %d", device.OutputChannelCount);
                        Text("InputChannelCount: %d", device.InputChannelCount);
                        Text("DuplexChannelCount: %d", device.DuplexChannelCount);
                        Text("CurrentSampleRate: %d", device.CurrentSampleRate);
                        Text("PreferredSampleRate: %d", device.PreferredSampleRate);
                        EndTooltip();
                    }
                }

                EndCombo();
            }

            const audio::DeviceDesc* pSelectedOutDevice =
                m_SelectedInputDeviceIndex >= devices.size() ? nullptr : &devices[m_SelectedOutputDeviceIndex];
            if (BeginCombo("Output Device", pSelectedOutDevice ? pSelectedOutDevice->Name.Data() : "Select Output Device"))
            {
                for (uint32_t deviceIndex = 0; deviceIndex < devices.size(); ++deviceIndex)
                {
                    const audio::DeviceDesc& device = devices[deviceIndex];
                    if (device.OutputChannelCount == 0)
                        continue;

                    if (Selectable(device.Name.Data(), m_SelectedOutputDeviceIndex == deviceIndex))
                    {
                        m_SelectedOutputDeviceIndex = deviceIndex;
                    }

                    if (IsItemHovered(ImGuiHoveredFlags_ForTooltip))
                    {
                        BeginTooltip();
                        Text("ID: %d", device.ID.Value);
                        Text("Name: %s", device.Name.Data());
                        Text("OutputChannelCount: %d", device.OutputChannelCount);
                        Text("InputChannelCount: %d", device.InputChannelCount);
                        Text("DuplexChannelCount: %d", device.DuplexChannelCount);
                        Text("CurrentSampleRate: %d", device.CurrentSampleRate);
                        Text("PreferredSampleRate: %d", device.PreferredSampleRate);
                        EndTooltip();
                    }
                }

                EndCombo();
            }

            const StringSlice streamStateString = audio::ToString(pAPI->GetState());
            Text("Current stream state: %.*s", static_cast<int32_t>(streamStateString.Size()), streamStateString.Data());

            if (Button("Open Stream"))
            {
                if (pAPI->GetState() != audio::StreamState::Running)
                {
                    const audio::EngineStartInfo startInfo{
                        .InputDevice = pSelectedInDevice->ID,
                        .OutputDevice = pSelectedOutDevice->ID,
                        .BufferSize = 512,
                    };
                    Session::Get()->GetAudioEngine()->Start(startInfo);
                }
                else
                {
                    Session::Get()->GetAudioEngine()->Stop();
                }
            }

            static TrackMixerView s_TrackMixerView;
            s_TrackMixerView.Color = colors::kDarkRed;
            s_TrackMixerView.Volume = s_TrackMixerView.FaderAmplitude;
            s_TrackMixerView.Name = "Test Track";
            s_TrackMixerView.ID = 0;
            s_TrackMixerView.Draw();

            SameLine();
            static TrackMixerView s_TrackMixerView1;
            s_TrackMixerView1.Color = colors::kDarkGreen;
            s_TrackMixerView1.Volume = s_TrackMixerView1.FaderAmplitude;
            s_TrackMixerView1.Name = "Test 2";
            s_TrackMixerView1.ID = 1;
            s_TrackMixerView1.Draw();

            SameLine();
            static TrackMixerView s_TrackMixerView2;
            s_TrackMixerView2.Color = colors::kRebeccaPurple;
            s_TrackMixerView2.Volume = s_TrackMixerView2.FaderAmplitude;
            s_TrackMixerView2.ID = 2;
            s_TrackMixerView2.Draw();
        }

        End();
    }
} // namespace quinte
