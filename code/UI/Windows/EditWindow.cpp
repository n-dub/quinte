#include <UI/Windows/EditWindow.hpp>

namespace quinte
{
    void EditWindow::DrawUI()
    {
        using namespace ImGui;

        Begin(m_Name.Data(), nullptr, ImGuiWindowFlags_NoNavFocus);

        Text("Edit window");

        End();
    }
} // namespace quinte
