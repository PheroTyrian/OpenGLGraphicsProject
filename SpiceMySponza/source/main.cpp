#include "MyController.hpp"

#include <tygra/Window.hpp>

#include <crtdbg.h>
#include <cstdlib>
#include <iostream>
#include <memory>

int main(int argc, char *argv[])
{
    // enable debug memory checks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    try {
        auto controller = std::make_unique<MyController>();
        auto window = tygra::Window::mainWindow();
        window->setController(controller.get());

        const int window_width = 1280;
        const int window_height = 720;
        const int number_of_samples = 4;

        if (window->open(window_width, window_height,
            number_of_samples, true)) {
            while (window->isVisible()) {
                window->update();
            }
            window->close();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Opps ... something went wrong:" << std::endl;
        std::cerr << e.what() << std::endl;
    }

    // pause to display any console debug messages
    system("PAUSE");
    return 0;
}
