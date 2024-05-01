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
            ImVector<ImWchar> GlyphRanges;
        };
    } // namespace


    static Impl g_Impl;


    static void SetupImGuiStyle()
    {
        // Quinte style from ImThemes
        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.800000011920929f;
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.WindowRounding = 4.0f;
        style.WindowBorderSize = 1.0f;
        style.WindowMinSize = ImVec2(32.0f, 32.0f);
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ChildRounding = 4.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupRounding = 4.0f;
        style.PopupBorderSize = 1.0f;
        style.FramePadding = ImVec2(4.0f, 3.0f);
        style.FrameRounding = 4.0f;
        style.FrameBorderSize = 0.0f;
        style.ItemSpacing = ImVec2(4.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(3.0f, 4.0f);
        style.CellPadding = ImVec2(4.0f, 2.0f);
        style.IndentSpacing = 10.0f;
        style.ColumnsMinSpacing = 6.0f;
        style.ScrollbarSize = 14.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabMinSize = 10.0f;
        style.GrabRounding = 0.0f;
        style.TabRounding = 4.0f;
        style.TabBorderSize = 0.0f;
        style.TabMinWidthForCloseButton = 0.0f;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

        style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.6738197803497314f, 0.6738130450248718f, 0.6738130450248718f, 1.0f);
        style.Colors[ImGuiCol_WindowBg] =
            ImVec4(0.3175965547561646f, 0.3175933659076691f, 0.3175933659076691f, 0.9399999976158142f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_PopupBg] =
            ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.2060086131095886f, 0.2060081213712692f, 0.2060065567493439f, 1.0f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2103004455566406f, 0.2102998346090317f, 0.2102983444929123f, 1.0f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2317596673965454f, 0.2317587584257126f, 0.2317573428153992f, 1.0f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.283261775970459f, 0.2832608222961426f, 0.2832589447498322f, 1.0f);
        style.Colors[ImGuiCol_TitleBg] =
            ImVec4(0.2549019753932953f, 0.2549019753932953f, 0.2549019753932953f, 0.5254902243614197f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2549019753932953f, 0.2549019753932953f, 0.2549019753932953f, 1.0f);
        style.Colors[ImGuiCol_TitleBgCollapsed] =
            ImVec4(0.2549019753932953f, 0.2549019753932953f, 0.2549019753932953f, 0.5098039507865906f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2549019753932953f, 0.2549019753932953f, 0.2549019753932953f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarBg] =
            ImVec4(0.2549019753932953f, 0.2549019753932953f, 0.2549019753932953f, 0.529411792755127f);
        style.Colors[ImGuiCol_ScrollbarGrab] =
            ImVec4(0.4823529422283173f, 0.4823529422283173f, 0.4823529422283173f, 0.4235294163227081f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] =
            ImVec4(0.686274528503418f, 0.686274528503418f, 0.686274528503418f, 0.4000000059604645f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] =
            ImVec4(0.5960784554481506f, 0.5960784554481506f, 0.5960784554481506f, 0.6705882549285889f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.2145922183990479f, 0.7467811107635498f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6480686664581299f, 0.6480664014816284f, 0.6480621695518494f, 1.0f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.7982832789421082f, 0.7982801198959351f, 0.7982752919197083f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.3991416096687317f, 0.3991376161575317f, 0.3991376161575317f, 1.0f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.459227442741394f, 0.4592228531837463f, 0.4592228531837463f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5622317790985107f, 0.5622261762619019f, 0.5622261762619019f, 1.0f);
        style.Colors[ImGuiCol_Header] =
            ImVec4(0.4823529422283173f, 0.4823529422283173f, 0.4823529422283173f, 0.4235294163227081f);
        style.Colors[ImGuiCol_HeaderHovered] =
            ImVec4(0.686274528503418f, 0.686274528503418f, 0.686274528503418f, 0.4000000059604645f);
        style.Colors[ImGuiCol_HeaderActive] =
            ImVec4(0.5960784554481506f, 0.5960784554481506f, 0.5960784554481506f, 0.6705882549285889f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
        style.Colors[ImGuiCol_SeparatorHovered] =
            ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 0.7799999713897705f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 0.5960784554481506f, 0.0f, 0.8666666746139526f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 0.5960784554481506f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.7019608020782471f, 0.2627451121807098f, 1.0f);
        style.Colors[ImGuiCol_Tab] = ImVec4(1.0f, 0.5960784554481506f, 0.0f, 0.8666666746139526f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 0.5960784554481506f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 0.7019608020782471f, 0.2627451121807098f, 1.0f);
        style.Colors[ImGuiCol_TabUnfocused] =
            ImVec4(0.1450980454683304f, 0.1373558938503265f, 0.06666667014360428f, 0.9724000096321106f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.4235294163227081f, 0.3799377381801605f, 0.1333333551883698f, 1.0f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.5960784554481506f, 0.0f, 0.8666666746139526f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.7019608020782471f, 0.2627451121807098f, 1.0f);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
        style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
        style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
        style.Colors[ImGuiCol_TextSelectedBg] =
            ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3499999940395355f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
        style.Colors[ImGuiCol_NavWindowingDimBg] =
            ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
        style.Colors[ImGuiCol_ModalWindowDimBg] =
            ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
    }


    void Initialize()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImFontGlyphRangesBuilder grb;
        grb.AddRanges(io.Fonts->GetGlyphRangesGreek());
        grb.AddRanges(io.Fonts->GetGlyphRangesKorean());
        grb.AddRanges(io.Fonts->GetGlyphRangesJapanese());
        grb.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
        grb.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
        grb.AddRanges(io.Fonts->GetGlyphRangesThai());
        grb.AddRanges(io.Fonts->GetGlyphRangesVietnamese());

        const ImWchar iconRanges[] = {
            0xe5fa,  0xe6b5,                                  // Seti-UI + Custom
            0xe700,  0xe7c5,                                  // Devicons
            0xed00,  0xf2ff,                                  // Font Awesome
            0xe200,  0xe2a9,                                  // Font Awesome Extension
            0xf0001, 0xf1af0,                                 // Material Design Icons
            0xe300,  0xe3e3,                                  // Weather
            0xf400,  0xf533,  0x2665, 0x2665, 0x26A1, 0x26A1, // Octicons
            0xe0a0,  0xe0a2,  0xe0b0, 0xe0b3,                 // Powerline Symbols
            0x23fb,  0x23fe,  0x2b58, 0x2b58,                 // IEC Power Symbols
            0xf300,  0xf375,                                  // Font Logos
            0xe000,  0xe00a,                                  // Pomicons
            0xea60,  0xec1e,                                  // Codicons
            0x0,
        };
        grb.AddRanges(iconRanges);

        grb.BuildRanges(&g_Impl.GlyphRanges);

        g_Impl.Fonts[static_cast<size_t>(FontKind::Default)] = io.Fonts->AddFontFromFileTTF(
            "resources/fonts/JetBrainsMonoNLNerdFont-Regular.ttf", 16.0f, nullptr, g_Impl.GlyphRanges.Data);

        g_Impl.Fonts[static_cast<size_t>(FontKind::Tiny)] = io.Fonts->AddFontFromFileTTF(
            "resources/fonts/JetBrainsMonoNLNerdFont-Light.ttf", 12.0f, nullptr, g_Impl.GlyphRanges.Data);

        io.FontDefault = GetFont(FontKind::Default);

        SetupImGuiStyle();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }


    ImFont* GetFont(FontKind kind)
    {
        QU_Assert(kind < FontKind::Count);
        return g_Impl.Fonts[static_cast<size_t>(kind)];
    }
} // namespace quinte::ui
