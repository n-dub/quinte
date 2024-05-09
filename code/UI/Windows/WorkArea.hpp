#pragma once
#include <UI/Utils.hpp>
#include <UI/Widgets/Tracks/TrackMixerView.hpp>

namespace quinte
{
    class EditWindow;


    class WorkArea final
    {
        memory::unique_ptr<EditWindow> m_pEditWindow;

        uint32_t m_SelectedInputDeviceIndex = 0;
        uint32_t m_SelectedOutputDeviceIndex = 0;

    public:
        WorkArea();
        ~WorkArea();
        void Draw();
    };
} // namespace quinte
