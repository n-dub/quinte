#include <UI/Icons.hpp>
#include <UI/Widgets/Track.hpp>
#include <numbers>
#include <numeric>

#include <imgui_internal.h>

namespace quinte
{
    namespace ui
    {
        enum AmplitudeValueColorIndex
        {
            kAmplitudeCol_Normal,
            kAmplitudeCol_Medium,
            kAmplitudeCol_High,
        };


        inline static uint32_t GetAmplitudeValueColorByIndex(uint32_t index)
        {
            if (index == kAmplitudeCol_High)
                return colors::CreateU32(0xff, 0x30, 0x30, 0xff);
            if (index == kAmplitudeCol_Medium)
                return colors::kYellow;
            return ImGui::GetColorU32(ImGuiCol_CheckMark);
        }


        inline static uint32_t GetAmplitudeValueColor(float value)
        {
            const float dbfs = audio::ConvertAmplitudeToDBFS(value);
            if (dbfs >= -3.0f)
                return GetAmplitudeValueColorByIndex(kAmplitudeCol_High);
            if (dbfs >= -6.0f)
                return GetAmplitudeValueColorByIndex(kAmplitudeCol_Medium);
            return GetAmplitudeValueColorByIndex(kAmplitudeCol_Normal);
        }


        bool Knob(const char* strId, float* pValue, const char* leftLabel, const char* rightLabel, float minValue, float maxValue)
        {
            using namespace ImGui;

            QU_Unused(leftLabel);
            QU_Unused(rightLabel);

            constexpr float kMinAngle = std::numbers::pi_v<float> * 2.0f / 3.0f;
            constexpr float kMaxAngle = std::numbers::pi_v<float> * 7.0f / 3.0f;
            constexpr float kMidAngle = std::numbers::pi_v<float> * 1.50f;

            constexpr float kOuterRadius = 20.0f;
            constexpr float kInnerRadius = kOuterRadius * 0.75f;

            const float dpiScale = GetWindowDpiScale();
            const float markLength = 6.0f * dpiScale;

            const ImGuiIO& io = GetIO();
            const ImGuiStyle& style = GetStyle();

            const ImVec2 pos = GetCursorScreenPos();
            InvisibleButton(strId, ImVec2{ kOuterRadius * 2, kOuterRadius * 2 + style.ItemInnerSpacing.y * 2 });

            const bool isActive = IsItemActive();

            bool valueChanged = false;
            if (isActive && io.MouseDelta.y != 0.0f)
            {
                const float step = (maxValue - minValue) / 200.0f;
                *pValue -= io.MouseDelta.y * step;
                *pValue = Clamp(*pValue, minValue, maxValue);
                if (std::abs(*pValue) <= step)
                    *pValue = 0.0f;

                valueChanged = true;
            }
            else if (IsMouseDoubleClicked(ImGuiMouseButton_Left) && IsItemHovered())
            {
                const float midValue = std::midpoint(minValue, maxValue);
                valueChanged = *pValue != midValue;
                *pValue = midValue;
            }

            const float value = *pValue;
            const float t = (value - minValue) / (maxValue - minValue);
            const float angle = kMinAngle + (kMaxAngle - kMinAngle) * t;

            const ImVec2 center{ pos.x + kOuterRadius, pos.y + kOuterRadius + style.ItemInnerSpacing.y };

            const uint32_t frameBg = GetColorU32(ImGuiCol_FrameBg);
            const uint32_t knobColor = GetColorU32(ImGuiCol_SliderGrab);
            const uint32_t knobColorActive = GetColorU32(ImGuiCol_SliderGrabActive);
            const uint32_t fillColor = GetColorU32(ImGuiCol_CheckMark);

            ImDrawList* pDrawList = GetWindowDrawList();
            pDrawList->PathArcToFast(center, kOuterRadius, 4, 9);
            pDrawList->PathLineTo(center);
            pDrawList->PathFillConvex(frameBg);

            pDrawList->PathLineTo(center);
            pDrawList->PathArcToFast(center, kOuterRadius, 9, 14);
            pDrawList->PathFillConvex(frameBg);

            if (value == 0.0f)
            {
                pDrawList->AddLine(ImVec2{ center.x, center.y - kOuterRadius + dpiScale }, center, fillColor, 2.0f);
            }
            else if (angle < kMidAngle)
            {
                pDrawList->PathArcTo(center, kOuterRadius - dpiScale, angle, kMidAngle);
                pDrawList->PathLineTo(center);
                pDrawList->PathFillConvex(fillColor);
            }
            else
            {
                pDrawList->PathLineTo(center);
                pDrawList->PathArcTo(center, kOuterRadius - dpiScale, kMidAngle, angle);
                pDrawList->PathFillConvex(fillColor);
            }

            const uint32_t innerColor = isActive ? knobColorActive : knobColor;
            pDrawList->AddCircleFilled(center, kInnerRadius, innerColor);

            const auto [angleSin, angleCos] = math::SinCos(angle);
            pDrawList->AddLine(
                ImVec2{ center.x + angleCos * (kInnerRadius - markLength), center.y + angleSin * (kInnerRadius - markLength) },
                ImVec2{ center.x + angleCos * kInnerRadius, center.y + angleSin * kInnerRadius },
                frameBg,
                2.0f);

            const FontScope font{ FontKind::Tiny };

            const float textY = center.y + kInnerRadius + dpiScale * 2.0f;
            const float rightOffset = CalcTextSize(rightLabel).x;
            pDrawList->AddText(ImVec2{ center.x - kInnerRadius, textY }, GetColorU32(ImGuiCol_Text), leftLabel);
            pDrawList->AddText(ImVec2{ center.x + kInnerRadius - rightOffset, textY }, GetColorU32(ImGuiCol_Text), rightLabel);

            return valueChanged;
        }


        bool Fader(const char* strId, float* pValue, float meterValue, float height)
        {
            using namespace ImGui;

            constexpr float kGrabHeight = 50.0f;
            constexpr float kBorderHeight = kGrabHeight / 8.0f;
            constexpr float kWidth = 22.0f;

            constexpr float kMainLineThickness = 4.0f;
            constexpr uint32_t kThinLineCount = 3;
            constexpr float kThinLineStep = (0.5f * kGrabHeight - kBorderHeight) / (kThinLineCount + 1);

            const ImGuiIO& io = GetIO();
            const ImGuiStyle& style = GetStyle();
            const ImVec2 innerSpacing = style.ItemInnerSpacing;

            const float grabMinPos = kGrabHeight * 0.5f;
            const float grabMaxPos = height - grabMinPos;

            const float valueClamped = Clamp(*pValue, 0.0f, 2.0f);
            const float grabPos = Lerp(grabMinPos, grabMaxPos, 1.0f - audio::ConvertAmplitudeToFader(valueClamped));

            const ImVec2 cursorPos = GetCursorScreenPos();
            const ImVec2 pos{ cursorPos.x + 4.0f, cursorPos.y };

            const ImVec2 grabRectMin{ pos.x, pos.y + grabPos - kGrabHeight * 0.5f };

            SetCursorScreenPos(grabRectMin);
            InvisibleButton(strId, ImVec2{ kWidth, kGrabHeight });

            const bool isActive = IsItemActive();

            bool valueChanged = false;
            if (isActive && io.MouseDelta.y != 0.0f)
            {
                const float grabNewPos = grabPos + io.MouseDelta.y;
                *pValue = audio::ConvertFaderToAmplitude(1.0f - (grabNewPos - grabMinPos) / (grabMaxPos - grabMinPos));
                *pValue = Clamp(*pValue, 0.0f, 2.0f);

                valueChanged = true;
            }
            else if (IsMouseDoubleClicked(ImGuiMouseButton_Left) && IsItemHovered())
            {
                valueChanged = *pValue != 1.0f;
                *pValue = 1.0f;
            }

            SetCursorScreenPos(ImVec2{ pos.x, pos.y + innerSpacing.y * 2.0f + height });
            Dummy({ 0, 0 });

            const float centerX = pos.x + kWidth * 0.5f;

            const uint32_t frameBg = GetColorU32(ImGuiCol_FrameBg);
            const uint32_t brightColor = GetColorU32(ImGuiCol_SliderGrab);
            const uint32_t dimmedColor = colors::Dim(brightColor, 0.8f);

            const ImVec2 scaleTop{ centerX, pos.y + innerSpacing.y + kGrabHeight * 0.5f };
            const ImVec2 scaleBottom{ centerX, pos.y + height - innerSpacing.y - kGrabHeight * 0.5f };

            ImDrawList* pDrawList = GetWindowDrawList();
            pDrawList->AddLine(scaleTop, scaleBottom, frameBg, kMainLineThickness);

            const ImVec2 scaleMin{ scaleTop.x - 16.0f, scaleTop.y };
            const ImVec2 scaleMax{ scaleBottom.x - kMainLineThickness * 0.5f, scaleBottom.y };
            DrawAmplitudeScale(pDrawList, scaleMin, scaleMax, 2.0f, false);

            DrawMeter(pDrawList,
                      ImVec2{ scaleTop.x + 40.0f, pos.y + innerSpacing.y },
                      ImVec2{ scaleTop.x + 55.0f, pos.y + height - innerSpacing.y },
                      meterValue);

            const ImVec2 topRectMax{ grabRectMin.x + kWidth, grabRectMin.y + kBorderHeight };
            pDrawList->AddRectFilled(grabRectMin, topRectMax, brightColor);

            const int32_t startVertIndex = pDrawList->VtxBuffer.size();

            const ImVec2 midRectMin{ grabRectMin.x, topRectMax.y };
            const ImVec2 midRectMax{ grabRectMin.x + kWidth, grabRectMin.y + kGrabHeight - kBorderHeight };
            pDrawList->AddRectFilled(midRectMin, midRectMax, brightColor);

            const ImVec2 bottomRectMin{ grabRectMin.x, midRectMax.y };
            ShadeVertsLinearColorGradientKeepAlpha(
                pDrawList, startVertIndex, pDrawList->VtxBuffer.size(), midRectMin, bottomRectMin, dimmedColor, brightColor);

            const ImVec2 bottomRectMax{ grabRectMin.x + kWidth, grabRectMin.y + kGrabHeight };
            pDrawList->AddRectFilled(bottomRectMin, bottomRectMax, dimmedColor);

            pDrawList->AddLine(ImVec2{ grabRectMin.x, grabRectMin.y + kGrabHeight * 0.5f },
                               ImVec2{ grabRectMin.x + kWidth, grabRectMin.y + kGrabHeight * 0.5f },
                               frameBg,
                               2.0f);

            const uint32_t thinLineColor = colors::Dim(brightColor, 0.7f);
            for (uint32_t i = 0; i < kThinLineCount; ++i)
            {
                const float offset1 = topRectMax.y + kThinLineStep * (i + 1);
                const float offset2 = bottomRectMin.y - kThinLineStep * (i + 1);
                pDrawList->AddLine(ImVec2{ grabRectMin.x, offset1 }, ImVec2{ grabRectMin.x + kWidth, offset1 }, thinLineColor);
                pDrawList->AddLine(ImVec2{ grabRectMin.x, offset2 }, ImVec2{ grabRectMin.x + kWidth, offset2 }, thinLineColor);
            }

            return valueChanged;
        }


        bool ToggleButton(const char* label, bool* pToggled, const ImVec2& size, uint32_t color)
        {
            using namespace ImGui;

            if (color == 0)
                color = GetColorU32(ImGuiCol_Tab);

            const StyleScope borderStyle{ ImGuiStyleVar_FrameBorderSize, 1.0f };
            const ColorScope colorBtn{ ImGuiCol_Button, colors::Dim(color, 0.8f), *pToggled };
            const ColorScope colorHover{ ImGuiCol_ButtonHovered, colors::Dim(color, 0.9f), *pToggled };
            const ColorScope colorActive{ ImGuiCol_ButtonActive, color, *pToggled };

            if (Button(label, size))
            {
                *pToggled = !*pToggled;
                return true;
            }

            return false;
        }


        void DrawAmplitudeScale(ImDrawList* pDrawList, const ImVec2& min, const ImVec2& max, float maxValue, bool drawLabels)
        {
            const float maxFader = audio::ConvertAmplitudeToFader(maxValue);
            const float pixelStep = ImGui::GetTextLineHeight();

            float prevHeight = min.y - pixelStep - 5.0f;
            const auto drawLine = [&](float dbfs, bool fullLength, uint32_t color) {
                const float amplitude = audio::ConvertDBFSToAmplitude(dbfs);
                if (amplitude > maxValue)
                    return true;

                const float height = min.y + (max.y - min.y) * (maxFader - audio::ConvertAmplitudeToFader(amplitude)) / maxFader;

                if (height - prevHeight < pixelStep && fullLength)
                    return false;

                if (fullLength)
                    prevHeight = height;

                const float width = fullLength ? (max.x - min.x) : Max(2.0f, (max.x - min.x) * 0.3f);
                pDrawList->AddLine(ImVec2{ max.x - width, height }, ImVec2{ max.x, height }, color, 1.0f);

                if (drawLabels && fullLength)
                {
                    const auto labelText = std::isinf(dbfs) ? NF_FA_INFINITY : FixFmt32{ "{}", dbfs }.Get();
                    ImVec2 textSize = ImGui::CalcTextSize(labelText.Data(), labelText.Data() + labelText.Size());
                    if (std::isinf(dbfs))
                        textSize.x *= 2.0f;

                    pDrawList->AddText(ImVec2{ min.x - textSize.x - 2.0f, height - textSize.y * 0.5f },
                                       color,
                                       labelText.Data(),
                                       labelText.Data() + labelText.Size());
                }

                return true;
            };


            // FIXME: this is so bad in so many ways...
            uint32_t dec = 1;
            uint32_t fullLengthInterval = 3;
            drawLine(0, true, ImGui::GetColorU32(ImGuiCol_Text));
            for (int32_t dbfs = 6; dbfs >= -50; dbfs -= dec)
            {
                if (dbfs == 0)
                    continue;

                if (!drawLine(static_cast<float>(dbfs), dbfs % fullLengthInterval == 0, ImGui::GetColorU32(ImGuiCol_TextDisabled))
                    && dbfs < -10)
                {
                    dec = 3;
                    fullLengthInterval = 5;
                }
            }

            if (drawLabels)
                drawLine(-std::numeric_limits<float>::infinity(), true, ImGui::GetColorU32(ImGuiCol_Text));
        }


        void DrawMeter(ImDrawList* pDrawList, const ImVec2& min, const ImVec2& max, float value)
        {
            using namespace ImGui;

            pDrawList->AddRectFilled(min, max, GetColorU32(ImGuiCol_FrameBg));

            const ImGuiStyle& style = GetStyle();
            const ImVec2 scaleMin{ min.x - 5.0f, min.y + style.ItemInnerSpacing.y };
            const ImVec2 scaleMax{ min.x, max.y - style.ItemInnerSpacing.y };
            DrawAmplitudeScale(pDrawList, scaleMin, scaleMax, 1.0f, true);

            if (ApproxEqual(value, 0.0f))
                return;

            constexpr float kMaxFader = audio::ConvertAmplitudeToFader(1.0f);

            const float dbfs = audio::ConvertAmplitudeToDBFS(value);
            const float fader = audio::ConvertAmplitudeToFader(value);

            const float workMinY = min.y + style.ItemInnerSpacing.y;
            const float workMaxY = max.y - style.ItemInnerSpacing.y;
            const float workRange = workMaxY - workMinY;
            const float height = workMinY + (kMaxFader - fader) * workRange / kMaxFader;

            constexpr float kFaderNormal = audio::ConvertDBFSToFader(-12.0f);
            constexpr float kFaderMidNormal = audio::ConvertDBFSToFader(-6.0f);
            constexpr float kFaderMedium = audio::ConvertDBFSToFader(-3.0f);

            const float heightNormal = workMinY + (kMaxFader - kFaderNormal) * workRange / kMaxFader;
            const float heightMidNormal = workMinY + (kMaxFader - kFaderMidNormal) * workRange / kMaxFader;
            const float heightMedium = workMinY + (kMaxFader - kFaderMedium) * workRange / kMaxFader;

            ImVec2 meterMin{ min.x + style.ItemInnerSpacing.x, 0.0f };
            ImVec2 meterMax{ max.x - style.ItemInnerSpacing.x, 0.0f };

            const uint32_t colorNormal = colors::Dim(ui::GetAmplitudeValueColorByIndex(kAmplitudeCol_Normal), 0.8f);
            const uint32_t colorMedium = colors::Dim(ui::GetAmplitudeValueColorByIndex(kAmplitudeCol_Medium), 0.8f);
            const uint32_t colorHigh = colors::Dim(ui::GetAmplitudeValueColorByIndex(kAmplitudeCol_High), 0.8f);

            meterMax.y = workMaxY;
            meterMin.y = dbfs > -12.0f ? heightNormal : height;
            pDrawList->AddRectFilled(meterMin, meterMax, colorNormal);
            if (dbfs <= -12.0f)
                return;

            meterMax.y = meterMin.y;
            meterMin.y = dbfs > -6.0f ? heightMidNormal : height;
            pDrawList->AddRectFilledMultiColor(meterMin, meterMax, colorMedium, colorMedium, colorNormal, colorNormal);
            if (dbfs <= -6.0f)
                return;

            meterMax.y = meterMin.y;
            meterMin.y = dbfs > -3.0f ? heightMedium : height;
            pDrawList->AddRectFilled(meterMin, meterMax, colorMedium);
            if (dbfs <= -3.0f)
                return;

            meterMax.y = meterMin.y;
            meterMin.y = Max(height, workMinY);
            pDrawList->AddRectFilledMultiColor(meterMin, meterMax, colorHigh, colorHigh, colorMedium, colorMedium);
        }
    } // namespace ui


    bool TrackMixerView::Draw()
    {
        using namespace ImGui;

        bool valuesChanged = false;

        MaxVolume = Max(MaxVolume, Volume);

        const ImGuiStyle& style = GetStyle();
        const float width = 80.0f + style.ItemInnerSpacing.x * 2.0f;
        if (BeginChild(FixFmt32{ "TrackMixer_{}", ID }.Data(),
                       ImVec2{ width, 0 },
                       ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY))
        {
            SetCursorPosX(20.0f + style.ItemInnerSpacing.x);
            if (ui::Knob("##PanKnob", &Pan, "L", "R"))
                valuesChanged = true;

            if (IsItemActive() || IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            {
                BeginTooltip();
                if (Pan == 0.0f)
                    TextDisabled("center");
                else if (Pan < 0.0f)
                    TextDisabled("left %d%%", static_cast<uint32_t>(Pan * -100.0f));
                else if (Pan > 0.0f)
                    TextDisabled("right %d%%", static_cast<uint32_t>(Pan * 100.0f));

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

                if (Button(FixFmt32{ "{:.1f}", audio::ConvertAmplitudeToDBFS(FaderAmplitude) }.Data(),
                           ImVec2{ labelWidth, 0.0f }))
                    FaderAmplitude = 1.0f;

                SameLine(0.0f, labelX);

                const ColorScope colorTxt{ ImGuiCol_Text, ui::GetAmplitudeValueColor(MaxVolume) };
                if (Button(FixFmt32{ "{:.1f}", audio::ConvertAmplitudeToDBFS(MaxVolume) }.Data(), ImVec2{ labelWidth, 0.0f }))
                    MaxVolume = Volume;
            }

            SetCursorPosY(GetCursorPosY() + 8.0f);
            if (ui::Fader("##Fader", &FaderAmplitude, Volume, 200.0f))
                valuesChanged = true;

            SetCursorPos(ImVec2{ labelX, GetCursorPosY() + 8.0f });
            ui::ToggleButton("M", &Muted, ImVec2{ labelWidth, 0.0f }, colors::kRed);
            SetItemTooltip(Muted ? "muted" : "not muted");

            SameLine(0.0f, labelX);
            ui::ToggleButton("S", &Soloed, ImVec2{ labelWidth, 0.0f }, colors::kGold);
            SetItemTooltip(Soloed ? "soloed" : "not soloed");

            {
                const ColorScope colorBtn{ ImGuiCol_Button, Color };
                const ColorScope colorHover{ ImGuiCol_ButtonHovered, Color };
                const ColorScope colorActive{ ImGuiCol_ButtonActive, Color };
                const StyleScope borderStyle{ ImGuiStyleVar_FrameBorderSize, 1.0f };

                SetCursorPosX(4.0f);

                const FixFmt32 defaultID{ "##Track_{}", ID };
                Button(Name.Empty() ? defaultID.Data() : Name.Data(), ImVec2{ width - 8.0f, GetTextLineHeight() * 3.0f });
            }
        }

        EndChild();

        return valuesChanged;
    }
} // namespace quinte
