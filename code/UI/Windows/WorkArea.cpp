#include <Application/Application.hpp>
#include <Audio/Tracks/Track.hpp>
#include <UI/Icons.hpp>
#include <UI/Widgets/Tracks/TrackMixerView.hpp>
#include <UI/Windows/EditWindow.hpp>
#include <UI/Windows/WorkArea.hpp>

#include <imgui_internal.h>

namespace quinte
{
    WorkArea::WorkArea()
    {
        m_pEditWindow = memory::make_unique<EditWindow>();
    }


    WorkArea::~WorkArea() = default;


    void WorkArea::Draw()
    {
        using namespace ImGui;

        const float barHeight = GetFrameHeight();
        const ImGuiWindowFlags kBarFlags =
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

        static bool s_ShowDemo = false;

        const auto* pApplication = Interface<Application>::Get();
        const int32_t rootWidth = pApplication->GetWidth();
        const int32_t rootHeight = pApplication->GetHeight();

        ImGuiViewport* viewport = GetMainViewport();
        if (BeginViewportSideBar("##MainMenuBar", viewport, ImGuiDir_Up, barHeight, kBarFlags))
        {
            if (BeginMenuBar())
            {
                if (BeginMenu("File"))
                {
                    MenuItem("Save", "Ctrl+S");
                    MenuItem("Open", "Ctrl+O");
                    if (MenuItem("Show Demo"))
                        s_ShowDemo = true;
                    EndMenu();
                }

                EndMenuBar();
            }
        }
        End();

        if (s_ShowDemo)
            ShowDemoWindow();

        {
            const ColorScope windowBgColor{ ImGuiCol_WindowBg, pApplication->GetBackgroundColor() };
            PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            PushStyleVar(ImGuiStyleVar_WindowPadding, { 4.0f, 4.0f });

            SetNextWindowPos({ viewport->Pos.x, viewport->Pos.y + barHeight });
            SetNextWindowSize(ImVec2(rootWidth, rootHeight - barHeight * 2));
            SetNextWindowViewport(viewport->ID);
            Begin("##RootWindow",
                  nullptr,
                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus
                      | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings
                      | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking
                      | ImGuiWindowFlags_NoNavFocus);

            PopStyleVar(3);
        }

        {
            Transport* pTransport = Interface<Transport>::Get();
            if (pTransport->IsActuallyRolling())
            {
                if (Button(NF_FA_PAUSE))
                    pTransport->RequestPause();
            }
            else
            {
                if (Button(NF_FA_PLAY))
                    pTransport->RequestRoll();
            }

            SameLine();
            if (Button(NF_MD_REWIND))
                pTransport->SetPlayhead(0);
        }

        const ImGuiID dockspaceID = GetID("MainDockspace");
        DockSpace(dockspaceID, { 0, 0 }, ImGuiDockNodeFlags_NoDockingInCentralNode);
        if (ImGuiDockNode* pCentralNode = DockBuilderGetCentralNode(dockspaceID))
        {
            pCentralNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
        }

        SetNextWindowDockID(dockspaceID);
        m_pEditWindow->DrawUI();

        End();

        if (BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, barHeight, kBarFlags))
        {
            if (BeginMenuBar())
            {
                Button("Test");
                SameLine();
                Text("Status bar");
                EndMenuBar();
            }
        }
        End();

        if (Begin("Audio Device"))
        {
            TextUnformatted("Test " NF_FA_ARROW_RIGHT " " NF_COD_ACCOUNT " " NF_FA_MEMORY);
            TextUnformatted("API Kind: WASAPI");

            const IAudioAPI* pAPI = Interface<AudioEngine>::Get()->GetAPI();
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
                    Interface<AudioEngine>::Get()->Start(startInfo);
                }
                else
                {
                    Interface<AudioEngine>::Get()->Stop();
                }
            }
        }
        End();

        if (Begin("Mix Window"))
        {
            Session* pSession = Interface<Session>::Get();
            TrackList& tracks = pSession->GetTrackList();

            for (TrackInfo& trackInfo : tracks)
            {
                TrackMixerView trackView{ .pTrack = trackInfo.pTrack, .ID = trackInfo.Color, .Color = trackInfo.Color };

                if (trackView.ID > 0)
                    SameLine();

                trackView.Draw();
            }
        }
        End();
    }
} // namespace quinte
