#pragma once
#include <UI/Utils.hpp>

namespace quinte::ui
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


    bool Fader(const char* strId, float* pValue, float meterValue, float height);

    void DrawAmplitudeScale(ImDrawList* pDrawList, const ImVec2& min, const ImVec2& max, float maxValue, bool drawLabels);

    void DrawMeter(ImDrawList* pDrawList, const ImVec2& min, const ImVec2& max, float value);
} // namespace quinte::ui
