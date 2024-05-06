#pragma once
#include <Core/Interface.hpp>
#include <Core/FixedString.hpp>
#include <UI/Utils.hpp>

namespace quinte
{
    class BaseWindow
    {
    protected:
        FixStr64 m_Name;

        inline BaseWindow(StringSlice name)
            : m_Name(name)
        {
        }

    public:
        [[nodiscard]] inline StringSlice GetName() const
        {
            return m_Name;
        }

        virtual void DrawUI() = 0;
    };
} // namespace quinte
