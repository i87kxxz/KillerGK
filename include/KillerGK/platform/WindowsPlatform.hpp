/**
 * @file WindowsPlatform.hpp
 * @brief Windows platform implementation for KillerGK
 */

#pragma once

#ifdef _WIN32

#include "Platform.hpp"
#include <unordered_map>

// Forward declare Windows types to avoid including windows.h in header
struct HWND__;
typedef HWND__* HWND;

namespace KillerGK {

/**
 * @brief Windows-specific window implementation
 */
class WindowsWindow : public IPlatformWindow {
public:
    explicit WindowsWindow(const WindowParams& params);
    ~WindowsWindow() override;
    
    // Prevent copying
    WindowsWindow(const WindowsWindow&) = delete;
    WindowsWindow& operator=(const WindowsWindow&) = delete;
    
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
    
    // Internal: Handle Windows messages
    long long handleMessage(unsigned int msg, unsigned long long wParam, long long lParam);
    
private:
    void createNativeWindow(const WindowParams& params);
    void destroyNativeWindow();
    void updateWindowStyle();
    
    HWND m_hwnd = nullptr;
    std::string m_title;
    int m_width = 800;
    int m_height = 600;
    int m_x = 0;
    int m_y = 0;
    bool m_frameless = false;
    bool m_transparent = false;
    bool m_resizable = true;
    bool m_alwaysOnTop = false;
    bool m_visible = true;
    bool m_shouldClose = false;
    bool m_customTitleBar = false;
    int m_titleBarHeight = 32;
    
    // Callbacks
    CloseCallback m_closeCallback;
    ResizeCallback m_resizeCallback;
    FocusCallback m_focusCallback;
    MoveCallback m_moveCallback;
    MouseCallback m_mouseMoveCallback;
    MouseCallback m_mouseButtonCallback;
    MouseCallback m_mouseScrollCallback;
    KeyCallback m_keyCallback;
    TouchCallback m_touchCallback;
    CharCallback m_charCallback;
};


/**
 * @brief Windows platform implementation
 */
class WindowsPlatform : public IPlatform {
public:
    WindowsPlatform();
    ~WindowsPlatform() override;
    
    // Prevent copying
    WindowsPlatform(const WindowsPlatform&) = delete;
    WindowsPlatform& operator=(const WindowsPlatform&) = delete;
    
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
    
    // Internal: Register/unregister windows
    void registerWindow(HWND hwnd, WindowsWindow* window);
    void unregisterWindow(HWND hwnd);
    WindowsWindow* getWindow(HWND hwnd) const;
    
private:
    bool m_initialized = false;
    double m_timeOffset = 0.0;
    double m_timerFrequency = 0.0;
    std::unordered_map<HWND, WindowsWindow*> m_windows;
    void* m_cursors[11] = {nullptr};  // Array of cursor handles
};

} // namespace KillerGK

#endif // _WIN32
