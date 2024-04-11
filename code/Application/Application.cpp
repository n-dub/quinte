#include <Application/Application.hpp>

namespace quinte
{
    void Application::DrawUI()
    {
        using namespace ImGui;

        if (Begin("TextWindow"))
        {
            if (Button("Show message box"))
            {
                const alert::Response response =
                    DisplayAlert("Text Message Box", "Test", alert::Kind::Information, alert::Buttons::OKCancel);
                if (response == alert::Response::OK)
                {
                    DisplayAlert("Pressed OK", "Test", alert::Kind::Warning, alert::Buttons::OK);
                }
                else
                {
                    DisplayAlert("Pressed Cancel", "Test", alert::Kind::Error, alert::Buttons::OK);
                }
            }
        }

        End();
    }
} // namespace quinte
