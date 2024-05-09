#include <Audio/Session.hpp>
#include <Core/Memory/TempAllocator.hpp>
#include <UI/Widgets/Common.hpp>
#include <UI/Windows/EditWindow.hpp>

namespace quinte
{
    EditWindow::EditWindow()
        : BaseWindow("EditWindow")
    {
    }


    void EditWindow::DrawUI()
    {
        using namespace ImGui;

        Begin(m_Name.Data(), nullptr, ImGuiWindowFlags_NoNavFocus);

        float size2 = GetContentRegionAvail().x - m_LeftPanelSize;
        ui::Splitter(true, 5.0f, &m_LeftPanelSize, &size2, 150.0f, 200.0f);

        memory::TempAllocatorScope temp;

        Session* pSession = Interface<Session>::Get();
        TrackList& tracks = pSession->GetTrackList();

        std::pmr::vector<float> yOffsets{ &temp };
        if (BeginChild("##EditWindowLeftPanel", { m_LeftPanelSize, 0 }))
        {
            for (TrackInfo& trackInfo : tracks)
            {
                TrackEditView trackView{ .pTrack = trackInfo.pTrack, .ID = trackInfo.Color, .Color = trackInfo.Color };

                yOffsets.push_back(GetCursorPosY());
                trackView.Draw(m_LeftPanelSize - 5.0f);
            }
        }
        EndChild();

        SameLine();

        if (BeginChild("##EditWindowRightPanel", { size2, 0 }))
        {
            for (uint32_t trackIndex = 0; trackIndex < tracks.Size(); ++trackIndex)
            {
                SetCursorPosY(yOffsets[trackIndex]);
                Text("Track lane #%d", trackIndex);
            }
        }
        EndChild();

        End();
    }
} // namespace quinte
