#include <Core/FixedString.hpp>
#include <UI/Icons.hpp>
#include <UI/Widgets/Common.hpp>
#include <UI/Widgets/Tracks/TrackMixerView.hpp>
#include <UI/Widgets/Tracks/TracksCommon.hpp>

namespace quinte
{
    bool TrackMixerView::Draw()
    {
        using namespace ImGui;

        bool valuesChanged = false;

        Fader* pFader = pTrack->GetFader();

        MaxVolume = Max(MaxVolume, pFader->GetGain().GetAmplitude());

        const ImGuiStyle& style = GetStyle();
        const float width = 80.0f + style.ItemInnerSpacing.x * 2.0f;
        if (BeginChild(FixFmt32{ "TrackMixer_{}", ID }.Data(),
                       ImVec2{ width, 0 },
                       ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY))
        {
            SetCursorPosX(20.0f + style.ItemInnerSpacing.x);
            float pan = pFader->GetPan().Value;
            if (ui::Knob("##PanKnob", &pan, "L", "R"))
            {
                valuesChanged = true;
                pFader->SetPan(audio::PanValue{ pan });
            }

            if (IsItemActive() || IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            {
                BeginTooltip();
                const audio::PanValue::Pos panPos = pFader->GetPan().GetPos();
                if (panPos == audio::PanValue::Pos::Center)
                    TextDisabled("pan: center");
                else if (panPos == audio::PanValue::Pos::Left)
                    TextDisabled("pan: left %d%%", pFader->GetPan().GetLeftPercent());
                else if (panPos == audio::PanValue::Pos::Right)
                    TextDisabled("pan: right %d%%", pFader->GetPan().GetRightPercent());

                EndTooltip();
            }

            const float labelWidth = 0.5f * width - 2.0f * style.ItemInnerSpacing.x - 2.0f;
            const float labelX = style.ItemInnerSpacing.x + 2.0f;

            SetCursorPos(ImVec2{ labelX, GetCursorPosY() + 8.0f });

            {
                const FontScope fontScope{ ui::FontKind::Tiny };
                const ColorScope colorBtn{ ImGuiCol_Button, GetColorU32(ImGuiCol_FrameBg) };
                const ColorScope colorHover{ ImGuiCol_ButtonHovered, GetColorU32(ImGuiCol_FrameBg) };
                const ColorScope colorActive{ ImGuiCol_ButtonActive, GetColorU32(ImGuiCol_FrameBgHovered) };

                if (Button(FixFmt32{ "{:.1f}", pFader->GetGain().GetDBFS() }.Data(), ImVec2{ labelWidth, 0.0f }))
                {
                    pFader->SetGain(audio::GainValue{ 1.0f });
                }

                SameLine(0.0f, labelX);

                const ColorScope colorTxt{ ImGuiCol_Text, ui::GetAmplitudeValueColor(MaxVolume) };
                if (Button(FixFmt32{ "{:.1f}", audio::ConvertAmplitudeToDBFS(MaxVolume) }.Data(), ImVec2{ labelWidth, 0.0f }))
                    MaxVolume = pFader->GetGain().GetAmplitude();
            }

            // TODO: move fader view to its own class
            float faderAmplitude = pFader->GetGain().GetAmplitude();

            SetCursorPosY(GetCursorPosY() + 8.0f);
            if (ui::Fader("##Fader", &faderAmplitude, faderAmplitude, 200.0f))
            {
                pFader->SetGain(audio::GainValue{ faderAmplitude });
                valuesChanged = true;
            }

            bool monitored = pTrack->IsMonitored();
            bool recordArmed = pTrack->IsRecordArmed();
            bool muted = pFader->IsMuted();
            bool soloed = pFader->IsSoloed();

            SetCursorPos(ImVec2{ labelX, GetCursorPosY() + 8.0f });
            if (ui::ToggleButton("I", &monitored, ImVec2{ labelWidth, 0.0f }, colors::kLightGray))
                pTrack->SetMonitor(monitored);
            SetItemTooltip(monitored ? "input monitoring on" : "input monitoring off");

            SameLine(0.0f, labelX);
            if (ui::ToggleButton(NF_MD_RECORD, &recordArmed, ImVec2{ labelWidth, 0.0f }, colors::kDarkRed))
                pTrack->SetRecordArm(recordArmed);
            SetItemTooltip(recordArmed ? "armed for recording" : "not armed for recording");

            SetCursorPos(ImVec2{ labelX, GetCursorPosY() + 8.0f });
            if (ui::ToggleButton("M", &muted, ImVec2{ labelWidth, 0.0f }, colors::kRed))
                pFader->SetMute(muted);
            SetItemTooltip(muted ? "muted" : "not muted");

            SameLine(0.0f, labelX);
            if (ui::ToggleButton("S", &soloed, ImVec2{ labelWidth, 0.0f }, colors::kGold))
                pFader->SetSolo(soloed);
            SetItemTooltip(soloed ? "soloed" : "not soloed");

            {
                const ColorScope colorBtn{ ImGuiCol_Button, Color };
                const ColorScope colorHover{ ImGuiCol_ButtonHovered, Color };
                const ColorScope colorActive{ ImGuiCol_ButtonActive, Color };
                const StyleScope borderStyle{ ImGuiStyleVar_FrameBorderSize, 1.0f };

                SetCursorPosX(4.0f);

                const StringSlice trackName = pTrack->GetName();
                const FixFmt32 defaultID{ "##Track_{}", ID };
                Button(trackName.Empty() ? defaultID.Data() : trackName.Data(),
                       ImVec2{ width - 8.0f, GetTextLineHeight() * 3.0f });
            }
        }

        EndChild();

        return valuesChanged;
    }
} // namespace quinte
