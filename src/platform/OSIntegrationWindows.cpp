/**
 * @file OSIntegrationWindows.cpp
 * @brief Windows-specific OS integration implementation for KillerGK
 */

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <intrin.h>
#include <powerbase.h>
#include <dxgi.h>
#include <shellscalingapi.h>
#include <mmsystem.h>
#include <unordered_map>

#include "KillerGK/platform/OSIntegration.hpp"
#include <sstream>
#include <algorithm>
#include <atomic>
#include <mutex>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "winmm.lib")

namespace KillerGK {

// ============================================================================
// Utility Functions
// ============================================================================

static std::wstring utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}

static std::string wideToUtf8(const std::wstring& str) {
    if (str.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
}

// ============================================================================
// Windows File Dialog Implementation
// ============================================================================

class WindowsFileDialog : public IFileDialog {
public:
    WindowsFileDialog() {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    }
    
    ~WindowsFileDialog() override {
        CoUninitialize();
    }
    
    FileDialogResult showOpenDialog(const OpenFileDialogOptions& options) override {
        FileDialogResult result;
        
        IFileOpenDialog* pFileOpen = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
        
        if (FAILED(hr)) return result;
        
        // Set options
        DWORD dwFlags;
        pFileOpen->GetOptions(&dwFlags);
        dwFlags |= FOS_FORCEFILESYSTEM;
        if (options.allowMultiple) dwFlags |= FOS_ALLOWMULTISELECT;
        if (options.showHidden) dwFlags |= FOS_FORCESHOWHIDDEN;
        pFileOpen->SetOptions(dwFlags);
        
        // Set title
        if (!options.title.empty()) {
            pFileOpen->SetTitle(utf8ToWide(options.title).c_str());
        }
        
        // Set default path
        if (!options.defaultPath.empty()) {
            IShellItem* pFolder = nullptr;
            hr = SHCreateItemFromParsingName(utf8ToWide(options.defaultPath).c_str(),
                nullptr, IID_IShellItem, reinterpret_cast<void**>(&pFolder));
            if (SUCCEEDED(hr)) {
                pFileOpen->SetFolder(pFolder);
                pFolder->Release();
            }
        }
        
        // Set filters
        std::vector<COMDLG_FILTERSPEC> filterSpecs;
        std::vector<std::wstring> filterNames;
        std::vector<std::wstring> filterPatterns;
        
        for (const auto& filter : options.filters) {
            filterNames.push_back(utf8ToWide(filter.name));
            filterPatterns.push_back(utf8ToWide(filter.extensions));
        }
        
        for (size_t i = 0; i < options.filters.size(); ++i) {
            COMDLG_FILTERSPEC spec;
            spec.pszName = filterNames[i].c_str();
            spec.pszSpec = filterPatterns[i].c_str();
            filterSpecs.push_back(spec);
        }
        
        if (!filterSpecs.empty()) {
            pFileOpen->SetFileTypes(static_cast<UINT>(filterSpecs.size()), filterSpecs.data());
        }
        
        // Show dialog
        hr = pFileOpen->Show(nullptr);
        
        if (SUCCEEDED(hr)) {
            result.success = true;
            
            // Get selected filter index
            UINT filterIndex;
            if (SUCCEEDED(pFileOpen->GetFileTypeIndex(&filterIndex))) {
                result.filterIndex = static_cast<int>(filterIndex - 1);
            }
            
            if (options.allowMultiple) {
                IShellItemArray* pItems = nullptr;
                hr = pFileOpen->GetResults(&pItems);
                if (SUCCEEDED(hr)) {
                    DWORD count;
                    pItems->GetCount(&count);
                    for (DWORD i = 0; i < count; ++i) {
                        IShellItem* pItem = nullptr;
                        if (SUCCEEDED(pItems->GetItemAt(i, &pItem))) {
                            PWSTR pszPath = nullptr;
                            if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                                result.paths.push_back(wideToUtf8(pszPath));
                                CoTaskMemFree(pszPath);
                            }
                            pItem->Release();
                        }
                    }
                    pItems->Release();
                }
            } else {
                IShellItem* pItem = nullptr;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszPath = nullptr;
                    if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                        result.paths.push_back(wideToUtf8(pszPath));
                        CoTaskMemFree(pszPath);
                    }
                    pItem->Release();
                }
            }
        }
        
        pFileOpen->Release();
        return result;
    }
    
    FileDialogResult showSaveDialog(const SaveFileDialogOptions& options) override {
        FileDialogResult result;
        
        IFileSaveDialog* pFileSave = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL,
            IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));
        
        if (FAILED(hr)) return result;
        
        // Set options
        DWORD dwFlags;
        pFileSave->GetOptions(&dwFlags);
        dwFlags |= FOS_FORCEFILESYSTEM;
        if (options.confirmOverwrite) dwFlags |= FOS_OVERWRITEPROMPT;
        pFileSave->SetOptions(dwFlags);
        
        // Set title
        if (!options.title.empty()) {
            pFileSave->SetTitle(utf8ToWide(options.title).c_str());
        }
        
        // Set default filename
        if (!options.defaultName.empty()) {
            pFileSave->SetFileName(utf8ToWide(options.defaultName).c_str());
        }
        
        // Set default path
        if (!options.defaultPath.empty()) {
            IShellItem* pFolder = nullptr;
            hr = SHCreateItemFromParsingName(utf8ToWide(options.defaultPath).c_str(),
                nullptr, IID_IShellItem, reinterpret_cast<void**>(&pFolder));
            if (SUCCEEDED(hr)) {
                pFileSave->SetFolder(pFolder);
                pFolder->Release();
            }
        }
        
        // Set filters
        std::vector<COMDLG_FILTERSPEC> filterSpecs;
        std::vector<std::wstring> filterNames;
        std::vector<std::wstring> filterPatterns;
        
        for (const auto& filter : options.filters) {
            filterNames.push_back(utf8ToWide(filter.name));
            filterPatterns.push_back(utf8ToWide(filter.extensions));
        }
        
        for (size_t i = 0; i < options.filters.size(); ++i) {
            COMDLG_FILTERSPEC spec;
            spec.pszName = filterNames[i].c_str();
            spec.pszSpec = filterPatterns[i].c_str();
            filterSpecs.push_back(spec);
        }
        
        if (!filterSpecs.empty()) {
            pFileSave->SetFileTypes(static_cast<UINT>(filterSpecs.size()), filterSpecs.data());
        }
        
        // Show dialog
        hr = pFileSave->Show(nullptr);
        
        if (SUCCEEDED(hr)) {
            result.success = true;
            
            // Get selected filter index
            UINT filterIndex;
            if (SUCCEEDED(pFileSave->GetFileTypeIndex(&filterIndex))) {
                result.filterIndex = static_cast<int>(filterIndex - 1);
            }
            
            IShellItem* pItem = nullptr;
            hr = pFileSave->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                    result.paths.push_back(wideToUtf8(pszPath));
                    CoTaskMemFree(pszPath);
                }
                pItem->Release();
            }
        }
        
        pFileSave->Release();
        return result;
    }
    
    FileDialogResult showFolderDialog(const FolderDialogOptions& options) override {
        FileDialogResult result;
        
        IFileOpenDialog* pFileOpen = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
        
        if (FAILED(hr)) return result;
        
        // Set options for folder selection
        DWORD dwFlags;
        pFileOpen->GetOptions(&dwFlags);
        dwFlags |= FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM;
        pFileOpen->SetOptions(dwFlags);
        
        // Set title
        if (!options.title.empty()) {
            pFileOpen->SetTitle(utf8ToWide(options.title).c_str());
        }
        
        // Set default path
        if (!options.defaultPath.empty()) {
            IShellItem* pFolder = nullptr;
            hr = SHCreateItemFromParsingName(utf8ToWide(options.defaultPath).c_str(),
                nullptr, IID_IShellItem, reinterpret_cast<void**>(&pFolder));
            if (SUCCEEDED(hr)) {
                pFileOpen->SetFolder(pFolder);
                pFolder->Release();
            }
        }
        
        // Show dialog
        hr = pFileOpen->Show(nullptr);
        
        if (SUCCEEDED(hr)) {
            result.success = true;
            
            IShellItem* pItem = nullptr;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                    result.paths.push_back(wideToUtf8(pszPath));
                    CoTaskMemFree(pszPath);
                }
                pItem->Release();
            }
        }
        
        pFileOpen->Release();
        return result;
    }
};


// ============================================================================
// Windows System Tray Implementation
// ============================================================================

class WindowsSystemTray : public ISystemTray {
public:
    WindowsSystemTray() {
        // Create a hidden message window for tray notifications
        createMessageWindow();
    }
    
    ~WindowsSystemTray() override {
        hide();
        destroyMessageWindow();
    }
    
    bool setIcon(const std::string& iconPath) override {
        m_iconPath = iconPath;
        
        // Load icon from file
        HICON hIcon = nullptr;
        if (!iconPath.empty()) {
            hIcon = (HICON)LoadImageW(nullptr, utf8ToWide(iconPath).c_str(),
                IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
        }
        
        if (!hIcon) {
            // Use default application icon
            hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1));
            if (!hIcon) {
                hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));  // IDI_APPLICATION
            }
        }
        
        if (m_hIcon && m_hIcon != hIcon) {
            DestroyIcon(m_hIcon);
        }
        m_hIcon = hIcon;
        
        if (m_visible) {
            updateTrayIcon();
        }
        
        return m_hIcon != nullptr;
    }
    
    void setTooltip(const std::string& tooltip) override {
        m_tooltip = tooltip;
        if (m_visible) {
            updateTrayIcon();
        }
    }
    
    void setMenu(const std::vector<TrayMenuItem>& items) override {
        m_menuItems = items;
        rebuildMenu();
    }
    
    void show() override {
        if (m_visible) return;
        
        NOTIFYICONDATAW nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATAW);
        nid.hWnd = m_hwnd;
        nid.uID = TRAY_ICON_ID;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = m_hIcon ? m_hIcon : LoadIconW(nullptr, MAKEINTRESOURCEW(32512));
        
        std::wstring wideTooltip = utf8ToWide(m_tooltip);
        wcsncpy_s(nid.szTip, wideTooltip.c_str(), _TRUNCATE);
        
        Shell_NotifyIconW(NIM_ADD, &nid);
        m_visible = true;
    }
    
    void hide() override {
        if (!m_visible) return;
        
        NOTIFYICONDATAW nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATAW);
        nid.hWnd = m_hwnd;
        nid.uID = TRAY_ICON_ID;
        
        Shell_NotifyIconW(NIM_DELETE, &nid);
        m_visible = false;
    }
    
    bool isVisible() const override {
        return m_visible;
    }
    
    void setOnClick(std::function<void()> callback) override {
        m_onClick = std::move(callback);
    }
    
    void setOnDoubleClick(std::function<void()> callback) override {
        m_onDoubleClick = std::move(callback);
    }

private:
    static constexpr UINT TRAY_ICON_ID = 1;
    static constexpr UINT WM_TRAYICON = WM_USER + 1;
    
    HWND m_hwnd = nullptr;
    HICON m_hIcon = nullptr;
    HMENU m_hMenu = nullptr;
    std::string m_iconPath;
    std::string m_tooltip;
    std::vector<TrayMenuItem> m_menuItems;
    bool m_visible = false;
    std::function<void()> m_onClick;
    std::function<void()> m_onDoubleClick;
    std::vector<std::function<void()>> m_menuCallbacks;
    
    static WindowsSystemTray* s_instance;
    
    void createMessageWindow() {
        static const wchar_t* CLASS_NAME = L"KillerGKTrayClass";
        static bool classRegistered = false;
        
        if (!classRegistered) {
            WNDCLASSEXW wc = {};
            wc.cbSize = sizeof(WNDCLASSEXW);
            wc.lpfnWndProc = trayWndProc;
            wc.hInstance = GetModuleHandleW(nullptr);
            wc.lpszClassName = CLASS_NAME;
            RegisterClassExW(&wc);
            classRegistered = true;
        }
        
        m_hwnd = CreateWindowExW(0, CLASS_NAME, L"", 0, 0, 0, 0, 0,
            HWND_MESSAGE, nullptr, GetModuleHandleW(nullptr), nullptr);
        
        s_instance = this;
    }
    
    void destroyMessageWindow() {
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        if (m_hMenu) {
            DestroyMenu(m_hMenu);
            m_hMenu = nullptr;
        }
        if (m_hIcon) {
            DestroyIcon(m_hIcon);
            m_hIcon = nullptr;
        }
        s_instance = nullptr;
    }
    
    void updateTrayIcon() {
        NOTIFYICONDATAW nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATAW);
        nid.hWnd = m_hwnd;
        nid.uID = TRAY_ICON_ID;
        nid.uFlags = NIF_ICON | NIF_TIP;
        nid.hIcon = m_hIcon ? m_hIcon : LoadIconW(nullptr, MAKEINTRESOURCEW(32512));
        
        std::wstring wideTooltip = utf8ToWide(m_tooltip);
        wcsncpy_s(nid.szTip, wideTooltip.c_str(), _TRUNCATE);
        
        Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
    
    void rebuildMenu() {
        if (m_hMenu) {
            DestroyMenu(m_hMenu);
        }
        m_hMenu = CreatePopupMenu();
        m_menuCallbacks.clear();
        
        buildMenuItems(m_hMenu, m_menuItems);
    }
    
    void buildMenuItems(HMENU menu, const std::vector<TrayMenuItem>& items) {
        for (const auto& item : items) {
            if (item.type == TrayMenuItem::Type::Separator) {
                AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
            } else if (item.type == TrayMenuItem::Type::Submenu && !item.submenu.empty()) {
                HMENU submenu = CreatePopupMenu();
                buildMenuItems(submenu, item.submenu);
                AppendMenuW(menu, MF_POPUP | (item.enabled ? 0 : MF_GRAYED),
                    (UINT_PTR)submenu, utf8ToWide(item.label).c_str());
            } else {
                UINT flags = MF_STRING;
                if (!item.enabled) flags |= MF_GRAYED;
                if (item.type == TrayMenuItem::Type::Checkbox && item.checked) {
                    flags |= MF_CHECKED;
                }
                
                UINT id = static_cast<UINT>(m_menuCallbacks.size() + 1);
                m_menuCallbacks.push_back(item.onClick);
                AppendMenuW(menu, flags, id, utf8ToWide(item.label).c_str());
            }
        }
    }
    
    void showContextMenu() {
        if (!m_hMenu) return;
        
        POINT pt;
        GetCursorPos(&pt);
        
        SetForegroundWindow(m_hwnd);
        UINT cmd = TrackPopupMenu(m_hMenu, TPM_RETURNCMD | TPM_NONOTIFY,
            pt.x, pt.y, 0, m_hwnd, nullptr);
        PostMessageW(m_hwnd, WM_NULL, 0, 0);
        
        if (cmd > 0 && cmd <= m_menuCallbacks.size()) {
            auto& callback = m_menuCallbacks[cmd - 1];
            if (callback) callback();
        }
    }
    
    static LRESULT CALLBACK trayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_TRAYICON && s_instance) {
            switch (LOWORD(lParam)) {
                case WM_LBUTTONUP:
                    if (s_instance->m_onClick) s_instance->m_onClick();
                    break;
                case WM_LBUTTONDBLCLK:
                    if (s_instance->m_onDoubleClick) s_instance->m_onDoubleClick();
                    break;
                case WM_RBUTTONUP:
                    s_instance->showContextMenu();
                    break;
            }
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
};

WindowsSystemTray* WindowsSystemTray::s_instance = nullptr;

// ============================================================================
// Windows Notification Implementation
// ============================================================================

/**
 * @brief Windows notification implementation using Shell_NotifyIcon balloon notifications
 * 
 * This implementation provides native Windows notifications with support for:
 * - Title and body text
 * - Custom icons
 * - Timeout configuration
 * - Click callbacks
 * - Action buttons (via context menu on balloon click)
 * 
 * Requirements 14.3: Send native OS notifications, Support notification actions
 */
class WindowsNotification : public INotification {
public:
    WindowsNotification() {
        // Create a hidden message window for notification callbacks
        createMessageWindow();
    }
    
    ~WindowsNotification() override {
        closeAll();
        destroyMessageWindow();
    }
    
    std::string show(const NotificationOptions& options) override {
        std::string id = generateId();
        
        // Store notification data for callbacks
        NotificationData data;
        data.id = id;
        data.options = options;
        data.trayId = m_nextId++;
        
        // Create notification icon data
        NOTIFYICONDATAW nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATAW);
        nid.hWnd = m_hwnd;
        nid.uID = data.trayId;
        nid.uFlags = NIF_INFO | NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP;
        nid.uCallbackMessage = WM_NOTIFICATION;
        
        // Set icon - try to load custom icon, fall back to info icon
        if (!options.iconPath.empty()) {
            data.hIcon = (HICON)LoadImageW(nullptr, utf8ToWide(options.iconPath).c_str(),
                IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
        }
        if (!data.hIcon) {
            data.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32516));  // IDI_INFORMATION
        }
        nid.hIcon = data.hIcon;
        
        // Set balloon info type based on whether we have actions
        if (!options.actions.empty()) {
            nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
        } else {
            nid.dwInfoFlags = NIIF_INFO;
        }
        
        // Set title and body
        std::wstring wideTitle = utf8ToWide(options.title);
        std::wstring wideBody = utf8ToWide(options.body);
        
        // If we have actions, append them to the body as hints
        if (!options.actions.empty()) {
            wideBody += L"\n\n";
            for (size_t i = 0; i < options.actions.size(); ++i) {
                wideBody += L"[" + std::to_wstring(i + 1) + L"] " + 
                           utf8ToWide(options.actions[i].label);
                if (i < options.actions.size() - 1) {
                    wideBody += L"  ";
                }
            }
        }
        
        wcsncpy_s(nid.szInfoTitle, wideTitle.c_str(), _TRUNCATE);
        wcsncpy_s(nid.szInfo, wideBody.c_str(), _TRUNCATE);
        wcsncpy_s(nid.szTip, wideTitle.c_str(), _TRUNCATE);
        
        // Set timeout (Windows may ignore this on newer versions)
        if (options.timeoutMs > 0) {
            nid.uTimeout = static_cast<UINT>(options.timeoutMs);
        }
        
        // Add the notification icon and show balloon
        Shell_NotifyIconW(NIM_ADD, &nid);
        Shell_NotifyIconW(NIM_MODIFY, &nid);
        
        // Play sound unless silent
        if (!options.silent) {
            if (!options.soundPath.empty()) {
                PlaySoundW(utf8ToWide(options.soundPath).c_str(), nullptr, 
                          SND_FILENAME | SND_ASYNC);
            } else {
                // Play default notification sound
                PlaySoundW(L"SystemNotification", nullptr, SND_ALIAS | SND_ASYNC);
            }
        }
        
        // Store notification data
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_notifications[id] = std::move(data);
            m_trayIdToNotifId[data.trayId] = id;
        }
        
        return id;
    }
    
    void close(const std::string& id) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_notifications.find(id);
        if (it != m_notifications.end()) {
            removeNotificationIcon(it->second.trayId);
            
            // Call onClose callback
            if (it->second.options.onClose) {
                it->second.options.onClose();
            }
            
            // Clean up icon if we created one
            if (it->second.hIcon) {
                DestroyIcon(it->second.hIcon);
            }
            
            m_trayIdToNotifId.erase(it->second.trayId);
            m_notifications.erase(it);
        }
    }
    
    void closeAll() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [id, data] : m_notifications) {
            removeNotificationIcon(data.trayId);
            if (data.options.onClose) {
                data.options.onClose();
            }
            if (data.hIcon) {
                DestroyIcon(data.hIcon);
            }
        }
        m_notifications.clear();
        m_trayIdToNotifId.clear();
    }
    
    bool isSupported() const override {
        return true;
    }
    
    bool requestPermission() override {
        // No permission needed on Windows for balloon notifications
        return true;
    }

private:
    static constexpr UINT WM_NOTIFICATION = WM_USER + 100;
    
    struct NotificationData {
        std::string id;
        NotificationOptions options;
        UINT trayId = 0;
        HICON hIcon = nullptr;
    };
    
    HWND m_hwnd = nullptr;
    std::atomic<UINT> m_nextId{1000};
    std::unordered_map<std::string, NotificationData> m_notifications;
    std::unordered_map<UINT, std::string> m_trayIdToNotifId;
    std::mutex m_mutex;
    
    static WindowsNotification* s_instance;
    
    std::string generateId() {
        static std::atomic<uint64_t> counter{0};
        return "notif_" + std::to_string(++counter);
    }
    
    void createMessageWindow() {
        static const wchar_t* CLASS_NAME = L"KillerGKNotificationClass";
        static bool classRegistered = false;
        
        if (!classRegistered) {
            WNDCLASSEXW wc = {};
            wc.cbSize = sizeof(WNDCLASSEXW);
            wc.lpfnWndProc = notificationWndProc;
            wc.hInstance = GetModuleHandleW(nullptr);
            wc.lpszClassName = CLASS_NAME;
            RegisterClassExW(&wc);
            classRegistered = true;
        }
        
        m_hwnd = CreateWindowExW(0, CLASS_NAME, L"", 0, 0, 0, 0, 0,
            HWND_MESSAGE, nullptr, GetModuleHandleW(nullptr), nullptr);
        
        s_instance = this;
    }
    
    void destroyMessageWindow() {
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        s_instance = nullptr;
    }
    
    void removeNotificationIcon(UINT trayId) {
        NOTIFYICONDATAW nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATAW);
        nid.hWnd = m_hwnd;
        nid.uID = trayId;
        Shell_NotifyIconW(NIM_DELETE, &nid);
    }
    
    void handleNotificationClick(UINT trayId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto idIt = m_trayIdToNotifId.find(trayId);
        if (idIt == m_trayIdToNotifId.end()) return;
        
        auto it = m_notifications.find(idIt->second);
        if (it == m_notifications.end()) return;
        
        const auto& data = it->second;
        
        // If we have actions, show a context menu
        if (!data.options.actions.empty()) {
            showActionsMenu(data);
        } else if (data.options.onClick) {
            // No actions, just call onClick
            data.options.onClick();
        }
    }
    
    void showActionsMenu(const NotificationData& data) {
        HMENU hMenu = CreatePopupMenu();
        if (!hMenu) return;
        
        // Add action items
        for (size_t i = 0; i < data.options.actions.size(); ++i) {
            AppendMenuW(hMenu, MF_STRING, i + 1, 
                       utf8ToWide(data.options.actions[i].label).c_str());
        }
        
        // Add separator and dismiss option
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, 1000, L"Dismiss");
        
        POINT pt;
        GetCursorPos(&pt);
        
        SetForegroundWindow(m_hwnd);
        UINT cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY,
            pt.x, pt.y, 0, m_hwnd, nullptr);
        PostMessageW(m_hwnd, WM_NULL, 0, 0);
        
        DestroyMenu(hMenu);
        
        if (cmd > 0 && cmd < 1000) {
            // Action selected
            size_t actionIndex = cmd - 1;
            if (actionIndex < data.options.actions.size()) {
                if (data.options.onAction) {
                    data.options.onAction(data.options.actions[actionIndex].id);
                }
            }
        }
        
        // Close the notification after interaction
        // Note: We need to unlock mutex before calling close()
        std::string notifId = data.id;
        m_mutex.unlock();
        close(notifId);
        m_mutex.lock();
    }
    
    void handleBalloonClick(UINT trayId) {
        handleNotificationClick(trayId);
    }
    
    void handleBalloonTimeout(UINT trayId) {
        std::string notifId;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto idIt = m_trayIdToNotifId.find(trayId);
            if (idIt != m_trayIdToNotifId.end()) {
                notifId = idIt->second;
            }
        }
        if (!notifId.empty()) {
            close(notifId);
        }
    }
    
    static LRESULT CALLBACK notificationWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_NOTIFICATION && s_instance) {
            UINT trayId = static_cast<UINT>(wParam);
            switch (LOWORD(lParam)) {
                case NIN_BALLOONUSERCLICK:
                    s_instance->handleBalloonClick(trayId);
                    break;
                case NIN_BALLOONTIMEOUT:
                case NIN_BALLOONHIDE:
                    s_instance->handleBalloonTimeout(trayId);
                    break;
                case WM_LBUTTONUP:
                    s_instance->handleNotificationClick(trayId);
                    break;
            }
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
};

WindowsNotification* WindowsNotification::s_instance = nullptr;


// ============================================================================
// Windows Clipboard Implementation
// ============================================================================

class WindowsClipboard : public IClipboard {
public:
    WindowsClipboard() = default;
    ~WindowsClipboard() override = default;
    
    bool setText(const std::string& text) override {
        if (!OpenClipboard(nullptr)) return false;
        
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
        return hMem != nullptr;
    }
    
    std::string getText() const override {
        if (!OpenClipboard(nullptr)) return "";
        
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
    
    bool hasText() const override {
        return IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;
    }
    
    bool setHtml(const std::string& html) override {
        static UINT CF_HTML = RegisterClipboardFormatW(L"HTML Format");
        if (!CF_HTML) return false;
        
        // Format HTML for clipboard
        std::string formatted = formatHtmlForClipboard(html);
        
        if (!OpenClipboard(nullptr)) return false;
        
        EmptyClipboard();
        
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, formatted.size() + 1);
        if (hMem) {
            char* pMem = static_cast<char*>(GlobalLock(hMem));
            if (pMem) {
                memcpy(pMem, formatted.c_str(), formatted.size() + 1);
                GlobalUnlock(hMem);
                SetClipboardData(CF_HTML, hMem);
            }
        }
        
        CloseClipboard();
        return hMem != nullptr;
    }
    
    std::string getHtml() const override {
        static UINT CF_HTML = RegisterClipboardFormatW(L"HTML Format");
        if (!CF_HTML) return "";
        
        if (!OpenClipboard(nullptr)) return "";
        
        std::string result;
        HANDLE hData = GetClipboardData(CF_HTML);
        if (hData) {
            char* pData = static_cast<char*>(GlobalLock(hData));
            if (pData) {
                result = extractHtmlFromClipboard(pData);
                GlobalUnlock(hData);
            }
        }
        
        CloseClipboard();
        return result;
    }
    
    bool hasHtml() const override {
        static UINT CF_HTML = RegisterClipboardFormatW(L"HTML Format");
        return CF_HTML && IsClipboardFormatAvailable(CF_HTML) != 0;
    }
    
    bool setImage(const ClipboardImage& image) override {
        if (image.width <= 0 || image.height <= 0 || image.data.empty()) {
            return false;
        }
        
        if (!OpenClipboard(nullptr)) return false;
        
        EmptyClipboard();
        
        // Create DIB
        BITMAPINFOHEADER bi = {};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = image.width;
        bi.biHeight = -image.height;  // Top-down
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        
        size_t dataSize = image.width * image.height * 4;
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dataSize);
        
        if (hMem) {
            char* pMem = static_cast<char*>(GlobalLock(hMem));
            if (pMem) {
                memcpy(pMem, &bi, sizeof(BITMAPINFOHEADER));
                
                // Convert RGBA to BGRA
                uint8_t* dst = reinterpret_cast<uint8_t*>(pMem + sizeof(BITMAPINFOHEADER));
                const uint8_t* src = image.data.data();
                for (int i = 0; i < image.width * image.height; ++i) {
                    dst[i * 4 + 0] = src[i * 4 + 2];  // B
                    dst[i * 4 + 1] = src[i * 4 + 1];  // G
                    dst[i * 4 + 2] = src[i * 4 + 0];  // R
                    dst[i * 4 + 3] = src[i * 4 + 3];  // A
                }
                
                GlobalUnlock(hMem);
                SetClipboardData(CF_DIB, hMem);
            }
        }
        
        CloseClipboard();
        return hMem != nullptr;
    }
    
    ClipboardImage getImage() const override {
        ClipboardImage result;
        
        if (!OpenClipboard(nullptr)) return result;
        
        HANDLE hData = GetClipboardData(CF_DIB);
        if (hData) {
            BITMAPINFOHEADER* bi = static_cast<BITMAPINFOHEADER*>(GlobalLock(hData));
            if (bi) {
                result.width = bi->biWidth;
                result.height = std::abs(bi->biHeight);
                result.channels = 4;
                
                size_t dataSize = result.width * result.height * 4;
                result.data.resize(dataSize);
                
                const uint8_t* src = reinterpret_cast<const uint8_t*>(bi) + bi->biSize;
                uint8_t* dst = result.data.data();
                
                // Convert BGRA to RGBA
                bool topDown = bi->biHeight < 0;
                for (int y = 0; y < result.height; ++y) {
                    int srcY = topDown ? y : (result.height - 1 - y);
                    for (int x = 0; x < result.width; ++x) {
                        int srcIdx = (srcY * result.width + x) * 4;
                        int dstIdx = (y * result.width + x) * 4;
                        dst[dstIdx + 0] = src[srcIdx + 2];  // R
                        dst[dstIdx + 1] = src[srcIdx + 1];  // G
                        dst[dstIdx + 2] = src[srcIdx + 0];  // B
                        dst[dstIdx + 3] = src[srcIdx + 3];  // A
                    }
                }
                
                GlobalUnlock(hData);
            }
        }
        
        CloseClipboard();
        return result;
    }
    
    bool hasImage() const override {
        return IsClipboardFormatAvailable(CF_DIB) != 0 ||
               IsClipboardFormatAvailable(CF_BITMAP) != 0;
    }
    
    bool setFiles(const std::vector<std::string>& paths) override {
        if (paths.empty()) return false;
        
        // Calculate total size needed
        size_t totalSize = sizeof(DROPFILES);
        for (const auto& path : paths) {
            totalSize += (utf8ToWide(path).size() + 1) * sizeof(wchar_t);
        }
        totalSize += sizeof(wchar_t);  // Double null terminator
        
        if (!OpenClipboard(nullptr)) return false;
        
        EmptyClipboard();
        
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, totalSize);
        if (hMem) {
            char* pMem = static_cast<char*>(GlobalLock(hMem));
            if (pMem) {
                DROPFILES* df = reinterpret_cast<DROPFILES*>(pMem);
                df->pFiles = sizeof(DROPFILES);
                df->fWide = TRUE;
                
                wchar_t* pFiles = reinterpret_cast<wchar_t*>(pMem + sizeof(DROPFILES));
                for (const auto& path : paths) {
                    std::wstring widePath = utf8ToWide(path);
                    wmemcpy(pFiles, widePath.c_str(), widePath.size() + 1);
                    pFiles += widePath.size() + 1;
                }
                *pFiles = L'\0';  // Double null terminator
                
                GlobalUnlock(hMem);
                SetClipboardData(CF_HDROP, hMem);
            }
        }
        
        CloseClipboard();
        return hMem != nullptr;
    }
    
    std::vector<std::string> getFiles() const override {
        std::vector<std::string> result;
        
        if (!OpenClipboard(nullptr)) return result;
        
        HANDLE hData = GetClipboardData(CF_HDROP);
        if (hData) {
            HDROP hDrop = static_cast<HDROP>(hData);
            UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
            
            for (UINT i = 0; i < count; ++i) {
                UINT size = DragQueryFileW(hDrop, i, nullptr, 0) + 1;
                std::wstring path(size, L'\0');
                DragQueryFileW(hDrop, i, &path[0], size);
                path.resize(wcslen(path.c_str()));
                result.push_back(wideToUtf8(path));
            }
        }
        
        CloseClipboard();
        return result;
    }
    
    bool hasFiles() const override {
        return IsClipboardFormatAvailable(CF_HDROP) != 0;
    }
    
    bool setCustom(const std::string& format, const std::vector<uint8_t>& data) override {
        UINT cf = RegisterClipboardFormatW(utf8ToWide(format).c_str());
        if (!cf) return false;
        
        if (!OpenClipboard(nullptr)) return false;
        
        EmptyClipboard();
        
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, data.size());
        if (hMem) {
            void* pMem = GlobalLock(hMem);
            if (pMem) {
                memcpy(pMem, data.data(), data.size());
                GlobalUnlock(hMem);
                SetClipboardData(cf, hMem);
            }
        }
        
        CloseClipboard();
        return hMem != nullptr;
    }
    
    std::vector<uint8_t> getCustom(const std::string& format) const override {
        std::vector<uint8_t> result;
        
        UINT cf = RegisterClipboardFormatW(utf8ToWide(format).c_str());
        if (!cf) return result;
        
        if (!OpenClipboard(nullptr)) return result;
        
        HANDLE hData = GetClipboardData(cf);
        if (hData) {
            size_t size = GlobalSize(hData);
            void* pData = GlobalLock(hData);
            if (pData) {
                result.resize(size);
                memcpy(result.data(), pData, size);
                GlobalUnlock(hData);
            }
        }
        
        CloseClipboard();
        return result;
    }
    
    bool hasCustom(const std::string& format) const override {
        UINT cf = RegisterClipboardFormatW(utf8ToWide(format).c_str());
        return cf && IsClipboardFormatAvailable(cf) != 0;
    }
    
    void clear() override {
        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            CloseClipboard();
        }
    }
    
    std::vector<ClipboardFormat> getAvailableFormats() const override {
        std::vector<ClipboardFormat> formats;
        
        if (hasText()) formats.push_back(ClipboardFormat::Text);
        if (hasHtml()) formats.push_back(ClipboardFormat::Html);
        if (hasImage()) formats.push_back(ClipboardFormat::Image);
        if (hasFiles()) formats.push_back(ClipboardFormat::Files);
        
        return formats;
    }

private:
    std::string formatHtmlForClipboard(const std::string& html) {
        // CF_HTML format requires specific header
        std::string header = "Version:0.9\r\n"
                            "StartHTML:XXXXXXXX\r\n"
                            "EndHTML:XXXXXXXX\r\n"
                            "StartFragment:XXXXXXXX\r\n"
                            "EndFragment:XXXXXXXX\r\n";
        
        std::string prefix = "<html><body><!--StartFragment-->";
        std::string suffix = "<!--EndFragment--></body></html>";
        
        size_t startHtml = header.size();
        size_t startFragment = startHtml + prefix.size();
        size_t endFragment = startFragment + html.size();
        size_t endHtml = endFragment + suffix.size();
        
        char buf[9];
        snprintf(buf, sizeof(buf), "%08zu", startHtml);
        header.replace(header.find("XXXXXXXX"), 8, buf);
        snprintf(buf, sizeof(buf), "%08zu", endHtml);
        header.replace(header.find("XXXXXXXX"), 8, buf);
        snprintf(buf, sizeof(buf), "%08zu", startFragment);
        header.replace(header.find("XXXXXXXX"), 8, buf);
        snprintf(buf, sizeof(buf), "%08zu", endFragment);
        header.replace(header.find("XXXXXXXX"), 8, buf);
        
        return header + prefix + html + suffix;
    }
    
    std::string extractHtmlFromClipboard(const char* data) const {
        std::string str(data);
        
        // Find StartFragment and EndFragment
        size_t startPos = str.find("StartFragment:");
        size_t endPos = str.find("EndFragment:");
        
        if (startPos == std::string::npos || endPos == std::string::npos) {
            return "";
        }
        
        size_t startOffset = std::stoul(str.substr(startPos + 14, 8));
        size_t endOffset = std::stoul(str.substr(endPos + 12, 8));
        
        return str.substr(startOffset, endOffset - startOffset);
    }
};


// ============================================================================
// Windows Drag Source Implementation
// ============================================================================

// Forward declarations for OLE interfaces
class WindowsDataObject;
class WindowsDropSource;

/**
 * @brief OLE IDropSource implementation for drag operations
 */
class WindowsDropSource : public ::IDropSource {
public:
    WindowsDropSource() : m_refCount(1) {}
    
    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == IID_IUnknown || riid == IID_IDropSource) {
            *ppvObject = static_cast<IDropSource*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    
    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }
    
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }
    
    // IDropSource
    HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override {
        if (fEscapePressed) return DRAGDROP_S_CANCEL;
        if (!(grfKeyState & MK_LBUTTON)) return DRAGDROP_S_DROP;
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect) override {
        (void)dwEffect;
        return DRAGDROP_S_USEDEFAULTCURSORS;
    }
    
private:
    LONG m_refCount;
};

/**
 * @brief OLE IDataObject implementation for drag data
 */
class WindowsDataObject : public ::IDataObject {
public:
    WindowsDataObject(const DragData& data) : m_refCount(1), m_data(data) {}
    
    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == IID_IUnknown || riid == IID_IDataObject) {
            *ppvObject = static_cast<IDataObject*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    
    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }
    
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }
    
    // IDataObject
    HRESULT STDMETHODCALLTYPE GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium) override {
        if (!pformatetcIn || !pmedium) return E_INVALIDARG;
        
        pmedium->tymed = TYMED_NULL;
        pmedium->pUnkForRelease = nullptr;
        pmedium->hGlobal = nullptr;
        
        // Handle text data
        if (pformatetcIn->cfFormat == CF_UNICODETEXT && m_data.type == DragDataType::Text) {
            std::wstring wideText = utf8ToWide(m_data.text);
            size_t size = (wideText.size() + 1) * sizeof(wchar_t);
            
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
            if (!hMem) return E_OUTOFMEMORY;
            
            wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
            if (pMem) {
                memcpy(pMem, wideText.c_str(), size);
                GlobalUnlock(hMem);
            }
            
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = hMem;
            return S_OK;
        }
        
        // Handle file data
        if (pformatetcIn->cfFormat == CF_HDROP && m_data.type == DragDataType::Files) {
            size_t totalSize = sizeof(DROPFILES);
            for (const auto& path : m_data.files) {
                totalSize += (utf8ToWide(path).size() + 1) * sizeof(wchar_t);
            }
            totalSize += sizeof(wchar_t);  // Double null terminator
            
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, totalSize);
            if (!hMem) return E_OUTOFMEMORY;
            
            char* pMem = static_cast<char*>(GlobalLock(hMem));
            if (pMem) {
                DROPFILES* df = reinterpret_cast<DROPFILES*>(pMem);
                df->pFiles = sizeof(DROPFILES);
                df->fWide = TRUE;
                df->pt.x = 0;
                df->pt.y = 0;
                df->fNC = FALSE;
                
                wchar_t* pFiles = reinterpret_cast<wchar_t*>(pMem + sizeof(DROPFILES));
                for (const auto& path : m_data.files) {
                    std::wstring widePath = utf8ToWide(path);
                    wmemcpy(pFiles, widePath.c_str(), widePath.size() + 1);
                    pFiles += widePath.size() + 1;
                }
                *pFiles = L'\0';
                GlobalUnlock(hMem);
            }
            
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = hMem;
            return S_OK;
        }
        
        return DV_E_FORMATETC;
    }
    
    HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium) override {
        (void)pformatetc; (void)pmedium;
        return E_NOTIMPL;
    }
    
    HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* pformatetc) override {
        if (!pformatetc) return E_INVALIDARG;
        
        if (pformatetc->cfFormat == CF_UNICODETEXT && m_data.type == DragDataType::Text) {
            return S_OK;
        }
        if (pformatetc->cfFormat == CF_HDROP && m_data.type == DragDataType::Files) {
            return S_OK;
        }
        return DV_E_FORMATETC;
    }
    
    HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut) override {
        (void)pformatectIn;
        if (pformatetcOut) pformatetcOut->ptd = nullptr;
        return E_NOTIMPL;
    }
    
    HRESULT STDMETHODCALLTYPE SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease) override {
        (void)pformatetc; (void)pmedium; (void)fRelease;
        return E_NOTIMPL;
    }
    
    HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc) override {
        (void)dwDirection; (void)ppenumFormatEtc;
        return E_NOTIMPL;
    }
    
    HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) override {
        (void)pformatetc; (void)advf; (void)pAdvSink; (void)pdwConnection;
        return OLE_E_ADVISENOTSUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection) override {
        (void)dwConnection;
        return OLE_E_ADVISENOTSUPPORTED;
    }
    
    HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA** ppenumAdvise) override {
        (void)ppenumAdvise;
        return OLE_E_ADVISENOTSUPPORTED;
    }
    
private:
    LONG m_refCount;
    DragData m_data;
};

class WindowsDragSource : public IDragSource {
public:
    WindowsDragSource() {
        OleInitialize(nullptr);
    }
    
    ~WindowsDragSource() override {
        OleUninitialize();
    }
    
    DragOperation startDrag(const DragData& data, DragOperation allowedOps) override {
        // Create OLE data object and drop source
        WindowsDataObject* dataObject = new WindowsDataObject(data);
        WindowsDropSource* dropSource = new WindowsDropSource();
        
        // Convert allowed operations to DROPEFFECT
        DWORD dwOKEffects = 0;
        if ((static_cast<int>(allowedOps) & static_cast<int>(DragOperation::Copy)) != 0)
            dwOKEffects |= DROPEFFECT_COPY;
        if ((static_cast<int>(allowedOps) & static_cast<int>(DragOperation::Move)) != 0)
            dwOKEffects |= DROPEFFECT_MOVE;
        if ((static_cast<int>(allowedOps) & static_cast<int>(DragOperation::Link)) != 0)
            dwOKEffects |= DROPEFFECT_LINK;
        
        // Perform the drag operation
        DWORD dwEffect = 0;
        HRESULT hr = DoDragDrop(dataObject, dropSource, dwOKEffects, &dwEffect);
        
        // Release COM objects
        dataObject->Release();
        dropSource->Release();
        
        // Convert result back to DragOperation
        if (hr == DRAGDROP_S_DROP) {
            if (dwEffect & DROPEFFECT_COPY) return DragOperation::Copy;
            if (dwEffect & DROPEFFECT_MOVE) return DragOperation::Move;
            if (dwEffect & DROPEFFECT_LINK) return DragOperation::Link;
        }
        
        return DragOperation::None;
    }
};

// ============================================================================
// Windows Drop Target Implementation
// ============================================================================

/**
 * @brief OLE IDropTarget implementation for receiving drops
 */
class WindowsOleDropTarget : public ::IDropTarget {
public:
    WindowsOleDropTarget(KillerGK::IDropTarget* target) 
        : m_refCount(1), m_target(target), m_currentOp(DragOperation::None) {}
    
    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == IID_IUnknown || riid == IID_IDropTarget) {
            *ppvObject = static_cast<::IDropTarget*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    
    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&m_refCount);
    }
    
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }
    
    // IDropTarget
    HRESULT STDMETHODCALLTYPE DragEnter(::IDataObject* pDataObj, DWORD grfKeyState, 
                                        POINTL pt, DWORD* pdwEffect) override {
        if (!m_target || !pdwEffect) return E_INVALIDARG;
        
        // Extract drag data from IDataObject
        m_currentData = extractDragData(pDataObj);
        
        // Convert key state to operation
        DragOperation op = keyStateToOperation(grfKeyState, *pdwEffect);
        
        // Call target
        m_currentOp = m_target->onDragEnter(
            static_cast<float>(pt.x), static_cast<float>(pt.y), op, m_currentData);
        
        // Convert result
        *pdwEffect = operationToDropEffect(m_currentOp);
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override {
        if (!m_target || !pdwEffect) return E_INVALIDARG;
        
        DragOperation op = keyStateToOperation(grfKeyState, *pdwEffect);
        m_currentOp = m_target->onDragOver(
            static_cast<float>(pt.x), static_cast<float>(pt.y), op);
        
        *pdwEffect = operationToDropEffect(m_currentOp);
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE DragLeave() override {
        if (m_target) m_target->onDragLeave();
        m_currentData = DragData{};
        m_currentOp = DragOperation::None;
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE Drop(::IDataObject* pDataObj, DWORD grfKeyState, 
                                   POINTL pt, DWORD* pdwEffect) override {
        if (!m_target || !pdwEffect) return E_INVALIDARG;
        
        // Extract fresh data for the drop
        DragData data = extractDragData(pDataObj);
        
        DropEvent event;
        event.x = static_cast<float>(pt.x);
        event.y = static_cast<float>(pt.y);
        event.operation = keyStateToOperation(grfKeyState, *pdwEffect);
        event.data = data;
        
        bool accepted = m_target->onDrop(event);
        
        *pdwEffect = accepted ? operationToDropEffect(event.operation) : DROPEFFECT_NONE;
        
        m_currentData = DragData{};
        m_currentOp = DragOperation::None;
        
        return S_OK;
    }
    
private:
    LONG m_refCount;
    KillerGK::IDropTarget* m_target;
    DragData m_currentData;
    DragOperation m_currentOp;
    
    DragData extractDragData(::IDataObject* pDataObj) {
        DragData data;
        if (!pDataObj) return data;
        
        // Try to get text
        FORMATETC fmtText = { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stgText = {};
        if (SUCCEEDED(pDataObj->GetData(&fmtText, &stgText))) {
            if (stgText.hGlobal) {
                wchar_t* pText = static_cast<wchar_t*>(GlobalLock(stgText.hGlobal));
                if (pText) {
                    data.type = DragDataType::Text;
                    data.text = wideToUtf8(pText);
                    GlobalUnlock(stgText.hGlobal);
                }
            }
            ReleaseStgMedium(&stgText);
            return data;
        }
        
        // Try to get files
        FORMATETC fmtFiles = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stgFiles = {};
        if (SUCCEEDED(pDataObj->GetData(&fmtFiles, &stgFiles))) {
            if (stgFiles.hGlobal) {
                HDROP hDrop = static_cast<HDROP>(stgFiles.hGlobal);
                UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
                
                data.type = DragDataType::Files;
                for (UINT i = 0; i < count; ++i) {
                    UINT size = DragQueryFileW(hDrop, i, nullptr, 0) + 1;
                    std::wstring path(size, L'\0');
                    DragQueryFileW(hDrop, i, &path[0], size);
                    path.resize(wcslen(path.c_str()));
                    data.files.push_back(wideToUtf8(path));
                }
            }
            ReleaseStgMedium(&stgFiles);
            return data;
        }
        
        return data;
    }
    
    DragOperation keyStateToOperation(DWORD grfKeyState, DWORD allowedEffects) {
        // Determine operation based on key state
        if ((grfKeyState & MK_CONTROL) && (grfKeyState & MK_SHIFT)) {
            if (allowedEffects & DROPEFFECT_LINK) return DragOperation::Link;
        } else if (grfKeyState & MK_CONTROL) {
            if (allowedEffects & DROPEFFECT_COPY) return DragOperation::Copy;
        } else if (grfKeyState & MK_SHIFT) {
            if (allowedEffects & DROPEFFECT_MOVE) return DragOperation::Move;
        }
        
        // Default: prefer copy, then move, then link
        if (allowedEffects & DROPEFFECT_COPY) return DragOperation::Copy;
        if (allowedEffects & DROPEFFECT_MOVE) return DragOperation::Move;
        if (allowedEffects & DROPEFFECT_LINK) return DragOperation::Link;
        
        return DragOperation::None;
    }
    
    DWORD operationToDropEffect(DragOperation op) {
        switch (op) {
            case DragOperation::Copy: return DROPEFFECT_COPY;
            case DragOperation::Move: return DROPEFFECT_MOVE;
            case DragOperation::Link: return DROPEFFECT_LINK;
            default: return DROPEFFECT_NONE;
        }
    }
};

/**
 * @brief Windows drop target manager implementation
 */
class WindowsDropTargetManager : public IDropTargetManager {
public:
    WindowsDropTargetManager() {
        OleInitialize(nullptr);
    }
    
    ~WindowsDropTargetManager() override {
        // Unregister all targets
        for (auto& [hwnd, oleTarget] : m_targets) {
            RevokeDragDrop(static_cast<HWND>(hwnd));
            oleTarget->Release();
        }
        m_targets.clear();
        OleUninitialize();
    }
    
    bool registerDropTarget(void* windowHandle, KillerGK::IDropTarget* target) override {
        if (!windowHandle || !target) return false;
        
        HWND hwnd = static_cast<HWND>(windowHandle);
        
        // Unregister existing target if any
        unregisterDropTarget(windowHandle);
        
        // Create OLE drop target wrapper
        WindowsOleDropTarget* oleTarget = new WindowsOleDropTarget(target);
        
        // Register with OLE
        HRESULT hr = RegisterDragDrop(hwnd, oleTarget);
        if (FAILED(hr)) {
            oleTarget->Release();
            return false;
        }
        
        m_targets[windowHandle] = oleTarget;
        return true;
    }
    
    void unregisterDropTarget(void* windowHandle) override {
        auto it = m_targets.find(windowHandle);
        if (it != m_targets.end()) {
            RevokeDragDrop(static_cast<HWND>(windowHandle));
            it->second->Release();
            m_targets.erase(it);
        }
    }
    
    bool isRegistered(void* windowHandle) const override {
        return m_targets.find(windowHandle) != m_targets.end();
    }
    
private:
    std::unordered_map<void*, WindowsOleDropTarget*> m_targets;
};

// ============================================================================
// Windows Internal Drag Manager Implementation
// ============================================================================

/**
 * @brief Internal drag manager for widget-to-widget drag operations
 */
class WindowsInternalDragManager : public IInternalDragManager {
public:
    WindowsInternalDragManager() = default;
    ~WindowsInternalDragManager() override = default;
    
    bool startDrag(const DragData& data, void* sourceWidget) override {
        if (m_isDragging) return false;
        
        m_dragData = data;
        m_sourceWidget = sourceWidget;
        m_isDragging = true;
        m_currentTarget = nullptr;
        
        return true;
    }
    
    void cancelDrag() override {
        if (m_isDragging && m_currentTarget) {
            m_currentTarget->onDragLeave();
        }
        m_isDragging = false;
        m_dragData = DragData{};
        m_sourceWidget = nullptr;
        m_currentTarget = nullptr;
    }
    
    bool isDragging() const override {
        return m_isDragging;
    }
    
    const DragData* getDragData() const override {
        return m_isDragging ? &m_dragData : nullptr;
    }
    
    void updateDragPosition(float x, float y) override {
        if (!m_isDragging) return;
        
        // Find drop zone at position
        KillerGK::IDropTarget* newTarget = nullptr;
        for (const auto& [id, zone] : m_dropZones) {
            if (zone.bounds.contains(x, y)) {
                newTarget = zone.target;
                break;
            }
        }
        
        // Handle target changes
        if (newTarget != m_currentTarget) {
            if (m_currentTarget) {
                m_currentTarget->onDragLeave();
            }
            m_currentTarget = newTarget;
            if (m_currentTarget) {
                m_currentTarget->onDragEnter(x, y, DragOperation::Copy, m_dragData);
            }
        } else if (m_currentTarget) {
            m_currentTarget->onDragOver(x, y, DragOperation::Copy);
        }
    }
    
    DragOperation completeDrag(float x, float y) override {
        if (!m_isDragging) return DragOperation::None;
        
        DragOperation result = DragOperation::None;
        
        // Find drop zone at position
        for (const auto& [id, zone] : m_dropZones) {
            if (zone.bounds.contains(x, y) && zone.target) {
                DropEvent event;
                event.x = x;
                event.y = y;
                event.operation = DragOperation::Copy;
                event.data = m_dragData;
                
                if (zone.target->onDrop(event)) {
                    result = event.operation;
                }
                break;
            }
        }
        
        // Clean up
        m_isDragging = false;
        m_dragData = DragData{};
        m_sourceWidget = nullptr;
        m_currentTarget = nullptr;
        
        return result;
    }
    
    void registerDropZone(const std::string& id, const Rect& bounds, KillerGK::IDropTarget* target) override {
        m_dropZones[id] = DropZone{ bounds, target };
    }
    
    void unregisterDropZone(const std::string& id) override {
        m_dropZones.erase(id);
    }
    
    void updateDropZoneBounds(const std::string& id, const Rect& bounds) override {
        auto it = m_dropZones.find(id);
        if (it != m_dropZones.end()) {
            it->second.bounds = bounds;
        }
    }
    
private:
    struct DropZone {
        Rect bounds;
        KillerGK::IDropTarget* target = nullptr;
    };
    
    bool m_isDragging = false;
    DragData m_dragData;
    void* m_sourceWidget = nullptr;
    KillerGK::IDropTarget* m_currentTarget = nullptr;
    std::unordered_map<std::string, DropZone> m_dropZones;
};

// ============================================================================
// Windows System Info Implementation
// ============================================================================

class WindowsSystemInfo : public ISystemInfo {
public:
    WindowsSystemInfo() = default;
    ~WindowsSystemInfo() override = default;
    
    OSInfo getOSInfo() const override {
        OSInfo info;
        info.name = "Windows";
        
        // Get version using RtlGetVersion
        typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (hNtdll) {
            auto RtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
                GetProcAddress(hNtdll, "RtlGetVersion"));
            if (RtlGetVersion) {
                RTL_OSVERSIONINFOW osvi = {};
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                if (RtlGetVersion(&osvi) == 0) {
                    std::ostringstream oss;
                    oss << osvi.dwMajorVersion << "." << osvi.dwMinorVersion;
                    info.version = oss.str();
                    info.buildNumber = std::to_string(osvi.dwBuildNumber);
                }
            }
        }
        
        // Get edition
        DWORD productType;
        if (GetProductInfo(10, 0, 0, 0, &productType)) {
            switch (productType) {
                case PRODUCT_HOME_BASIC:
                case PRODUCT_HOME_PREMIUM:
                    info.edition = "Home";
                    break;
                case PRODUCT_PROFESSIONAL:
                    info.edition = "Pro";
                    break;
                case PRODUCT_ENTERPRISE:
                    info.edition = "Enterprise";
                    break;
                default:
                    info.edition = "Unknown";
            }
        }
        
        // Get architecture
        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);
        switch (si.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64:
                info.architecture = "x64";
                break;
            case PROCESSOR_ARCHITECTURE_ARM64:
                info.architecture = "ARM64";
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:
                info.architecture = "x86";
                break;
            default:
                info.architecture = "Unknown";
        }
        
        // Get hostname
        wchar_t hostname[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
        if (GetComputerNameW(hostname, &size)) {
            info.hostname = wideToUtf8(hostname);
        }
        
        // Get username
        wchar_t username[256];
        size = 256;
        if (GetUserNameW(username, &size)) {
            info.username = wideToUtf8(username);
        }
        
        // Get locale
        wchar_t locale[LOCALE_NAME_MAX_LENGTH];
        if (GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH)) {
            info.locale = wideToUtf8(locale);
        }
        
        // Get timezone
        TIME_ZONE_INFORMATION tzi;
        if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_INVALID) {
            info.timezone = wideToUtf8(tzi.StandardName);
        }
        
        return info;
    }
    
    CPUInfo getCPUInfo() const override {
        CPUInfo info;
        
        // Get CPU name using CPUID
        int cpuInfo[4] = {0};
        char brand[49] = {0};
        
        __cpuid(cpuInfo, 0x80000000);
        int nExIds = cpuInfo[0];
        
        if (nExIds >= 0x80000004) {
            __cpuid(cpuInfo, 0x80000002);
            memcpy(brand, cpuInfo, sizeof(cpuInfo));
            __cpuid(cpuInfo, 0x80000003);
            memcpy(brand + 16, cpuInfo, sizeof(cpuInfo));
            __cpuid(cpuInfo, 0x80000004);
            memcpy(brand + 32, cpuInfo, sizeof(cpuInfo));
            info.name = brand;
            
            // Trim whitespace
            size_t start = info.name.find_first_not_of(" ");
            if (start != std::string::npos) {
                info.name = info.name.substr(start);
            }
        }
        
        // Get vendor
        __cpuid(cpuInfo, 0);
        char vendor[13] = {0};
        memcpy(vendor, &cpuInfo[1], 4);
        memcpy(vendor + 4, &cpuInfo[3], 4);
        memcpy(vendor + 8, &cpuInfo[2], 4);
        info.vendor = vendor;
        
        // Get core/thread count
        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);
        info.threads = si.dwNumberOfProcessors;
        
        // Estimate cores (simplified)
        DWORD length = 0;
        GetLogicalProcessorInformation(nullptr, &length);
        std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(
            length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
        if (GetLogicalProcessorInformation(buffer.data(), &length)) {
            for (const auto& proc : buffer) {
                if (proc.Relationship == RelationProcessorCore) {
                    info.cores++;
                }
            }
        }
        
        // Get architecture
        switch (si.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64:
                info.architecture = "x64";
                break;
            case PROCESSOR_ARCHITECTURE_ARM64:
                info.architecture = "ARM64";
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:
                info.architecture = "x86";
                break;
            default:
                info.architecture = "Unknown";
        }
        
        return info;
    }
    
    MemoryInfo getMemoryInfo() const override {
        MemoryInfo info;
        
        MEMORYSTATUSEX ms;
        ms.dwLength = sizeof(ms);
        if (GlobalMemoryStatusEx(&ms)) {
            info.totalPhysical = ms.ullTotalPhys;
            info.availablePhysical = ms.ullAvailPhys;
            info.totalVirtual = ms.ullTotalVirtual;
            info.availableVirtual = ms.ullAvailVirtual;
            info.usagePercent = static_cast<float>(ms.dwMemoryLoad);
        }
        
        return info;
    }
    
    std::vector<GPUInfo> getGPUInfo() const override {
        std::vector<GPUInfo> gpus;
        
        IDXGIFactory* factory = nullptr;
        if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) {
            IDXGIAdapter* adapter = nullptr;
            for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
                DXGI_ADAPTER_DESC desc;
                if (SUCCEEDED(adapter->GetDesc(&desc))) {
                    GPUInfo info;
                    info.name = wideToUtf8(desc.Description);
                    info.dedicatedMemory = desc.DedicatedVideoMemory;
                    info.sharedMemory = desc.SharedSystemMemory;
                    
                    // Determine vendor
                    switch (desc.VendorId) {
                        case 0x10DE: info.vendor = "NVIDIA"; break;
                        case 0x1002: info.vendor = "AMD"; break;
                        case 0x8086: info.vendor = "Intel"; break;
                        default: info.vendor = "Unknown";
                    }
                    
                    info.supportsVulkan = true;  // Assume Vulkan support
                    gpus.push_back(info);
                }
                adapter->Release();
            }
            factory->Release();
        }
        
        return gpus;
    }
    
    std::vector<DisplayInfoEx> getDisplays() const override {
        std::vector<DisplayInfoEx> displays;
        
        EnumDisplayMonitors(nullptr, nullptr,
            [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
                auto* displays = reinterpret_cast<std::vector<DisplayInfoEx>*>(lParam);
                
                MONITORINFOEXW mi = {};
                mi.cbSize = sizeof(MONITORINFOEXW);
                GetMonitorInfoW(hMonitor, &mi);
                
                DisplayInfoEx info;
                info.name = wideToUtf8(mi.szDevice);
                info.width = mi.rcMonitor.right - mi.rcMonitor.left;
                info.height = mi.rcMonitor.bottom - mi.rcMonitor.top;
                info.positionX = mi.rcMonitor.left;
                info.positionY = mi.rcMonitor.top;
                info.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
                
                // Get DPI
                UINT dpiX, dpiY;
                if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
                    info.dpiScale = static_cast<float>(dpiX) / 96.0f;
                }
                
                // Get display settings
                DEVMODEW dm = {};
                dm.dmSize = sizeof(DEVMODEW);
                if (EnumDisplaySettingsW(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm)) {
                    info.refreshRate = dm.dmDisplayFrequency;
                    info.bitsPerPixel = dm.dmBitsPerPel;
                    
                    // Orientation
                    switch (dm.dmDisplayOrientation) {
                        case DMDO_DEFAULT: info.orientation = "Landscape"; break;
                        case DMDO_90: info.orientation = "Portrait"; break;
                        case DMDO_180: info.orientation = "Landscape (flipped)"; break;
                        case DMDO_270: info.orientation = "Portrait (flipped)"; break;
                    }
                }
                
                displays->push_back(info);
                return TRUE;
            },
            reinterpret_cast<LPARAM>(&displays));
        
        return displays;
    }
    
    DisplayInfoEx getPrimaryDisplay() const override {
        auto displays = getDisplays();
        for (const auto& display : displays) {
            if (display.isPrimary) return display;
        }
        return displays.empty() ? DisplayInfoEx{} : displays[0];
    }
    
    BatteryInfo getBatteryInfo() const override {
        BatteryInfo info;
        
        SYSTEM_POWER_STATUS sps;
        if (GetSystemPowerStatus(&sps)) {
            info.hasBattery = (sps.BatteryFlag != 128);  // 128 = no battery
            info.isCharging = (sps.ACLineStatus == 1);
            
            if (sps.BatteryLifePercent != 255) {
                info.chargePercent = static_cast<float>(sps.BatteryLifePercent);
            }
            
            if (sps.BatteryLifeTime != 0xFFFFFFFF) {
                info.remainingMinutes = sps.BatteryLifeTime / 60;
            }
            
            if (sps.BatteryFlag & 8) {
                info.status = "Charging";
            } else if (sps.BatteryFlag & 4) {
                info.status = "Critical";
            } else if (sps.BatteryFlag & 2) {
                info.status = "Low";
            } else if (sps.BatteryFlag & 1) {
                info.status = "High";
            } else {
                info.status = "Unknown";
            }
        }
        
        return info;
    }
    
    bool supportsFeature(const std::string& feature) const override {
        if (feature == "vulkan") return true;
        if (feature == "touch") return GetSystemMetrics(SM_DIGITIZER) != 0;
        if (feature == "transparency") return true;
        if (feature == "notifications") return true;
        if (feature == "systemTray") return true;
        if (feature == "fileDialogs") return true;
        if (feature == "clipboard") return true;
        if (feature == "dragDrop") return true;
        return false;
    }
    
    std::string getEnvironmentVariable(const std::string& name) const override {
        wchar_t buffer[32767];
        DWORD size = GetEnvironmentVariableW(utf8ToWide(name).c_str(), buffer, 32767);
        if (size > 0 && size < 32767) {
            return wideToUtf8(buffer);
        }
        return "";
    }
    
    bool setEnvironmentVariable(const std::string& name, const std::string& value) override {
        return SetEnvironmentVariableW(utf8ToWide(name).c_str(),
            utf8ToWide(value).c_str()) != 0;
    }
    
    std::string getHomeDirectory() const override {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PROFILE, nullptr, 0, path))) {
            return wideToUtf8(path);
        }
        return getEnvironmentVariable("USERPROFILE");
    }
    
    std::string getTempDirectory() const override {
        wchar_t path[MAX_PATH];
        DWORD size = GetTempPathW(MAX_PATH, path);
        if (size > 0 && size < MAX_PATH) {
            return wideToUtf8(path);
        }
        return "";
    }
    
    std::string getAppDataDirectory() const override {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
            return wideToUtf8(path);
        }
        return "";
    }
    
    std::string getDocumentsDirectory() const override {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, path))) {
            return wideToUtf8(path);
        }
        return "";
    }
    
    std::string getDesktopDirectory() const override {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, path))) {
            return wideToUtf8(path);
        }
        return "";
    }
};

// ============================================================================
// Factory Functions
// ============================================================================

std::unique_ptr<IFileDialog> createFileDialog() {
    return std::make_unique<WindowsFileDialog>();
}

std::unique_ptr<ISystemTray> createSystemTray() {
    return std::make_unique<WindowsSystemTray>();
}

std::unique_ptr<INotification> createNotificationManager() {
    return std::make_unique<WindowsNotification>();
}

std::unique_ptr<IClipboard> createClipboard() {
    return std::make_unique<WindowsClipboard>();
}

std::unique_ptr<IDragSource> createDragSource() {
    return std::make_unique<WindowsDragSource>();
}

std::unique_ptr<IDropTargetManager> createDropTargetManager() {
    return std::make_unique<WindowsDropTargetManager>();
}

std::unique_ptr<IInternalDragManager> createInternalDragManager() {
    return std::make_unique<WindowsInternalDragManager>();
}

std::unique_ptr<ISystemInfo> createSystemInfo() {
    return std::make_unique<WindowsSystemInfo>();
}

} // namespace KillerGK

#endif // _WIN32
