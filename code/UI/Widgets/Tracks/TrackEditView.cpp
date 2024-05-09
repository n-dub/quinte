#include <Core/FixedString.hpp>
#include <UI/Icons.hpp>
#include <UI/Widgets/Common.hpp>
#include <UI/Widgets/Tracks/TrackEditView.hpp>
#include <UI/Widgets/Tracks/TracksCommon.hpp>

namespace quinte
{
    bool TrackEditView::Draw(float width)
    {
        using namespace ImGui;

        const IDScope idScope{ FixFmt32{ "##Track_", ID }.Data() };

        const float colorButtonWidth = GetFrameHeight();
        const float height = GetFrameHeightWithSpacing() * 3.0f;

        BeginGroup();
        ArrowButton("##CollapseArrow", ImGuiDir_Down);

        {
            const ColorScope normalColor{ ImGuiCol_Button, Color };
            const ColorScope hoverColor{ ImGuiCol_ButtonHovered, Color };
            const ColorScope activeColor{ ImGuiCol_ButtonActive, Color };
            Button("##ColorButton", { colorButtonWidth, height - GetFrameHeightWithSpacing() });
        }

        EndGroup();
        SameLine();
        BeginGroup();

        {
            const uint32_t trackNameColor = GetColorU32(ImGuiCol_ButtonHovered);
            const ColorScope normalColor{ ImGuiCol_Button, trackNameColor };
            const ColorScope hoverColor{ ImGuiCol_ButtonHovered, trackNameColor };
            const ColorScope activeColor{ ImGuiCol_ButtonActive, trackNameColor };

            const StringSlice trackName = pTrack->GetName();
            const FixFmt32 defaultID{ "##Track_{}", ID };
            Button(trackName.Empty() ? defaultID.Data() : trackName.Data(), { width - GetFrameHeightWithSpacing(), 0 });
        }

        Fader* pFader = pTrack->GetFader();

        bool monitored = pTrack->IsMonitored();
        bool recordArmed = pTrack->IsRecordArmed();
        bool muted = pFader->IsMuted();
        bool soloed = pFader->IsSoloed();

        const float sqButtonSize = GetTextLineHeightWithSpacing();
        if (ui::ToggleButton("I", &monitored, ImVec2{ sqButtonSize, sqButtonSize }, colors::kLightGray))
            pTrack->SetMonitor(monitored);
        SetItemTooltip(monitored ? "input monitoring on" : "input monitoring off");

        SameLine();
        if (ui::ToggleButton(NF_MD_RECORD, &recordArmed, ImVec2{ sqButtonSize, sqButtonSize }, colors::kDarkRed))
            pTrack->SetRecordArm(recordArmed);
        SetItemTooltip(recordArmed ? "armed for recording" : "not armed for recording");

        SameLine();
        if (ui::ToggleButton("M", &muted, ImVec2{ sqButtonSize, sqButtonSize }, colors::kRed))
            pFader->SetMute(muted);
        SetItemTooltip(muted ? "muted" : "not muted");

        SameLine();
        if (ui::ToggleButton("S", &soloed, ImVec2{ sqButtonSize, sqButtonSize }, colors::kGold))
            pFader->SetSolo(soloed);
        SetItemTooltip(soloed ? "soloed" : "not soloed");

        EndGroup();

        return false;
    }
} // namespace quinte
