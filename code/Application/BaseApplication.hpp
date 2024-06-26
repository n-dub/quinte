﻿#pragma once
#include <Core/Core.hpp>
#include <Core/FixedString.hpp>
#include <UI/Alerts.hpp>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace quinte
{
    class BaseApplication : public NoCopyMove
    {
        int32_t m_ResultCode = 0;

        bool SetupWindow();
        void SetupUI();

    protected:
        const FixStr256 m_Name;
        GLFWwindow* m_pWindow = nullptr;

        ImVec4 m_ClearColor = { 41.0f, 41.0f, 41.0f, 1.0f / 255.0f };

        BaseApplication(StringSlice name);

        virtual bool SetupBackend() = 0;

        virtual bool BackendBeginFrame() = 0;
        virtual bool BackendEndFrame() = 0;

        virtual void DrawUI() = 0;

    public:
        virtual ~BaseApplication();

        [[nodiscard]] inline ImVec4 GetBackgroundColor() const
        {
            return m_ClearColor;
        }

        bool Initialize();
        int32_t Run();

        void Exit(int32_t resultCode);
        void FatalError(StringSlice message, int32_t errorCode = 1);

        alert::Response DisplayAlert(StringSlice text, StringSlice caption, alert::Kind kind, alert::Buttons buttons);
    };
} // namespace quinte
