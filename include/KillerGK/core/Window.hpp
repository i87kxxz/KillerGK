/**
 * @file Window.hpp
 * @brief Window class for KillerGK
 */

#pragma once

#include <string>
#include <functional>
#include <memory>

namespace KillerGK {

// Forward declarations
class Widget;

/**
 * @brief Handle to a built window
 */
using WindowHandle = std::shared_ptr<class WindowImpl>;

/**
 * @class Window
 * @brief Window builder class using Builder Pattern
 */
class Window {
public:
    /**
     * @brief Create a new window builder
     */
    static Window create();

    // Builder Pattern methods
    Window& title(const std::string& title);
    Window& size(int width, int height);
    Window& position(int x, int y);
    Window& icon(const std::string& path);
    Window& frameless(bool enabled);
    Window& transparent(bool enabled);
    Window& resizable(bool enabled);
    Window& alwaysOnTop(bool enabled);

    // Event callbacks
    Window& onClose(std::function<bool()> callback);
    Window& onResize(std::function<void(int, int)> callback);
    Window& onFocus(std::function<void(bool)> callback);

    // Child widget
    Window& child(Widget& widget);

    /**
     * @brief Build and show the window
     */
    WindowHandle build();

private:
    Window();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

} // namespace KillerGK
