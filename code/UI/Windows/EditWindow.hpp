#pragma once
#include <UI/Windows/BaseWindow.hpp>

namespace quinte
{
    class EditWindow final
        : public BaseWindow
        , public Interface<EditWindow>::Registrar
    {
    public:
        inline EditWindow()
            : BaseWindow("EditWindow")
        {
        }

        void DrawUI() override;
    };
} // namespace quinte
