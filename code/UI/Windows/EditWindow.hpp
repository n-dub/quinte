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

        int64_t m_TimelineStart = 0;
        double m_SamplesPerPixel = 100.0;

        void DrawTrackLane(TrackInfo& trackInfo);

    public:
        EditWindow();

        void DrawUI() override;
    };
} // namespace quinte
