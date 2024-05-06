#pragma once
#include <Core/FixedString.hpp>
#include <Core/String.hpp>
#include <UI/Utils.hpp>

namespace quinte
{
    namespace ui
    {
        bool Knob(const char* strId, float* pValue, const char* leftLabel, const char* rightLabel, float minValue = -1,
                  float maxValue = 1);

        bool Fader(const char* strId, float* pValue, float meterValue, float height);

        bool ToggleButton(const char* label, bool* pToggled, const ImVec2& size = ImVec2{ 0, 0 }, uint32_t color = 0);

        void DrawAmplitudeScale(ImDrawList* pDrawList, const ImVec2& min, const ImVec2& max, float maxValue, bool drawLabels);

        void DrawMeter(ImDrawList* pDrawList, const ImVec2& min, const ImVec2& max, float value);
    } // namespace ui


    class TrackMixerView final
    {
    public:
        String Name;
        uint32_t ID = 0;
        uint32_t Color = 0;

        float Volume = 1.0f;
        float MaxVolume = 1.0f;
        float FaderAmplitude = 1.0f;
        float Pan = 0.0f;
        bool Soloed = false;
        bool Muted = false;
        bool Monitored = false;
        bool RecordArmed = false;

        bool Draw();
    };
} // namespace quinte
