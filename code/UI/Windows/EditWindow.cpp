#include <Audio/Session.hpp>
#include <Audio/Transport.hpp>
#include <Core/Memory/TempAllocator.hpp>
#include <UI/Widgets/Common.hpp>
#include <UI/Windows/EditWindow.hpp>

namespace quinte
{
    EditWindow::EditWindow()
        : BaseWindow("EditWindow")
    {
    }


    void EditWindow::DrawTrackLane(TrackInfo& trackInfo)
    {
        using namespace ImGui;

        Transport* pTransport = Interface<Transport>::Get();

        const float frameHeight = GetFrameHeight();
        const float height = GetFrameHeightWithSpacing() * 3.0f;
        const float width = GetContentRegionAvail().x;

        const ImVec2 pos = GetCursorScreenPos();
        ImGuiStyle& style = GetStyle();
        ImDrawList* pDrawList = GetWindowDrawList();
        InvisibleButton(FixFmt32{ "##TrackLane_{}", trackInfo.ID }.Data(), { width, height });

        for (AudioClip& clip : trackInfo.pTrack->GetPlaylist())
        {
            const int64_t clipPos = static_cast<int64_t>(clip.GetPosition().GetSampleIndex()) - m_TimelineStart;
            const int64_t clipEndPos = static_cast<int64_t>(clip.GetEndPosition().GetSampleIndex()) - m_TimelineStart;

            if (clipEndPos < 0)
                continue;

            const ImVec2 rectMin{ static_cast<float>(pos.x + clipPos / m_SamplesPerPixel), pos.y + 4.0f };
            const ImVec2 rectMax{ static_cast<float>(pos.x + clipEndPos / m_SamplesPerPixel), pos.y + height - 4.0f };
            pDrawList->AddRectFilled(rectMin, rectMax, trackInfo.Color, 4.0f);

            const ImVec2 clipRectMax{ static_cast<float>(pos.x + clipEndPos / m_SamplesPerPixel), pos.y + 4.0f + frameHeight };
            pDrawList->PushClipRect(rectMin, clipRectMax, true);
            pDrawList->AddRectFilled(rectMin, rectMax, colors::Dim(trackInfo.Color, 0.7f), 4.0f);
            pDrawList->AddText(rectMin + style.FramePadding, colors::kWhite, "Test clip");
            pDrawList->PopClipRect();

            pDrawList->AddRect(rectMin, rectMax, colors::kWhite, 4.0f, 0, 2.0f);
        }

        const audio::TimePos64 playhead = pTransport->GetPlayhead();
        float linePos = static_cast<float>(pos.x + playhead.GetSampleIndex() / m_SamplesPerPixel);
        pDrawList->AddLine({ linePos, pos.y }, { linePos, pos.y + height }, colors::kWhite, 2.0f);
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
                DrawTrackLane(tracks[trackIndex]);
            }
        }
        EndChild();

        End();
    }
} // namespace quinte
