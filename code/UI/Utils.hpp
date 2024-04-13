#pragma once
#include <Core/Core.hpp>
#include <imgui.h>

namespace quinte
{
    namespace ui
    {
        enum class FontKind : uint32_t
        {
            Default,
            Count,
        };


        void LoadResources();
        ImFont* GetFont(FontKind kind);
    } // namespace ui


    struct FontScope final : NoCopyMove
    {
        inline FontScope(ui::FontKind kind)
        {
            ImGui::PushFont(ui::GetFont(kind));
        }

        inline ~FontScope()
        {
            ImGui::PopFont();
        }
    };
} // namespace quinte
