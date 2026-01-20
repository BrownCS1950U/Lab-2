#include "window.h"

int main(int argc, char *argv[])
{

    // Check command line arguments
    if (argc < 2)
    {
        std::cout << "Usage: viewer [filename.obj]" << std::endl;
        return 0;
    }

    Window::initialize(argv[1]);

    while (Window::isActive())
    {
        Window::update();
    }

    Window::cleanup();
    return 0;
}