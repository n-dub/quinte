#include <UI/Widgets/Common.hpp>
#include <imgui_internal.h>
#include <numbers>
#include <numeric>

namespace quinte::ui
{
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


    bool Splitter(bool splitVertically, float thickness, float* size1, float* size2, float minSize1, float minSize2,
                  float splitterLongAxisSize)
    {
        using namespace ImGui;
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImGuiID id = window->GetID("##Splitter");
        ImRect bb;
        bb.Min = window->DC.CursorPos + (splitVertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
        bb.Max = bb.Min
            + CalcItemSize(splitVertically ? ImVec2(thickness, splitterLongAxisSize) : ImVec2(splitterLongAxisSize, thickness),
                           0.0f,
                           0.0f);
        return SplitterBehavior(bb, id, splitVertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, minSize1, minSize2, 0.0f);
    }
} // namespace quinte::ui
