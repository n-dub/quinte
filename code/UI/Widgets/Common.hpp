#pragma once
#include <UI/Utils.hpp>

namespace quinte::ui
{
    bool Knob(const char* strId, float* pValue, const char* leftLabel, const char* rightLabel, float minValue = -1,
              float maxValue = 1);

    bool ToggleButton(const char* label, bool* pToggled, const ImVec2& size = ImVec2{ 0, 0 }, uint32_t color = 0);

    bool Splitter(bool splitVertically, float thickness, float* size1, float* size2, float minSize1, float minSize2,
                  float splitterLongAxisSize = -1.0f);
} // namespace quinte::ui
