/**
 * @file WindowsPlatform.cpp
 * @brief Windows platform implementation for KillerGK
 */

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <shellscalingapi.h>

#include "KillerGK/platform/WindowsPlatform.hpp"
#include <stdexcept>
#include <sstream>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shcore.lib")

namespace KillerGK {

// Window class name
static const wchar_t* WINDOW_CLASS_NAME = L"KillerGKWindowClass";
static bool s_windowClassRegistered = false;

// Global platform instance
static WindowsPlatform* s_platform = nullptr;

// Convert UTF-8 to wide string
static std::wstring utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}

// Convert wide string to UTF-8
static std::string wideToUtf8(const std::wstring& str) {
    if (str.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
}


// Map Windows virtual key to KeyCode
static KeyCode mapVirtualKey(WPARAM vk, LPARAM lParam) {
    switch (vk) {
        case 'A': return KeyCode::A;
        case 'B': return KeyCode::B;
        case 'C': return KeyCode::C;
        case 'D': return KeyCode::D;
        case 'E': return KeyCode::E;
        case 'F': return KeyCode::F;
        case 'G': return KeyCode::G;
        case 'H': return KeyCode::H;
        case 'I': return KeyCode::I;
        case 'J': return KeyCode::J;
        case 'K': return KeyCode::K;
        case 'L': return KeyCode::L;
        case 'M': return KeyCode::M;
        case 'N': return KeyCode::N;
        case 'O': return KeyCode::O;
        case 'P': return KeyCode::P;
        case 'Q': return KeyCode::Q;
        case 'R': return KeyCode::R;
        case 'S': return KeyCode::S;
        case 'T': return KeyCode::T;
        case 'U': return KeyCode::U;
        case 'V': return KeyCode::V;
        case 'W': return KeyCode::W;
        case 'X': return KeyCode::X;
        case 'Y': return KeyCode::Y;
        case 'Z': return KeyCode::Z;
        case '0': return KeyCode::Num0;
        case '1': return KeyCode::Num1;
        case '2': return KeyCode::Num2;
        case '3': return KeyCode::Num3;
        case '4': return KeyCode::Num4;
        case '5': return KeyCode::Num5;
        case '6': return KeyCode::Num6;
        case '7': return KeyCode::Num7;
        case '8': return KeyCode::Num8;
        case '9': return KeyCode::Num9;
        case VK_F1: return KeyCode::F1;
        case VK_F2: return KeyCode::F2;
        case VK_F3: return KeyCode::F3;
        case VK_F4: return KeyCode::F4;
        case VK_F5: return KeyCode::F5;
        case VK_F6: return KeyCode::F6;
        case VK_F7: return KeyCode::F7;
        case VK_F8: return KeyCode::F8;
        case VK_F9: return KeyCode::F9;
        case VK_F10: return KeyCode::F10;
        case VK_F11: return KeyCode::F11;
        case VK_F12: return KeyCode::F12;
        case VK_ESCAPE: return KeyCode::Escape;
        case VK_TAB: return KeyCode::Tab;
        case VK_CAPITAL: return KeyCode::CapsLock;
        case VK_SHIFT: return KeyCode::Shift;
        case VK_CONTROL: return KeyCode::Control;
        case VK_MENU: return KeyCode::Alt;
        case VK_LWIN: case VK_RWIN: return KeyCode::Super;
        case VK_SPACE: return KeyCode::Space;
        case VK_RETURN: return KeyCode::Enter;
        case VK_BACK: return KeyCode::Backspace;
        case VK_DELETE: return KeyCode::Delete;
        case VK_INSERT: return KeyCode::Insert;
        case VK_HOME: return KeyCode::Home;
        case VK_END: return KeyCode::End;
        case VK_PRIOR: return KeyCode::PageUp;
        case VK_NEXT: return KeyCode::PageDown;
        case VK_LEFT: return KeyCode::Left;
        case VK_RIGHT: return KeyCode::Right;
        case VK_UP: return KeyCode::Up;
        case VK_DOWN: return KeyCode::Down;
        default: return KeyCode::Unknown;
    }
}


// Get current modifier keys state
static ModifierKeys getModifierKeys() {
    ModifierKeys mods;
    mods.shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    mods.control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    mods.alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    mods.super = ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) != 0;
    mods.capsLock = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    mods.numLock = (GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
    return mods;
}

// Window procedure
static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (s_platform) {
        WindowsWindow* window = s_platform->getWindow(hwnd);
        if (window) {
            LRESULT result = window->handleMessage(msg, wParam, lParam);
            if (result != -1) {
                return result;
            }
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ============================================================================
// WindowsWindow Implementation
// ============================================================================

WindowsWindow::WindowsWindow(const WindowParams& params) {
    createNativeWindow(params);
}

WindowsWindow::~WindowsWindow() {
    destroyNativeWindow();
}

void WindowsWindow::createNativeWindow(const WindowParams& params) {
    m_title = params.title;
    m_width = params.width;
    m_height = params.height;
    m_frameless = params.frameless;
    m_transparent = params.transparent;
    m_resizable = params.resizable;
    m_alwaysOnTop = params.alwaysOnTop;
    m_visible = params.visible;
    
    // Calculate window style
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exStyle = WS_EX_APPWINDOW;
    
    if (m_frameless) {
        style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    }
    
    if (!m_resizable) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }
    
    if (m_alwaysOnTop) {
        exStyle |= WS_EX_TOPMOST;
    }
    
    if (m_transparent) {
        exStyle |= WS_EX_LAYERED;
    }
    
    // Calculate window rect
    RECT rect = {0, 0, m_width, m_height};
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    
    // Calculate position
    int x = params.x;
    int y = params.y;
    if (x < 0 || y < 0) {
        // Center on primary monitor
        x = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
        y = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
    }
    m_x = x;
    m_y = y;
    
    // Create window
    std::wstring wideTitle = utf8ToWide(m_title);
    m_hwnd = CreateWindowExW(
        exStyle,
        WINDOW_CLASS_NAME,
        wideTitle.c_str(),
        style,
        x, y, windowWidth, windowHeight,
        nullptr, nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );
    
    if (!m_hwnd) {
        throw std::runtime_error("Failed to create window");
    }
    
    // Register with platform
    if (s_platform) {
        s_platform->registerWindow(m_hwnd, this);
    }
    
    // Enable transparency if requested
    if (m_transparent) {
        SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    }
    
    // Show window
    if (m_visible) {
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
    }
}


void WindowsWindow::destroyNativeWindow() {
    if (m_hwnd) {
        if (s_platform) {
            s_platform->unregisterWindow(m_hwnd);
        }
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

void WindowsWindow::updateWindowStyle() {
    if (!m_hwnd) return;
    
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exStyle = WS_EX_APPWINDOW;
    
    if (m_frameless) {
        style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    }
    
    if (!m_resizable) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }
    
    if (m_alwaysOnTop) {
        exStyle |= WS_EX_TOPMOST;
    }
    
    if (m_transparent) {
        exStyle |= WS_EX_LAYERED;
    }
    
    SetWindowLongW(m_hwnd, GWL_STYLE, style);
    SetWindowLongW(m_hwnd, GWL_EXSTYLE, exStyle);
    
    // Update z-order for always on top
    SetWindowPos(m_hwnd, 
        m_alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void WindowsWindow::setTitle(const std::string& title) {
    m_title = title;
    if (m_hwnd) {
        SetWindowTextW(m_hwnd, utf8ToWide(title).c_str());
    }
}

std::string WindowsWindow::getTitle() const {
    return m_title;
}

void WindowsWindow::setSize(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_hwnd) {
        RECT rect = {0, 0, width, height};
        DWORD style = GetWindowLongW(m_hwnd, GWL_STYLE);
        DWORD exStyle = GetWindowLongW(m_hwnd, GWL_EXSTYLE);
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);
        SetWindowPos(m_hwnd, nullptr, 0, 0, 
            rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOMOVE | SWP_NOZORDER);
    }
}

void WindowsWindow::getSize(int& width, int& height) const {
    width = m_width;
    height = m_height;
}

void WindowsWindow::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
    if (m_hwnd) {
        SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void WindowsWindow::getPosition(int& x, int& y) const {
    x = m_x;
    y = m_y;
}

void WindowsWindow::setVisible(bool visible) {
    m_visible = visible;
    if (m_hwnd) {
        ShowWindow(m_hwnd, visible ? SW_SHOW : SW_HIDE);
    }
}

bool WindowsWindow::isVisible() const {
    return m_visible;
}

void WindowsWindow::setFrameless(bool frameless) {
    m_frameless = frameless;
    updateWindowStyle();
}

bool WindowsWindow::isFrameless() const {
    return m_frameless;
}

void WindowsWindow::setResizable(bool resizable) {
    m_resizable = resizable;
    updateWindowStyle();
}

bool WindowsWindow::isResizable() const {
    return m_resizable;
}

void WindowsWindow::setAlwaysOnTop(bool alwaysOnTop) {
    m_alwaysOnTop = alwaysOnTop;
    updateWindowStyle();
}

bool WindowsWindow::isAlwaysOnTop() const {
    return m_alwaysOnTop;
}

void WindowsWindow::setTransparent(bool transparent) {
    m_transparent = transparent;
    updateWindowStyle();
    if (m_hwnd && m_transparent) {
        SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    }
}

bool WindowsWindow::isTransparent() const {
    return m_transparent;
}


void WindowsWindow::minimize() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_MINIMIZE);
    }
}

void WindowsWindow::maximize() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_MAXIMIZE);
    }
}

void WindowsWindow::restore() {
    if (m_hwnd) {
        ShowWindow(m_hwnd, SW_RESTORE);
    }
}

void WindowsWindow::close() {
    m_shouldClose = true;
    if (m_hwnd) {
        PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
    }
}

void WindowsWindow::focus() {
    if (m_hwnd) {
        SetForegroundWindow(m_hwnd);
        SetFocus(m_hwnd);
    }
}

bool WindowsWindow::isMinimized() const {
    return m_hwnd && IsIconic(m_hwnd);
}

bool WindowsWindow::isMaximized() const {
    return m_hwnd && IsZoomed(m_hwnd);
}

bool WindowsWindow::isFocused() const {
    return m_hwnd && (GetForegroundWindow() == m_hwnd);
}

bool WindowsWindow::shouldClose() const {
    return m_shouldClose;
}

NativeWindowHandle WindowsWindow::getNativeHandle() const {
    NativeWindowHandle handle;
    handle.hwnd = m_hwnd;
    handle.hinstance = GetModuleHandleW(nullptr);
    return handle;
}

void WindowsWindow::setCustomTitleBar(bool enabled) {
    m_customTitleBar = enabled;
}

bool WindowsWindow::hasCustomTitleBar() const {
    return m_customTitleBar;
}

void WindowsWindow::setTitleBarHeight(int height) {
    m_titleBarHeight = height;
}

int WindowsWindow::getTitleBarHeight() const {
    return m_titleBarHeight;
}

void WindowsWindow::setCloseCallback(CloseCallback callback) {
    m_closeCallback = std::move(callback);
}

void WindowsWindow::setResizeCallback(ResizeCallback callback) {
    m_resizeCallback = std::move(callback);
}

void WindowsWindow::setFocusCallback(FocusCallback callback) {
    m_focusCallback = std::move(callback);
}

void WindowsWindow::setMoveCallback(MoveCallback callback) {
    m_moveCallback = std::move(callback);
}

void WindowsWindow::setMouseMoveCallback(MouseCallback callback) {
    m_mouseMoveCallback = std::move(callback);
}

void WindowsWindow::setMouseButtonCallback(MouseCallback callback) {
    m_mouseButtonCallback = std::move(callback);
}

void WindowsWindow::setMouseScrollCallback(MouseCallback callback) {
    m_mouseScrollCallback = std::move(callback);
}

void WindowsWindow::setKeyCallback(KeyCallback callback) {
    m_keyCallback = std::move(callback);
}

void WindowsWindow::setTouchCallback(TouchCallback callback) {
    m_touchCallback = std::move(callback);
}

void WindowsWindow::setCharCallback(CharCallback callback) {
    m_charCallback = std::move(callback);
}


long long WindowsWindow::handleMessage(unsigned int msg, unsigned long long wParam, long long lParam) {
    switch (msg) {
        case WM_CLOSE: {
            if (m_closeCallback) {
                if (!m_closeCallback()) {
                    return 0;  // Prevent close
                }
            }
            m_shouldClose = true;
            return 0;
        }
        
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            if (width > 0 && height > 0) {
                m_width = width;
                m_height = height;
                if (m_resizeCallback) {
                    m_resizeCallback(width, height);
                }
            }
            return 0;
        }
        
        case WM_MOVE: {
            m_x = (int)(short)LOWORD(lParam);
            m_y = (int)(short)HIWORD(lParam);
            if (m_moveCallback) {
                m_moveCallback(m_x, m_y);
            }
            return 0;
        }
        
        case WM_SETFOCUS: {
            if (m_focusCallback) {
                m_focusCallback(true);
            }
            return 0;
        }
        
        case WM_KILLFOCUS: {
            if (m_focusCallback) {
                m_focusCallback(false);
            }
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            if (m_mouseMoveCallback) {
                MouseEvent event;
                event.x = static_cast<float>(GET_X_LPARAM(lParam));
                event.y = static_cast<float>(GET_Y_LPARAM(lParam));
                event.modifiers = getModifierKeys();
                m_mouseMoveCallback(event);
            }
            return 0;
        }
        
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP: {
            if (m_mouseButtonCallback) {
                MouseEvent event;
                event.x = static_cast<float>(GET_X_LPARAM(lParam));
                event.y = static_cast<float>(GET_Y_LPARAM(lParam));
                event.modifiers = getModifierKeys();
                
                switch (msg) {
                    case WM_LBUTTONDOWN:
                    case WM_LBUTTONUP:
                        event.button = MouseButton::Left;
                        break;
                    case WM_RBUTTONDOWN:
                    case WM_RBUTTONUP:
                        event.button = MouseButton::Right;
                        break;
                    case WM_MBUTTONDOWN:
                    case WM_MBUTTONUP:
                        event.button = MouseButton::Middle;
                        break;
                }
                
                event.clicks = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN) ? 1 : 0;
                m_mouseButtonCallback(event);
            }
            return 0;
        }
        
        case WM_MOUSEWHEEL: {
            if (m_mouseScrollCallback) {
                MouseEvent event;
                POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                ScreenToClient(m_hwnd, &pt);
                event.x = static_cast<float>(pt.x);
                event.y = static_cast<float>(pt.y);
                event.scrollY = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
                event.modifiers = getModifierKeys();
                m_mouseScrollCallback(event);
            }
            return 0;
        }
        
        case WM_MOUSEHWHEEL: {
            if (m_mouseScrollCallback) {
                MouseEvent event;
                POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                ScreenToClient(m_hwnd, &pt);
                event.x = static_cast<float>(pt.x);
                event.y = static_cast<float>(pt.y);
                event.scrollX = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
                event.modifiers = getModifierKeys();
                m_mouseScrollCallback(event);
            }
            return 0;
        }

        
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP: {
            if (m_keyCallback) {
                KeyEvent event;
                event.key = mapVirtualKey(wParam, lParam);
                event.pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
                event.repeat = (lParam & 0x40000000) != 0;
                event.modifiers = getModifierKeys();
                m_keyCallback(event);
            }
            return 0;
        }
        
        case WM_CHAR: {
            if (m_charCallback && wParam >= 32) {
                m_charCallback(static_cast<unsigned int>(wParam));
            }
            return 0;
        }
        
        case WM_TOUCH: {
            if (m_touchCallback) {
                UINT inputCount = LOWORD(wParam);
                std::vector<TOUCHINPUT> inputs(inputCount);
                if (GetTouchInputInfo((HTOUCHINPUT)lParam, inputCount, inputs.data(), sizeof(TOUCHINPUT))) {
                    for (const auto& input : inputs) {
                        TouchEvent event;
                        event.touchId = input.dwID;
                        
                        POINT pt = {input.x / 100, input.y / 100};
                        ScreenToClient(m_hwnd, &pt);
                        event.x = static_cast<float>(pt.x);
                        event.y = static_cast<float>(pt.y);
                        
                        if (input.dwFlags & TOUCHEVENTF_DOWN) {
                            event.phase = TouchPhase::Began;
                        } else if (input.dwFlags & TOUCHEVENTF_UP) {
                            event.phase = TouchPhase::Ended;
                        } else if (input.dwFlags & TOUCHEVENTF_MOVE) {
                            event.phase = TouchPhase::Moved;
                        }
                        
                        m_touchCallback(event);
                    }
                    CloseTouchInputHandle((HTOUCHINPUT)lParam);
                }
            }
            return 0;
        }
        
        // Custom title bar hit testing
        case WM_NCHITTEST: {
            if (m_frameless && m_customTitleBar) {
                POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                ScreenToClient(m_hwnd, &pt);
                
                RECT rect;
                GetClientRect(m_hwnd, &rect);
                
                // Check if in title bar area
                if (pt.y < m_titleBarHeight) {
                    // Check for resize borders
                    const int borderWidth = 8;
                    if (pt.x < borderWidth && pt.y < borderWidth) return HTTOPLEFT;
                    if (pt.x > rect.right - borderWidth && pt.y < borderWidth) return HTTOPRIGHT;
                    if (pt.x < borderWidth) return HTLEFT;
                    if (pt.x > rect.right - borderWidth) return HTRIGHT;
                    if (pt.y < borderWidth) return HTTOP;
                    
                    return HTCAPTION;
                }
                
                // Check resize borders at bottom
                const int borderWidth = 8;
                if (pt.y > rect.bottom - borderWidth) {
                    if (pt.x < borderWidth) return HTBOTTOMLEFT;
                    if (pt.x > rect.right - borderWidth) return HTBOTTOMRIGHT;
                    return HTBOTTOM;
                }
                if (pt.x < borderWidth) return HTLEFT;
                if (pt.x > rect.right - borderWidth) return HTRIGHT;
            }
            break;
        }
        
        case WM_NCCALCSIZE: {
            if (m_frameless && wParam == TRUE) {
                // Remove the standard frame
                return 0;
            }
            break;
        }
    }
    
    return -1;  // Let default handler process
}


// ============================================================================
// WindowsPlatform Implementation
// ============================================================================

WindowsPlatform::WindowsPlatform() {
    s_platform = this;
}

WindowsPlatform::~WindowsPlatform() {
    shutdown();
    s_platform = nullptr;
}

std::string WindowsPlatform::getName() const {
    return "Windows";
}

bool WindowsPlatform::initialize() {
    if (m_initialized) return true;
    
    // Set DPI awareness
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    
    // Register window class
    if (!s_windowClassRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = windowProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));  // IDC_ARROW
        wc.lpszClassName = WINDOW_CLASS_NAME;
        
        if (!RegisterClassExW(&wc)) {
            return false;
        }
        s_windowClassRegistered = true;
    }
    
    // Initialize timer
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    m_timerFrequency = static_cast<double>(freq.QuadPart);
    
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    m_timeOffset = static_cast<double>(counter.QuadPart) / m_timerFrequency;
    
    // Load cursors - use MAKEINTRESOURCEW for wide string compatibility
    m_cursors[static_cast<int>(CursorType::Arrow)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));      // IDC_ARROW
    m_cursors[static_cast<int>(CursorType::IBeam)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32513));      // IDC_IBEAM
    m_cursors[static_cast<int>(CursorType::Crosshair)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32515));  // IDC_CROSS
    m_cursors[static_cast<int>(CursorType::Hand)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32649));       // IDC_HAND
    m_cursors[static_cast<int>(CursorType::ResizeH)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32644));    // IDC_SIZEWE
    m_cursors[static_cast<int>(CursorType::ResizeV)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32645));    // IDC_SIZENS
    m_cursors[static_cast<int>(CursorType::ResizeNESW)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32643)); // IDC_SIZENESW
    m_cursors[static_cast<int>(CursorType::ResizeNWSE)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32642)); // IDC_SIZENWSE
    m_cursors[static_cast<int>(CursorType::ResizeAll)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32646));  // IDC_SIZEALL
    m_cursors[static_cast<int>(CursorType::NotAllowed)] = LoadCursorW(nullptr, MAKEINTRESOURCEW(32648)); // IDC_NO
    
    m_initialized = true;
    return true;
}

void WindowsPlatform::shutdown() {
    if (!m_initialized) return;
    
    // Close all windows
    for (auto& [hwnd, window] : m_windows) {
        if (window) {
            DestroyWindow(hwnd);
        }
    }
    m_windows.clear();
    
    // Unregister window class
    if (s_windowClassRegistered) {
        UnregisterClassW(WINDOW_CLASS_NAME, GetModuleHandleW(nullptr));
        s_windowClassRegistered = false;
    }
    
    m_initialized = false;
}

void WindowsPlatform::pollEvents() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void WindowsPlatform::waitEvents() {
    WaitMessage();
    pollEvents();
}

void WindowsPlatform::waitEventsTimeout(double timeout) {
    MsgWaitForMultipleObjects(0, nullptr, FALSE, 
        static_cast<DWORD>(timeout * 1000.0), QS_ALLINPUT);
    pollEvents();
}

std::unique_ptr<IPlatformWindow> WindowsPlatform::createWindow(const WindowParams& params) {
    return std::make_unique<WindowsWindow>(params);
}


std::vector<DisplayInfo> WindowsPlatform::getDisplays() const {
    std::vector<DisplayInfo> displays;
    
    EnumDisplayMonitors(nullptr, nullptr, 
        [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
            auto* displays = reinterpret_cast<std::vector<DisplayInfo>*>(lParam);
            
            MONITORINFOEXW mi = {};
            mi.cbSize = sizeof(MONITORINFOEXW);
            GetMonitorInfoW(hMonitor, &mi);
            
            DisplayInfo info;
            info.name = wideToUtf8(mi.szDevice);
            info.width = mi.rcMonitor.right - mi.rcMonitor.left;
            info.height = mi.rcMonitor.bottom - mi.rcMonitor.top;
            info.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
            
            // Get DPI
            UINT dpiX, dpiY;
            if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
                info.dpiScale = static_cast<float>(dpiX) / 96.0f;
            }
            
            // Get refresh rate
            DEVMODEW dm = {};
            dm.dmSize = sizeof(DEVMODEW);
            if (EnumDisplaySettingsW(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm)) {
                info.refreshRate = dm.dmDisplayFrequency;
            }
            
            displays->push_back(info);
            return TRUE;
        }, 
        reinterpret_cast<LPARAM>(&displays));
    
    return displays;
}

DisplayInfo WindowsPlatform::getPrimaryDisplay() const {
    auto displays = getDisplays();
    for (const auto& display : displays) {
        if (display.isPrimary) {
            return display;
        }
    }
    return displays.empty() ? DisplayInfo{} : displays[0];
}

std::string WindowsPlatform::getClipboardText() const {
    if (!OpenClipboard(nullptr)) {
        return "";
    }
    
    std::string result;
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData) {
        wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
        if (pszText) {
            result = wideToUtf8(pszText);
            GlobalUnlock(hData);
        }
    }
    
    CloseClipboard();
    return result;
}

void WindowsPlatform::setClipboardText(const std::string& text) {
    if (!OpenClipboard(nullptr)) {
        return;
    }
    
    EmptyClipboard();
    
    std::wstring wideText = utf8ToWide(text);
    size_t size = (wideText.size() + 1) * sizeof(wchar_t);
    
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (hMem) {
        wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
        if (pMem) {
            memcpy(pMem, wideText.c_str(), size);
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
    }
    
    CloseClipboard();
}

void WindowsPlatform::setCursor(CursorType cursor) {
    int index = static_cast<int>(cursor);
    if (index >= 0 && index < 11 && m_cursors[index]) {
        SetCursor(static_cast<HCURSOR>(m_cursors[index]));
    }
}

void WindowsPlatform::setCursorVisible(bool visible) {
    ShowCursor(visible ? TRUE : FALSE);
}

double WindowsPlatform::getTime() const {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<double>(counter.QuadPart) / m_timerFrequency - m_timeOffset;
}

void WindowsPlatform::setTime(double time) {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    m_timeOffset = static_cast<double>(counter.QuadPart) / m_timerFrequency - time;
}

std::string WindowsPlatform::getOSVersion() const {
    std::ostringstream oss;
    oss << "Windows ";
    
    // Use RtlGetVersion for accurate version info
    typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll) {
        auto RtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
            GetProcAddress(hNtdll, "RtlGetVersion"));
        if (RtlGetVersion) {
            RTL_OSVERSIONINFOW osvi = {};
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            if (RtlGetVersion(&osvi) == 0) {
                oss << osvi.dwMajorVersion << "." << osvi.dwMinorVersion 
                    << " (Build " << osvi.dwBuildNumber << ")";
                return oss.str();
            }
        }
    }
    
    return "Windows (Unknown Version)";
}

bool WindowsPlatform::supportsFeature(const std::string& feature) const {
    if (feature == "vulkan") return true;
    if (feature == "touch") return GetSystemMetrics(SM_DIGITIZER) != 0;
    if (feature == "transparency") return true;
    if (feature == "frameless") return true;
    if (feature == "customTitleBar") return true;
    return false;
}

void WindowsPlatform::registerWindow(HWND hwnd, WindowsWindow* window) {
    m_windows[hwnd] = window;
}

void WindowsPlatform::unregisterWindow(HWND hwnd) {
    m_windows.erase(hwnd);
}

WindowsWindow* WindowsPlatform::getWindow(HWND hwnd) const {
    auto it = m_windows.find(hwnd);
    return it != m_windows.end() ? it->second : nullptr;
}

} // namespace KillerGK

#endif // _WIN32
