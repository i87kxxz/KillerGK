/**
 * @file Platform.hpp
 * @brief Platform abstraction interface for KillerGK
 * 
 * Provides a unified interface for OS-specific operations including
 * window creation, input handling, and system integration.
 */

#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <cstdint>

namespace KillerGK {

// Forward declarations
struct Color;
struct Rect;
struct Point;
struct Size;

/**
 * @brief Mouse button identifiers
 */
enum class MouseButton {
    Left,
    Right,
    Middle,
    X1,
    X2
};

/**
 * @brief Key codes for keyboard input
 */
enum class KeyCode {
    Unknown = 0,
    
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // Numbers
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    // Navigation
    Escape, Tab, CapsLock, Shift, Control, Alt, Super,
    Space, Enter, Backspace, Delete, Insert,
    Home, End, PageUp, PageDown,
    Left, Right, Up, Down,
    
    // Punctuation
    Comma, Period, Slash, Semicolon, Quote,
    LeftBracket, RightBracket, Backslash, Grave, Minus, Equal,
    
    // Numpad
    NumPad0, NumPad1, NumPad2, NumPad3, NumPad4,
    NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
    NumPadAdd, NumPadSubtract, NumPadMultiply, NumPadDivide,
    NumPadDecimal, NumPadEnter, NumLock,
    
    // Other
    PrintScreen, ScrollLock, Pause
};


/**
 * @brief Modifier key flags
 */
struct ModifierKeys {
    bool shift = false;
    bool control = false;
    bool alt = false;
    bool super = false;
    bool capsLock = false;
    bool numLock = false;
    
    bool operator==(const ModifierKeys& other) const = default;
};

/**
 * @brief Touch phase for touch events
 */
enum class TouchPhase {
    Began,
    Moved,
    Stationary,
    Ended,
    Cancelled
};

/**
 * @brief Mouse event data
 */
struct MouseEvent {
    float x = 0.0f;
    float y = 0.0f;
    MouseButton button = MouseButton::Left;
    int clicks = 0;
    float scrollX = 0.0f;
    float scrollY = 0.0f;
    ModifierKeys modifiers;
};

/**
 * @brief Keyboard event data
 */
struct KeyEvent {
    KeyCode key = KeyCode::Unknown;
    bool pressed = false;
    bool repeat = false;
    ModifierKeys modifiers;
    std::string text;  // For text input
};

/**
 * @brief Touch event data
 */
struct TouchEvent {
    int touchId = 0;
    float x = 0.0f;
    float y = 0.0f;
    TouchPhase phase = TouchPhase::Began;
};

/**
 * @brief Native window handle (platform-specific)
 */
struct NativeWindowHandle {
#ifdef _WIN32
    void* hwnd = nullptr;      // HWND on Windows
    void* hinstance = nullptr; // HINSTANCE on Windows
#elif defined(__linux__)
    void* display = nullptr;   // Display* on X11
    unsigned long window = 0;  // Window on X11
#elif defined(__APPLE__)
    void* nsWindow = nullptr;  // NSWindow* on macOS
    void* nsView = nullptr;    // NSView* on macOS
#endif
};


/**
 * @brief Window creation parameters
 */
struct WindowParams {
    std::string title = "KillerGK Window";
    int width = 800;
    int height = 600;
    int x = -1;  // -1 means centered
    int y = -1;
    bool frameless = false;
    bool transparent = false;
    bool resizable = true;
    bool alwaysOnTop = false;
    bool visible = true;
    std::string iconPath;
};

/**
 * @brief Display/monitor information
 */
struct DisplayInfo {
    std::string name;
    int width = 0;
    int height = 0;
    int refreshRate = 0;
    float dpiScale = 1.0f;
    bool isPrimary = false;
};

/**
 * @brief Platform window interface
 */
class IPlatformWindow {
public:
    virtual ~IPlatformWindow() = default;
    
    // Window properties
    virtual void setTitle(const std::string& title) = 0;
    virtual std::string getTitle() const = 0;
    
    virtual void setSize(int width, int height) = 0;
    virtual void getSize(int& width, int& height) const = 0;
    
    virtual void setPosition(int x, int y) = 0;
    virtual void getPosition(int& x, int& y) const = 0;
    
    virtual void setVisible(bool visible) = 0;
    virtual bool isVisible() const = 0;
    
    virtual void setFrameless(bool frameless) = 0;
    virtual bool isFrameless() const = 0;
    
    virtual void setResizable(bool resizable) = 0;
    virtual bool isResizable() const = 0;
    
    virtual void setAlwaysOnTop(bool alwaysOnTop) = 0;
    virtual bool isAlwaysOnTop() const = 0;
    
    virtual void setTransparent(bool transparent) = 0;
    virtual bool isTransparent() const = 0;
    
    // Window operations
    virtual void minimize() = 0;
    virtual void maximize() = 0;
    virtual void restore() = 0;
    virtual void close() = 0;
    virtual void focus() = 0;
    
    virtual bool isMinimized() const = 0;
    virtual bool isMaximized() const = 0;
    virtual bool isFocused() const = 0;
    virtual bool shouldClose() const = 0;
    
    // Native handle
    virtual NativeWindowHandle getNativeHandle() const = 0;
    
    // Custom title bar support
    virtual void setCustomTitleBar(bool enabled) = 0;
    virtual bool hasCustomTitleBar() const = 0;
    virtual void setTitleBarHeight(int height) = 0;
    virtual int getTitleBarHeight() const = 0;
    
    // Event callbacks
    using CloseCallback = std::function<bool()>;
    using ResizeCallback = std::function<void(int, int)>;
    using FocusCallback = std::function<void(bool)>;
    using MoveCallback = std::function<void(int, int)>;
    using MouseCallback = std::function<void(const MouseEvent&)>;
    using KeyCallback = std::function<void(const KeyEvent&)>;
    using TouchCallback = std::function<void(const TouchEvent&)>;
    using CharCallback = std::function<void(unsigned int)>;
    
    virtual void setCloseCallback(CloseCallback callback) = 0;
    virtual void setResizeCallback(ResizeCallback callback) = 0;
    virtual void setFocusCallback(FocusCallback callback) = 0;
    virtual void setMoveCallback(MoveCallback callback) = 0;
    virtual void setMouseMoveCallback(MouseCallback callback) = 0;
    virtual void setMouseButtonCallback(MouseCallback callback) = 0;
    virtual void setMouseScrollCallback(MouseCallback callback) = 0;
    virtual void setKeyCallback(KeyCallback callback) = 0;
    virtual void setTouchCallback(TouchCallback callback) = 0;
    virtual void setCharCallback(CharCallback callback) = 0;
};


/**
 * @brief Platform abstraction interface
 * 
 * Provides a unified interface for OS-specific operations.
 * Each platform (Windows, Linux, macOS) implements this interface.
 */
class IPlatform {
public:
    virtual ~IPlatform() = default;
    
    /**
     * @brief Get the platform name
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Initialize the platform
     * @return true if initialization succeeded
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Shutdown the platform
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Poll for events
     */
    virtual void pollEvents() = 0;
    
    /**
     * @brief Wait for events (blocks until event occurs)
     */
    virtual void waitEvents() = 0;
    
    /**
     * @brief Wait for events with timeout
     * @param timeout Timeout in seconds
     */
    virtual void waitEventsTimeout(double timeout) = 0;
    
    // Window management
    /**
     * @brief Create a new window
     * @param params Window creation parameters
     * @return Unique pointer to the created window
     */
    virtual std::unique_ptr<IPlatformWindow> createWindow(const WindowParams& params) = 0;
    
    // Display information
    /**
     * @brief Get information about all connected displays
     */
    virtual std::vector<DisplayInfo> getDisplays() const = 0;
    
    /**
     * @brief Get the primary display
     */
    virtual DisplayInfo getPrimaryDisplay() const = 0;
    
    // Clipboard
    /**
     * @brief Get text from clipboard
     */
    virtual std::string getClipboardText() const = 0;
    
    /**
     * @brief Set text to clipboard
     */
    virtual void setClipboardText(const std::string& text) = 0;
    
    // Cursor
    enum class CursorType {
        Arrow,
        IBeam,
        Crosshair,
        Hand,
        ResizeH,
        ResizeV,
        ResizeNESW,
        ResizeNWSE,
        ResizeAll,
        NotAllowed,
        Hidden
    };
    
    /**
     * @brief Set the cursor type
     */
    virtual void setCursor(CursorType cursor) = 0;
    
    /**
     * @brief Show or hide the cursor
     */
    virtual void setCursorVisible(bool visible) = 0;
    
    // Time
    /**
     * @brief Get high-resolution time in seconds
     */
    virtual double getTime() const = 0;
    
    /**
     * @brief Set the time base
     */
    virtual void setTime(double time) = 0;
    
    // System information
    /**
     * @brief Get the OS version string
     */
    virtual std::string getOSVersion() const = 0;
    
    /**
     * @brief Check if the platform supports a feature
     */
    virtual bool supportsFeature(const std::string& feature) const = 0;
};

/**
 * @brief Get the current platform instance
 * @return Reference to the platform singleton
 */
IPlatform& getPlatform();

/**
 * @brief Create a platform instance for the current OS
 * @return Unique pointer to the platform
 */
std::unique_ptr<IPlatform> createPlatform();

} // namespace KillerGK
