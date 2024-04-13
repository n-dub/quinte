#include <UI/Utils.hpp>
#include <array>
#include <vector>

namespace quinte::ui
{
    namespace
    {
        struct Impl
        {
            std::array<ImFont*, static_cast<size_t>(FontKind::Count)> Fonts;
        };
    } // namespace


    static Impl g_Impl;


    void LoadResources()
    {
        ImGuiIO& io = ImGui::GetIO();
        g_Impl.Fonts[static_cast<size_t>(FontKind::Default)] =
            io.Fonts->AddFontFromFileTTF("resources/fonts/JetBrainsMonoNLNerdFont-Regular.ttf", 18.0f);

        io.FontDefault = GetFont(FontKind::Default);
    }


    ImFont* GetFont(FontKind kind)
    {
        QUINTE_Assert(kind < FontKind::Count);
        return g_Impl.Fonts[static_cast<size_t>(kind)];
    }
} // namespace quinte::ui
