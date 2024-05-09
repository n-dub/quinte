#pragma once
#include <UI/Widgets/Tracks/TrackEditView.hpp>
#include <UI/Windows/BaseWindow.hpp>

namespace quinte
{
    class EditWindow final
        : public BaseWindow
        , public Interface<EditWindow>::Registrar
    {
        float m_LeftPanelSize = 150.0f;

    public:
        EditWindow();

        void DrawUI() override;
    };
} // namespace quinte
