#include <Application/Application.hpp>

int main()
{
    using namespace quinte;

    auto app = memory::make_unique<quinte::Application>();

    if (!app->Initialize())
        return EXIT_FAILURE;

    return app->Run();
}
