/**
 * @file Input.hpp
 * @brief Input handling utilities for KillerGK
 * 
 * Provides helper functions and utilities for input handling.
 * Implements Requirements 11.1, 11.2, 11.3 for keyboard, mouse, and touch input.
 */

#pragma once

#include "Platform.hpp"
#include <string>

namespace KillerGK {

/**
 * @brief Mouse event type for distinguishing different mouse actions
 */
enum class MouseEventType {
    Move,       ///< Mouse moved
    ButtonDown, ///< Mouse button pressed
    ButtonUp,   ///< Mouse button released
    Scroll,     ///< Mouse wheel scrolled
    Enter,      ///< Mouse entered window
    Leave       ///< Mouse left window
};

/**
 * @brief Extended mouse event with event type
 */
struct ExtendedMouseEvent : public MouseEvent {
    MouseEventType type = MouseEventType::Move;
    bool isPressed = false;  ///< For button events, whether button is pressed
    
    ExtendedMouseEvent() = default;
    explicit ExtendedMouseEvent(const MouseEvent& base, MouseEventType t = MouseEventType::Move)
        : MouseEvent(base), type(t), isPressed(t == MouseEventType::ButtonDown) {}
};

/**
 * @brief Extended key event with additional text input support
 */
struct ExtendedKeyEvent : public KeyEvent {
    unsigned int codepoint = 0;  ///< Unicode codepoint for character input
    bool isCharacter = false;    ///< True if this is a character input event
    
    ExtendedKeyEvent() = default;
    explicit ExtendedKeyEvent(const KeyEvent& base)
        : KeyEvent(base), codepoint(0), isCharacter(false) {}
};

/**
 * @brief Input utility functions
 */
class Input {
public:
    /**
     * @brief Check if a key is currently pressed
     * @param key The key code to check
     * @return true if the key is pressed
     */
    static bool isKeyPressed(KeyCode key);
    
    /**
     * @brief Check if a mouse button is currently pressed
     * @param button The mouse button to check
     * @return true if the button is pressed
     */
    static bool isMouseButtonPressed(MouseButton button);
    
    /**
     * @brief Get the current mouse position
     * @param x Output x coordinate
     * @param y Output y coordinate
     */
    static void getMousePosition(float& x, float& y);
    
    /**
     * @brief Get the current modifier keys state
     * @return Current modifier keys state
     */
    static ModifierKeys getModifiers();
    
    /**
     * @brief Convert a key code to its string representation
     * @param key The key code
     * @return String representation of the key
     */
    static std::string keyCodeToString(KeyCode key);
    
    /**
     * @brief Convert a string to a key code
     * @param str The string representation
     * @return The key code, or KeyCode::Unknown if not found
     */
    static KeyCode stringToKeyCode(const std::string& str);
    
    /**
     * @brief Convert a mouse button to its string representation
     * @param button The mouse button
     * @return String representation of the button
     */
    static std::string mouseButtonToString(MouseButton button);
    
    /**
     * @brief Convert a touch phase to its string representation
     * @param phase The touch phase
     * @return String representation of the phase
     */
    static std::string touchPhaseToString(TouchPhase phase);
    
    /**
     * @brief Check if a key code represents a modifier key
     * @param key The key code to check
     * @return true if the key is a modifier (Shift, Control, Alt, Super)
     */
    static bool isModifierKey(KeyCode key);
    
    /**
     * @brief Create a KeyEvent from key code and state
     * @param key The key code
     * @param pressed Whether the key is pressed
     * @param repeat Whether this is a repeat event
     * @return Constructed KeyEvent with current modifiers
     */
    static KeyEvent createKeyEvent(KeyCode key, bool pressed, bool repeat = false);
    
    /**
     * @brief Create a MouseEvent from position and button
     * @param x X coordinate
     * @param y Y coordinate
     * @param button Mouse button (for button events)
     * @param clicks Number of clicks (for button events)
     * @return Constructed MouseEvent with current modifiers
     */
    static MouseEvent createMouseEvent(float x, float y, MouseButton button = MouseButton::Left, int clicks = 0);
    
    /**
     * @brief Create a TouchEvent from touch data
     * @param touchId Touch identifier
     * @param x X coordinate
     * @param y Y coordinate
     * @param phase Touch phase
     * @return Constructed TouchEvent
     */
    static TouchEvent createTouchEvent(int touchId, float x, float y, TouchPhase phase);
};

/**
 * @brief Input event dispatcher
 * 
 * Manages input event callbacks at the application level.
 * Provides a centralized event dispatch system for keyboard, mouse, and touch input.
 * 
 * Requirements: 11.1, 11.2, 11.3
 */
class InputDispatcher {
public:
    using KeyCallback = std::function<void(const KeyEvent&)>;
    using MouseCallback = std::function<void(const MouseEvent&)>;
    using TouchCallback = std::function<void(const TouchEvent&)>;
    using CharCallback = std::function<void(unsigned int)>;
    using ExtendedKeyCallback = std::function<void(const ExtendedKeyEvent&)>;
    using ExtendedMouseCallback = std::function<void(const ExtendedMouseEvent&)>;
    
    /**
     * @brief Get the singleton instance
     */
    static InputDispatcher& instance();
    
    /**
     * @brief Register a global key callback
     * @param callback The callback function
     * @return Handle to unregister the callback
     */
    int addKeyCallback(KeyCallback callback);
    
    /**
     * @brief Register a global extended key callback (includes character events)
     * @param callback The callback function
     * @return Handle to unregister the callback
     */
    int addExtendedKeyCallback(ExtendedKeyCallback callback);
    
    /**
     * @brief Register a global mouse callback
     * @param callback The callback function
     * @return Handle to unregister the callback
     */
    int addMouseCallback(MouseCallback callback);
    
    /**
     * @brief Register a global extended mouse callback (includes event type)
     * @param callback The callback function
     * @return Handle to unregister the callback
     */
    int addExtendedMouseCallback(ExtendedMouseCallback callback);
    
    /**
     * @brief Register a global touch callback
     * @param callback The callback function
     * @return Handle to unregister the callback
     */
    int addTouchCallback(TouchCallback callback);
    
    /**
     * @brief Register a global character input callback
     * @param callback The callback function
     * @return Handle to unregister the callback
     */
    int addCharCallback(CharCallback callback);
    
    /**
     * @brief Remove a callback by handle
     * @param handle The handle returned by add*Callback
     */
    void removeCallback(int handle);
    
    /**
     * @brief Remove all callbacks
     */
    void clearAllCallbacks();
    
    /**
     * @brief Get the number of registered key callbacks
     */
    size_t getKeyCallbackCount() const;
    
    /**
     * @brief Get the number of registered mouse callbacks
     */
    size_t getMouseCallbackCount() const;
    
    /**
     * @brief Get the number of registered touch callbacks
     */
    size_t getTouchCallbackCount() const;
    
    // Dispatch events - called by platform layer
    void dispatchKeyEvent(const KeyEvent& event);
    void dispatchMouseEvent(const MouseEvent& event);
    void dispatchMouseEvent(const MouseEvent& event, MouseEventType type);
    void dispatchTouchEvent(const TouchEvent& event);
    void dispatchCharEvent(unsigned int codepoint);
    
    /**
     * @brief Connect this dispatcher to a platform window
     * 
     * Sets up the window's input callbacks to automatically dispatch
     * events through this dispatcher.
     * 
     * @param window The platform window to connect
     */
    void connectToWindow(IPlatformWindow* window);
    
    /**
     * @brief Disconnect from a platform window
     * @param window The platform window to disconnect
     */
    void disconnectFromWindow(IPlatformWindow* window);
    
private:
    InputDispatcher();
    ~InputDispatcher();
    InputDispatcher(const InputDispatcher&) = delete;
    InputDispatcher& operator=(const InputDispatcher&) = delete;
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace KillerGK
