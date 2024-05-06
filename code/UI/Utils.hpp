#pragma once
#include <Core/Core.hpp>
#include <UI/Colors.hpp>
#include <imgui.h>

namespace quinte
{
    namespace ui
    {
        enum class FontKind : uint32_t
        {
            Default,
            Tiny,
            Count,
        };


        void Initialize();
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


    struct IDScope final : NoCopyMove
    {
        inline IDScope(uint32_t id)
        {
            ImGui::PushID(id);
        }

        inline IDScope(const char* id)
        {
            ImGui::PushID(id);
        }

        inline ~IDScope()
        {
            ImGui::PopID();
        }
    };


    struct StyleScope final : NoCopyMove
    {
        bool Enabled;

        inline StyleScope(ImGuiStyleVar var, float val, bool enable = true)
            : Enabled(enable)
        {
            if (enable)
                ImGui::PushStyleVar(var, val);
        }

        inline StyleScope(ImGuiStyleVar var, const ImVec2& val, bool enable = true)
            : Enabled(enable)
        {
            if (enable)
                ImGui::PushStyleVar(var, val);
        }

        inline ~StyleScope()
        {
            if (Enabled)
                ImGui::PopStyleVar();
        }
    };


    struct ColorScope final : NoCopyMove
    {
        bool Enabled;

        inline ColorScope(ImGuiCol idx, uint32_t col, bool enable = true)
            : Enabled(enable)
        {
            if (enable)
                ImGui::PushStyleColor(idx, col);
        }

        inline ColorScope(ImGuiCol idx, const ImVec4& col, bool enable = true)
            : Enabled(enable)
        {
            if (enable)
                ImGui::PushStyleColor(idx, col);
        }

        inline ~ColorScope()
        {
            if (Enabled)
                ImGui::PopStyleColor();
        }
    };
} // namespace quinte
