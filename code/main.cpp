#include <Application/Application.hpp>
#include <memory>

int main()
{
    auto app = std::make_shared<quinte::Application>();

    if (!app->Initialize())
        return EXIT_FAILURE;

    return app->Run();
}
