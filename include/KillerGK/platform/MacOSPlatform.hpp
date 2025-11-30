/**
 * @file MacOSPlatform.hpp
 * @brief macOS platform implementation stub for KillerGK
 */

#pragma once

#ifdef __APPLE__

#include "Platform.hpp"

namespace KillerGK {

/**
 * @brief macOS-specific window implementation (stub)
 */
class MacOSWindow : public IPlatformWindow {
public:
    explicit MacOSWindow(const WindowParams& params);
    ~MacOSWindow() override;
    
    // Window properties
    void setTitle(const std::string& title) override;
    std::string getTitle() const override;
    
    void setSize(int width, int height) override;
    void getSize(int& width, int& height) const override;
    
    void setPosition(int x, int y) override;
    void getPosition(int& x, int& y) const override;
    
    void setVisible(bool visible) override;
    bool isVisible() const override;
    
    void setFrameless(bool frameless) override;
    bool isFrameless() const override;
    
    void setResizable(bool resizable) override;
    bool isResizable() const override;
    
    void setAlwaysOnTop(bool alwaysOnTop) override;
    bool isAlwaysOnTop() const override;
    
    void setTransparent(bool transparent) override;
    bool isTransparent() const override;
    
    // Window operations
    void minimize() override;
    void maximize() override;
    void restore() override;
    void close() override;
    void focus() override;
    
    bool isMinimized() const override;
    bool isMaximized() const override;
    bool isFocused() const override;
    bool shouldClose() const override;

    
    // Native handle
    NativeWindowHandle getNativeHandle() const override;
    
    // Custom title bar support
    void setCustomTitleBar(bool enabled) override;
    bool hasCustomTitleBar() const override;
    void setTitleBarHeight(int height) override;
    int getTitleBarHeight() const override;
    
    // Event callbacks
    void setCloseCallback(CloseCallback callback) override;
    void setResizeCallback(ResizeCallback callback) override;
    void setFocusCallback(FocusCallback callback) override;
    void setMoveCallback(MoveCallback callback) override;
    void setMouseMoveCallback(MouseCallback callback) override;
    void setMouseButtonCallback(MouseCallback callback) override;
    void setMouseScrollCallback(MouseCallback callback) override;
    void setKeyCallback(KeyCallback callback) override;
    void setTouchCallback(TouchCallback callback) override;
    void setCharCallback(CharCallback callback) override;
    
private:
    std::string m_title;
    int m_width = 800;
    int m_height = 600;
    bool m_shouldClose = false;
};

/**
 * @brief macOS platform implementation (stub)
 */
class MacOSPlatform : public IPlatform {
public:
    MacOSPlatform();
    ~MacOSPlatform() override;
    
    std::string getName() const override;
    bool initialize() override;
    void shutdown() override;
    
    void pollEvents() override;
    void waitEvents() override;
    void waitEventsTimeout(double timeout) override;
    
    std::unique_ptr<IPlatformWindow> createWindow(const WindowParams& params) override;
    
    std::vector<DisplayInfo> getDisplays() const override;
    DisplayInfo getPrimaryDisplay() const override;
    
    std::string getClipboardText() const override;
    void setClipboardText(const std::string& text) override;
    
    void setCursor(CursorType cursor) override;
    void setCursorVisible(bool visible) override;
    
    double getTime() const override;
    void setTime(double time) override;
    
    std::string getOSVersion() const override;
    bool supportsFeature(const std::string& feature) const override;
    
private:
    bool m_initialized = false;
};

} // namespace KillerGK

#endif // __APPLE__
