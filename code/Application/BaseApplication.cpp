#include <Application/BaseApplication.hpp>
#include <UI/Utils.hpp>

#if QU_WINDOWS
#    if !defined GLFW_EXPOSE_NATIVE_WIN32
#        define GLFW_EXPOSE_NATIVE_WIN32
#    endif
#else
#    error Unsupported
#endif

#include <GLFW/glfw3native.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>

namespace quinte
{
    static void GLFWErrorCallback(int error, const char* description)
    {
        const FixFmt512 message{ "GLFW Error: code {}\n\t{}", error, description };
        alert::Display(nullptr, message, FixFmt32{ "GLFW Error: {}", error }.Data(), alert::Kind::Error, alert::Buttons::OK);
    }


    BaseApplication::~BaseApplication()
    {
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }


    bool BaseApplication::Initialize()
    {
        if (!SetupWindow())
            return false;

        if (!SetupBackend())
            return false;

        return true;
    }


    int32_t BaseApplication::Run()
    {
        QU_Assert(m_pWindow);

        while (!glfwWindowShouldClose(m_pWindow) && m_ResultCode == 0)
        {
            glfwPollEvents();

            if (!BackendBeginFrame())
                break;

            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            DrawUI();

            ImGui::Render();
            if (!BackendEndFrame())
                break;
        }

        return m_ResultCode;
    }


    void BaseApplication::Exit(int32_t resultCode)
    {
        m_ResultCode = resultCode;
    }


    void BaseApplication::FatalError(StringSlice message, int32_t errorCode)
    {
        QU_Assert(errorCode);
        DisplayAlert(message, "Fatal Error", alert::Kind::Error, alert::Buttons::OK);
        Exit(errorCode);
    }


    alert::Response BaseApplication::DisplayAlert(StringSlice text, StringSlice caption, alert::Kind kind, alert::Buttons buttons)
    {
        return alert::Display(glfwGetWin32Window(m_pWindow), text, caption, kind, buttons);
    }


    bool BaseApplication::SetupWindow()
    {
        glfwSetErrorCallback(&GLFWErrorCallback);
        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        m_pWindow = glfwCreateWindow(1280, 720, m_Name.Data(), nullptr, nullptr);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        SetupUI();

        return m_pWindow;
    }


    void BaseApplication::SetupUI()
    {
        ui::Initialize();
    }


    BaseApplication::BaseApplication(StringSlice name)
        : m_Name(name)
    {
    }
} // namespace quinte
