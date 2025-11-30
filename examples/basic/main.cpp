/**
 * @file main.cpp
 * @brief Basic KillerGK example
 */

#include <KillerGK/KillerGK.hpp>

int main() {
    using namespace KillerGK;

    // Create theme
    auto theme = Theme::material();

    // Create and run application
    Application::instance()
        .title("KillerGK Basic Example")
        .size(1280, 720)
        .theme(theme)
        .run([]() {
            // Create main window
            auto window = Window::create()
                .title("Hello KillerGK!")
                .size(800, 600)
                .resizable(true)
                .build();
        });

    return 0;
}
