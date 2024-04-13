#include <Application/Application.hpp>
#include <UI/Icons.hpp>
#include <UI/Widgets/Track.hpp>

namespace quinte
{
    static audio::CallbackResult AudioCallback(void* pOutputBuffer, void* pInputBuffer, uint32_t frameCount, double streamTime,
                                               audio::StreamStatus status, void* pUserData)
    {
        QU_Unused(pInputBuffer);
        QU_Unused(streamTime);

        const uint32_t channelCount = 2;
        float* buffer = static_cast<float*>(pOutputBuffer);
        float* lastValues = static_cast<float*>(pUserData);

        QU_Assert(status == audio::StreamStatus::OK);

        for (uint32_t frameIndex = 0; frameIndex < frameCount; frameIndex++)
        {
            for (uint32_t channelIndex = 0; channelIndex < channelCount; channelIndex++)
            {
                *buffer++ = static_cast<float>(lastValues[channelIndex] * 0.5f);
                lastValues[channelIndex] += 0.005f * (channelIndex + 1 + (channelIndex * 0.1f));
                if (lastValues[channelIndex] >= 1.0f)
                    lastValues[channelIndex] -= 2.0f;
            }
        }

        return audio::CallbackResult::OK;
    }


    Application::Application()
        : VulkanApplication("Quinte")
    {
        m_Engine.InitializeAPI(audio::APIKind::WASAPI);
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

            IAudioAPI* pAPI = m_Engine.GetAPI();
            const auto& devices = pAPI->GetDevices();

            const audio::DeviceDesc* pSelectedDevice =
                m_SelectedDeviceIndex >= devices.size() ? nullptr : &devices[m_SelectedDeviceIndex];
            if (BeginCombo("###DeviceCombo", pSelectedDevice ? pSelectedDevice->Name.Data() : "Select Device"))
            {
                for (uint32_t deviceIndex = 0; deviceIndex < devices.size(); ++deviceIndex)
                {
                    const audio::DeviceDesc& device = devices[deviceIndex];
                    if (Selectable(device.Name.Data(), m_SelectedDeviceIndex == deviceIndex))
                    {
                        m_SelectedDeviceIndex = deviceIndex;
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
            Text("Current stream state: %.*s", streamStateString.Size(), streamStateString.Data());

            if (Button("Open Stream"))
            {
                if (pAPI->GetState() != audio::StreamState::Running)
                {
                    audio::StreamDesc outputDesc{};
                    outputDesc.DeviceID = devices[m_SelectedDeviceIndex].ID;
                    outputDesc.ChannelCount = 2;

                    audio::StreamOpenInfo openInfo{};
                    openInfo.pOutputDesc = &outputDesc;
                    openInfo.Format = audio::Format::Float32;
                    openInfo.BufferFrameCount = 512;
                    openInfo.Callback = &AudioCallback;

                    static float s_UserData[2];
                    openInfo.pUserData = s_UserData;

                    const audio::ResultCode openResult = pAPI->OpenStream(openInfo);
                    QU_Assert(openResult == audio::ResultCode::Success);
                    const audio::ResultCode startResult = pAPI->StartStream();
                    QU_Assert(startResult == audio::ResultCode::Success);
                }
                else
                {
                    pAPI->CloseStream();
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
            s_TrackMixerView2.Color = colors::kDarkGreen;
            s_TrackMixerView2.Volume = s_TrackMixerView2.FaderAmplitude;
            s_TrackMixerView2.ID = 2;
            s_TrackMixerView2.Color = colors::kRebeccaPurple;
            s_TrackMixerView2.Draw();
        }

        End();
    }
} // namespace quinte
