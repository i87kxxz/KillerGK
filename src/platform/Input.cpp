/**
 * @file Input.cpp
 * @brief Input handling utilities implementation for KillerGK
 * 
 * Implements Requirements 11.1, 11.2, 11.3 for keyboard, mouse, and touch input.
 */

#include "KillerGK/platform/Input.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <algorithm>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace KillerGK {

// ============================================================================
// Input Static Methods
// ============================================================================

bool Input::isKeyPressed(KeyCode key) {
#ifdef _WIN32
    int vk = 0;
    switch (key) {
        case KeyCode::A: vk = 'A'; break;
        case KeyCode::B: vk = 'B'; break;
        case KeyCode::C: vk = 'C'; break;
        case KeyCode::D: vk = 'D'; break;
        case KeyCode::E: vk = 'E'; break;
        case KeyCode::F: vk = 'F'; break;
        case KeyCode::G: vk = 'G'; break;
        case KeyCode::H: vk = 'H'; break;
        case KeyCode::I: vk = 'I'; break;
        case KeyCode::J: vk = 'J'; break;
        case KeyCode::K: vk = 'K'; break;
        case KeyCode::L: vk = 'L'; break;
        case KeyCode::M: vk = 'M'; break;
        case KeyCode::N: vk = 'N'; break;
        case KeyCode::O: vk = 'O'; break;
        case KeyCode::P: vk = 'P'; break;
        case KeyCode::Q: vk = 'Q'; break;
        case KeyCode::R: vk = 'R'; break;
        case KeyCode::S: vk = 'S'; break;
        case KeyCode::T: vk = 'T'; break;
        case KeyCode::U: vk = 'U'; break;
        case KeyCode::V: vk = 'V'; break;
        case KeyCode::W: vk = 'W'; break;
        case KeyCode::X: vk = 'X'; break;
        case KeyCode::Y: vk = 'Y'; break;
        case KeyCode::Z: vk = 'Z'; break;
        case KeyCode::Num0: vk = '0'; break;
        case KeyCode::Num1: vk = '1'; break;
        case KeyCode::Num2: vk = '2'; break;
        case KeyCode::Num3: vk = '3'; break;
        case KeyCode::Num4: vk = '4'; break;
        case KeyCode::Num5: vk = '5'; break;
        case KeyCode::Num6: vk = '6'; break;
        case KeyCode::Num7: vk = '7'; break;
        case KeyCode::Num8: vk = '8'; break;
        case KeyCode::Num9: vk = '9'; break;
        case KeyCode::F1: vk = VK_F1; break;
        case KeyCode::F2: vk = VK_F2; break;
        case KeyCode::F3: vk = VK_F3; break;
        case KeyCode::F4: vk = VK_F4; break;
        case KeyCode::F5: vk = VK_F5; break;
        case KeyCode::F6: vk = VK_F6; break;
        case KeyCode::F7: vk = VK_F7; break;
        case KeyCode::F8: vk = VK_F8; break;
        case KeyCode::F9: vk = VK_F9; break;
        case KeyCode::F10: vk = VK_F10; break;
        case KeyCode::F11: vk = VK_F11; break;
        case KeyCode::F12: vk = VK_F12; break;
        case KeyCode::Space: vk = VK_SPACE; break;
        case KeyCode::Enter: vk = VK_RETURN; break;
        case KeyCode::Escape: vk = VK_ESCAPE; break;
        case KeyCode::Tab: vk = VK_TAB; break;
        case KeyCode::Backspace: vk = VK_BACK; break;
        case KeyCode::Delete: vk = VK_DELETE; break;
        case KeyCode::Insert: vk = VK_INSERT; break;
        case KeyCode::Home: vk = VK_HOME; break;
        case KeyCode::End: vk = VK_END; break;
        case KeyCode::PageUp: vk = VK_PRIOR; break;
        case KeyCode::PageDown: vk = VK_NEXT; break;
        case KeyCode::Left: vk = VK_LEFT; break;
        case KeyCode::Right: vk = VK_RIGHT; break;
        case KeyCode::Up: vk = VK_UP; break;
        case KeyCode::Down: vk = VK_DOWN; break;
        case KeyCode::Shift: vk = VK_SHIFT; break;
        case KeyCode::Control: vk = VK_CONTROL; break;
        case KeyCode::Alt: vk = VK_MENU; break;
        case KeyCode::Super: vk = VK_LWIN; break;
        case KeyCode::CapsLock: vk = VK_CAPITAL; break;
        default: return false;
    }
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
#else
    // TODO: Implement for other platforms
    (void)key;
    return false;
#endif
}

bool Input::isMouseButtonPressed(MouseButton button) {
#ifdef _WIN32
    int vk = 0;
    switch (button) {
        case MouseButton::Left: vk = VK_LBUTTON; break;
        case MouseButton::Right: vk = VK_RBUTTON; break;
        case MouseButton::Middle: vk = VK_MBUTTON; break;
        case MouseButton::X1: vk = VK_XBUTTON1; break;
        case MouseButton::X2: vk = VK_XBUTTON2; break;
    }
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
#else
    // TODO: Implement for other platforms
    (void)button;
    return false;
#endif
}

void Input::getMousePosition(float& x, float& y) {
#ifdef _WIN32
    POINT pt;
    GetCursorPos(&pt);
    x = static_cast<float>(pt.x);
    y = static_cast<float>(pt.y);
#else
    // TODO: Implement for other platforms
    x = 0;
    y = 0;
#endif
}

ModifierKeys Input::getModifiers() {
    ModifierKeys mods;
#ifdef _WIN32
    mods.shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    mods.control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    mods.alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    mods.super = ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) != 0;
    mods.capsLock = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    mods.numLock = (GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
#endif
    return mods;
}

std::string Input::mouseButtonToString(MouseButton button) {
    switch (button) {
        case MouseButton::Left: return "Left";
        case MouseButton::Right: return "Right";
        case MouseButton::Middle: return "Middle";
        case MouseButton::X1: return "X1";
        case MouseButton::X2: return "X2";
    }
    return "Unknown";
}

std::string Input::touchPhaseToString(TouchPhase phase) {
    switch (phase) {
        case TouchPhase::Began: return "Began";
        case TouchPhase::Moved: return "Moved";
        case TouchPhase::Stationary: return "Stationary";
        case TouchPhase::Ended: return "Ended";
        case TouchPhase::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

bool Input::isModifierKey(KeyCode key) {
    return key == KeyCode::Shift ||
           key == KeyCode::Control ||
           key == KeyCode::Alt ||
           key == KeyCode::Super ||
           key == KeyCode::CapsLock;
}

KeyEvent Input::createKeyEvent(KeyCode key, bool pressed, bool repeat) {
    KeyEvent event;
    event.key = key;
    event.pressed = pressed;
    event.repeat = repeat;
    event.modifiers = getModifiers();
    return event;
}

MouseEvent Input::createMouseEvent(float x, float y, MouseButton button, int clicks) {
    MouseEvent event;
    event.x = x;
    event.y = y;
    event.button = button;
    event.clicks = clicks;
    event.scrollX = 0.0f;
    event.scrollY = 0.0f;
    event.modifiers = getModifiers();
    return event;
}

TouchEvent Input::createTouchEvent(int touchId, float x, float y, TouchPhase phase) {
    TouchEvent event;
    event.touchId = touchId;
    event.x = x;
    event.y = y;
    event.phase = phase;
    return event;
}


std::string Input::keyCodeToString(KeyCode key) {
    static const std::unordered_map<KeyCode, std::string> keyNames = {
        {KeyCode::A, "A"}, {KeyCode::B, "B"}, {KeyCode::C, "C"},
        {KeyCode::D, "D"}, {KeyCode::E, "E"}, {KeyCode::F, "F"},
        {KeyCode::G, "G"}, {KeyCode::H, "H"}, {KeyCode::I, "I"},
        {KeyCode::J, "J"}, {KeyCode::K, "K"}, {KeyCode::L, "L"},
        {KeyCode::M, "M"}, {KeyCode::N, "N"}, {KeyCode::O, "O"},
        {KeyCode::P, "P"}, {KeyCode::Q, "Q"}, {KeyCode::R, "R"},
        {KeyCode::S, "S"}, {KeyCode::T, "T"}, {KeyCode::U, "U"},
        {KeyCode::V, "V"}, {KeyCode::W, "W"}, {KeyCode::X, "X"},
        {KeyCode::Y, "Y"}, {KeyCode::Z, "Z"},
        {KeyCode::Num0, "0"}, {KeyCode::Num1, "1"}, {KeyCode::Num2, "2"},
        {KeyCode::Num3, "3"}, {KeyCode::Num4, "4"}, {KeyCode::Num5, "5"},
        {KeyCode::Num6, "6"}, {KeyCode::Num7, "7"}, {KeyCode::Num8, "8"},
        {KeyCode::Num9, "9"},
        {KeyCode::F1, "F1"}, {KeyCode::F2, "F2"}, {KeyCode::F3, "F3"},
        {KeyCode::F4, "F4"}, {KeyCode::F5, "F5"}, {KeyCode::F6, "F6"},
        {KeyCode::F7, "F7"}, {KeyCode::F8, "F8"}, {KeyCode::F9, "F9"},
        {KeyCode::F10, "F10"}, {KeyCode::F11, "F11"}, {KeyCode::F12, "F12"},
        {KeyCode::Escape, "Escape"}, {KeyCode::Tab, "Tab"},
        {KeyCode::CapsLock, "CapsLock"}, {KeyCode::Shift, "Shift"},
        {KeyCode::Control, "Control"}, {KeyCode::Alt, "Alt"},
        {KeyCode::Super, "Super"}, {KeyCode::Space, "Space"},
        {KeyCode::Enter, "Enter"}, {KeyCode::Backspace, "Backspace"},
        {KeyCode::Delete, "Delete"}, {KeyCode::Insert, "Insert"},
        {KeyCode::Home, "Home"}, {KeyCode::End, "End"},
        {KeyCode::PageUp, "PageUp"}, {KeyCode::PageDown, "PageDown"},
        {KeyCode::Left, "Left"}, {KeyCode::Right, "Right"},
        {KeyCode::Up, "Up"}, {KeyCode::Down, "Down"},
    };
    
    auto it = keyNames.find(key);
    return it != keyNames.end() ? it->second : "Unknown";
}

KeyCode Input::stringToKeyCode(const std::string& str) {
    static const std::unordered_map<std::string, KeyCode> keyMap = {
        {"A", KeyCode::A}, {"B", KeyCode::B}, {"C", KeyCode::C},
        {"D", KeyCode::D}, {"E", KeyCode::E}, {"F", KeyCode::F},
        {"G", KeyCode::G}, {"H", KeyCode::H}, {"I", KeyCode::I},
        {"J", KeyCode::J}, {"K", KeyCode::K}, {"L", KeyCode::L},
        {"M", KeyCode::M}, {"N", KeyCode::N}, {"O", KeyCode::O},
        {"P", KeyCode::P}, {"Q", KeyCode::Q}, {"R", KeyCode::R},
        {"S", KeyCode::S}, {"T", KeyCode::T}, {"U", KeyCode::U},
        {"V", KeyCode::V}, {"W", KeyCode::W}, {"X", KeyCode::X},
        {"Y", KeyCode::Y}, {"Z", KeyCode::Z},
        {"0", KeyCode::Num0}, {"1", KeyCode::Num1}, {"2", KeyCode::Num2},
        {"3", KeyCode::Num3}, {"4", KeyCode::Num4}, {"5", KeyCode::Num5},
        {"6", KeyCode::Num6}, {"7", KeyCode::Num7}, {"8", KeyCode::Num8},
        {"9", KeyCode::Num9},
        {"F1", KeyCode::F1}, {"F2", KeyCode::F2}, {"F3", KeyCode::F3},
        {"F4", KeyCode::F4}, {"F5", KeyCode::F5}, {"F6", KeyCode::F6},
        {"F7", KeyCode::F7}, {"F8", KeyCode::F8}, {"F9", KeyCode::F9},
        {"F10", KeyCode::F10}, {"F11", KeyCode::F11}, {"F12", KeyCode::F12},
        {"Escape", KeyCode::Escape}, {"Tab", KeyCode::Tab},
        {"CapsLock", KeyCode::CapsLock}, {"Shift", KeyCode::Shift},
        {"Control", KeyCode::Control}, {"Alt", KeyCode::Alt},
        {"Super", KeyCode::Super}, {"Space", KeyCode::Space},
        {"Enter", KeyCode::Enter}, {"Backspace", KeyCode::Backspace},
        {"Delete", KeyCode::Delete}, {"Insert", KeyCode::Insert},
        {"Home", KeyCode::Home}, {"End", KeyCode::End},
        {"PageUp", KeyCode::PageUp}, {"PageDown", KeyCode::PageDown},
        {"Left", KeyCode::Left}, {"Right", KeyCode::Right},
        {"Up", KeyCode::Up}, {"Down", KeyCode::Down},
    };
    
    auto it = keyMap.find(str);
    return it != keyMap.end() ? it->second : KeyCode::Unknown;
}


// ============================================================================
// InputDispatcher Implementation
// ============================================================================

struct InputDispatcher::Impl {
    mutable std::mutex mutex;
    int nextHandle = 1;
    std::unordered_map<int, KeyCallback> keyCallbacks;
    std::unordered_map<int, ExtendedKeyCallback> extendedKeyCallbacks;
    std::unordered_map<int, MouseCallback> mouseCallbacks;
    std::unordered_map<int, ExtendedMouseCallback> extendedMouseCallbacks;
    std::unordered_map<int, TouchCallback> touchCallbacks;
    std::unordered_map<int, CharCallback> charCallbacks;
    std::unordered_set<IPlatformWindow*> connectedWindows;
    
    // Track last character event for combining with key events
    unsigned int lastCharCodepoint = 0;
    bool hasLastChar = false;
};

InputDispatcher::InputDispatcher() : m_impl(std::make_unique<Impl>()) {}

InputDispatcher::~InputDispatcher() = default;

InputDispatcher& InputDispatcher::instance() {
    static InputDispatcher instance;
    return instance;
}

int InputDispatcher::addKeyCallback(KeyCallback callback) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    int handle = m_impl->nextHandle++;
    m_impl->keyCallbacks[handle] = std::move(callback);
    return handle;
}

int InputDispatcher::addExtendedKeyCallback(ExtendedKeyCallback callback) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    int handle = m_impl->nextHandle++;
    m_impl->extendedKeyCallbacks[handle] = std::move(callback);
    return handle;
}

int InputDispatcher::addMouseCallback(MouseCallback callback) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    int handle = m_impl->nextHandle++;
    m_impl->mouseCallbacks[handle] = std::move(callback);
    return handle;
}

int InputDispatcher::addExtendedMouseCallback(ExtendedMouseCallback callback) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    int handle = m_impl->nextHandle++;
    m_impl->extendedMouseCallbacks[handle] = std::move(callback);
    return handle;
}

int InputDispatcher::addTouchCallback(TouchCallback callback) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    int handle = m_impl->nextHandle++;
    m_impl->touchCallbacks[handle] = std::move(callback);
    return handle;
}

int InputDispatcher::addCharCallback(CharCallback callback) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    int handle = m_impl->nextHandle++;
    m_impl->charCallbacks[handle] = std::move(callback);
    return handle;
}

void InputDispatcher::removeCallback(int handle) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->keyCallbacks.erase(handle);
    m_impl->extendedKeyCallbacks.erase(handle);
    m_impl->mouseCallbacks.erase(handle);
    m_impl->extendedMouseCallbacks.erase(handle);
    m_impl->touchCallbacks.erase(handle);
    m_impl->charCallbacks.erase(handle);
}

void InputDispatcher::clearAllCallbacks() {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->keyCallbacks.clear();
    m_impl->extendedKeyCallbacks.clear();
    m_impl->mouseCallbacks.clear();
    m_impl->extendedMouseCallbacks.clear();
    m_impl->touchCallbacks.clear();
    m_impl->charCallbacks.clear();
}

size_t InputDispatcher::getKeyCallbackCount() const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->keyCallbacks.size() + m_impl->extendedKeyCallbacks.size();
}

size_t InputDispatcher::getMouseCallbackCount() const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->mouseCallbacks.size() + m_impl->extendedMouseCallbacks.size();
}

size_t InputDispatcher::getTouchCallbackCount() const {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->touchCallbacks.size();
}

void InputDispatcher::dispatchKeyEvent(const KeyEvent& event) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    // Dispatch to basic key callbacks
    for (const auto& [handle, callback] : m_impl->keyCallbacks) {
        callback(event);
    }
    
    // Dispatch to extended key callbacks
    ExtendedKeyEvent extEvent(event);
    for (const auto& [handle, callback] : m_impl->extendedKeyCallbacks) {
        callback(extEvent);
    }
}

void InputDispatcher::dispatchMouseEvent(const MouseEvent& event) {
    dispatchMouseEvent(event, MouseEventType::Move);
}

void InputDispatcher::dispatchMouseEvent(const MouseEvent& event, MouseEventType type) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    // Dispatch to basic mouse callbacks
    for (const auto& [handle, callback] : m_impl->mouseCallbacks) {
        callback(event);
    }
    
    // Dispatch to extended mouse callbacks with type
    ExtendedMouseEvent extEvent(event, type);
    for (const auto& [handle, callback] : m_impl->extendedMouseCallbacks) {
        callback(extEvent);
    }
}

void InputDispatcher::dispatchTouchEvent(const TouchEvent& event) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    for (const auto& [handle, callback] : m_impl->touchCallbacks) {
        callback(event);
    }
}

void InputDispatcher::dispatchCharEvent(unsigned int codepoint) {
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    
    // Store for potential combination with key events
    m_impl->lastCharCodepoint = codepoint;
    m_impl->hasLastChar = true;
    
    // Dispatch to char callbacks
    for (const auto& [handle, callback] : m_impl->charCallbacks) {
        callback(codepoint);
    }
    
    // Also dispatch as extended key event for character input
    ExtendedKeyEvent extEvent;
    extEvent.codepoint = codepoint;
    extEvent.isCharacter = true;
    extEvent.pressed = true;
    extEvent.modifiers = Input::getModifiers();
    
    for (const auto& [handle, callback] : m_impl->extendedKeyCallbacks) {
        callback(extEvent);
    }
}

void InputDispatcher::connectToWindow(IPlatformWindow* window) {
    if (!window) return;
    
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        if (m_impl->connectedWindows.count(window) > 0) {
            return; // Already connected
        }
        m_impl->connectedWindows.insert(window);
    }
    
    // Set up keyboard callback
    window->setKeyCallback([this](const KeyEvent& event) {
        dispatchKeyEvent(event);
    });
    
    // Set up character callback
    window->setCharCallback([this](unsigned int codepoint) {
        dispatchCharEvent(codepoint);
    });
    
    // Set up mouse move callback
    window->setMouseMoveCallback([this](const MouseEvent& event) {
        dispatchMouseEvent(event, MouseEventType::Move);
    });
    
    // Set up mouse button callback
    window->setMouseButtonCallback([this](const MouseEvent& event) {
        MouseEventType type = (event.clicks > 0) ? MouseEventType::ButtonDown : MouseEventType::ButtonUp;
        dispatchMouseEvent(event, type);
    });
    
    // Set up mouse scroll callback
    window->setMouseScrollCallback([this](const MouseEvent& event) {
        dispatchMouseEvent(event, MouseEventType::Scroll);
    });
    
    // Set up touch callback
    window->setTouchCallback([this](const TouchEvent& event) {
        dispatchTouchEvent(event);
    });
}

void InputDispatcher::disconnectFromWindow(IPlatformWindow* window) {
    if (!window) return;
    
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->connectedWindows.erase(window);
    
    // Clear the callbacks (set to nullptr/empty)
    window->setKeyCallback(nullptr);
    window->setCharCallback(nullptr);
    window->setMouseMoveCallback(nullptr);
    window->setMouseButtonCallback(nullptr);
    window->setMouseScrollCallback(nullptr);
    window->setTouchCallback(nullptr);
}

} // namespace KillerGK
