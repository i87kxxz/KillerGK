/**
 * @file OSIntegration.hpp
 * @brief OS integration features for KillerGK
 * 
 * Provides native OS integration including file dialogs, system tray,
 * notifications, clipboard, drag-drop, and system information.
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>
#include <variant>

namespace KillerGK {

// ============================================================================
// File Dialogs (Requirements 14.1)
// ============================================================================

/**
 * @brief File filter for file dialogs
 */
struct FileFilter {
    std::string name;        // Display name (e.g., "Image Files")
    std::string extensions;  // Extensions (e.g., "*.png;*.jpg;*.gif")
    
    FileFilter() = default;
    FileFilter(const std::string& n, const std::string& ext) 
        : name(n), extensions(ext) {}
};

/**
 * @brief Options for file open dialog
 */
struct OpenFileDialogOptions {
    std::string title = "Open File";
    std::string defaultPath;
    std::vector<FileFilter> filters;
    bool allowMultiple = false;
    bool showHidden = false;
};

/**
 * @brief Options for file save dialog
 */
struct SaveFileDialogOptions {
    std::string title = "Save File";
    std::string defaultPath;
    std::string defaultName;
    std::vector<FileFilter> filters;
    bool confirmOverwrite = true;
};

/**
 * @brief Options for folder selection dialog
 */
struct FolderDialogOptions {
    std::string title = "Select Folder";
    std::string defaultPath;
};

/**
 * @brief Result from file dialogs
 */
struct FileDialogResult {
    bool success = false;
    std::vector<std::string> paths;  // Selected file(s) or folder
    int filterIndex = 0;             // Selected filter index
};

/**
 * @brief File dialog interface
 */
class IFileDialog {
public:
    virtual ~IFileDialog() = default;
    
    /**
     * @brief Show native open file dialog
     */
    virtual FileDialogResult showOpenDialog(const OpenFileDialogOptions& options) = 0;
    
    /**
     * @brief Show native save file dialog
     */
    virtual FileDialogResult showSaveDialog(const SaveFileDialogOptions& options) = 0;
    
    /**
     * @brief Show native folder selection dialog
     */
    virtual FileDialogResult showFolderDialog(const FolderDialogOptions& options) = 0;
};

// ============================================================================
// System Tray (Requirements 14.2)
// ============================================================================

/**
 * @brief Menu item for tray context menu
 */
struct TrayMenuItem {
    enum class Type { Normal, Separator, Submenu, Checkbox };
    
    Type type = Type::Normal;
    std::string id;
    std::string label;
    bool enabled = true;
    bool checked = false;
    std::string iconPath;
    std::vector<TrayMenuItem> submenu;
    std::function<void()> onClick;
    
    static TrayMenuItem separator() {
        TrayMenuItem item;
        item.type = Type::Separator;
        return item;
    }
};

/**
 * @brief System tray icon interface
 */
class ISystemTray {
public:
    virtual ~ISystemTray() = default;
    
    /**
     * @brief Set the tray icon
     */
    virtual bool setIcon(const std::string& iconPath) = 0;
    
    /**
     * @brief Set the tooltip text
     */
    virtual void setTooltip(const std::string& tooltip) = 0;
    
    /**
     * @brief Set the context menu
     */
    virtual void setMenu(const std::vector<TrayMenuItem>& items) = 0;
    
    /**
     * @brief Show the tray icon
     */
    virtual void show() = 0;
    
    /**
     * @brief Hide the tray icon
     */
    virtual void hide() = 0;
    
    /**
     * @brief Check if tray icon is visible
     */
    virtual bool isVisible() const = 0;
    
    /**
     * @brief Set callback for left click on tray icon
     */
    virtual void setOnClick(std::function<void()> callback) = 0;
    
    /**
     * @brief Set callback for double click on tray icon
     */
    virtual void setOnDoubleClick(std::function<void()> callback) = 0;
};

// ============================================================================
// Notifications (Requirements 14.3)
// ============================================================================

/**
 * @brief Notification action button
 */
struct NotificationAction {
    std::string id;
    std::string label;
};

/**
 * @brief Notification options
 */
struct NotificationOptions {
    std::string title;
    std::string body;
    std::string iconPath;
    std::string soundPath;
    bool silent = false;
    int timeoutMs = 5000;  // 0 = no timeout
    std::vector<NotificationAction> actions;
    std::function<void()> onClick;
    std::function<void(const std::string&)> onAction;
    std::function<void()> onClose;
};

/**
 * @brief Notification interface
 */
class INotification {
public:
    virtual ~INotification() = default;
    
    /**
     * @brief Show a notification
     * @return Notification ID for tracking
     */
    virtual std::string show(const NotificationOptions& options) = 0;
    
    /**
     * @brief Close a notification by ID
     */
    virtual void close(const std::string& id) = 0;
    
    /**
     * @brief Close all notifications
     */
    virtual void closeAll() = 0;
    
    /**
     * @brief Check if notifications are supported
     */
    virtual bool isSupported() const = 0;
    
    /**
     * @brief Request notification permission (if needed)
     */
    virtual bool requestPermission() = 0;
};

// ============================================================================
// Clipboard (Requirements 14.4)
// ============================================================================

/**
 * @brief Clipboard data format
 */
enum class ClipboardFormat {
    Text,
    Html,
    Rtf,
    Image,
    Files,
    Custom
};

/**
 * @brief Image data for clipboard
 */
struct ClipboardImage {
    int width = 0;
    int height = 0;
    int channels = 4;  // RGBA
    std::vector<uint8_t> data;
};

/**
 * @brief Clipboard data container
 */
struct ClipboardData {
    ClipboardFormat format = ClipboardFormat::Text;
    std::string text;
    std::string html;
    std::string rtf;
    ClipboardImage image;
    std::vector<std::string> files;
    std::string customFormat;
    std::vector<uint8_t> customData;
};

/**
 * @brief Extended clipboard interface
 */
class IClipboard {
public:
    virtual ~IClipboard() = default;
    
    // Text operations
    virtual bool setText(const std::string& text) = 0;
    virtual std::string getText() const = 0;
    virtual bool hasText() const = 0;
    
    // HTML operations
    virtual bool setHtml(const std::string& html) = 0;
    virtual std::string getHtml() const = 0;
    virtual bool hasHtml() const = 0;
    
    // Image operations
    virtual bool setImage(const ClipboardImage& image) = 0;
    virtual ClipboardImage getImage() const = 0;
    virtual bool hasImage() const = 0;
    
    // File operations
    virtual bool setFiles(const std::vector<std::string>& paths) = 0;
    virtual std::vector<std::string> getFiles() const = 0;
    virtual bool hasFiles() const = 0;
    
    // Custom format operations
    virtual bool setCustom(const std::string& format, const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> getCustom(const std::string& format) const = 0;
    virtual bool hasCustom(const std::string& format) const = 0;
    
    // General operations
    virtual void clear() = 0;
    virtual std::vector<ClipboardFormat> getAvailableFormats() const = 0;
};

// ============================================================================
// Drag and Drop (Requirements 11.5)
// ============================================================================

/**
 * @brief Drag operation type
 */
enum class DragOperation {
    None = 0,
    Copy = 1,
    Move = 2,
    Link = 4,
    All = Copy | Move | Link
};

inline DragOperation operator|(DragOperation a, DragOperation b) {
    return static_cast<DragOperation>(static_cast<int>(a) | static_cast<int>(b));
}

inline DragOperation operator&(DragOperation a, DragOperation b) {
    return static_cast<DragOperation>(static_cast<int>(a) & static_cast<int>(b));
}

/**
 * @brief Drag data types
 */
enum class DragDataType {
    Text,
    Html,
    Files,
    Image,
    Custom
};

/**
 * @brief Drag data container
 */
struct DragData {
    DragDataType type = DragDataType::Text;
    std::string text;
    std::string html;
    std::vector<std::string> files;
    ClipboardImage image;
    std::string customFormat;
    std::vector<uint8_t> customData;
};

/**
 * @brief Drop event data
 */
struct DropEvent {
    float x = 0.0f;
    float y = 0.0f;
    DragOperation operation = DragOperation::None;
    DragData data;
};

/**
 * @brief Drag source interface for initiating drags
 */
class IDragSource {
public:
    virtual ~IDragSource() = default;
    
    /**
     * @brief Start a drag operation
     * @return The operation that was performed
     */
    virtual DragOperation startDrag(const DragData& data, DragOperation allowedOps) = 0;
};

/**
 * @brief Drop target interface for receiving drops
 */
class IDropTarget {
public:
    virtual ~IDropTarget() = default;
    
    /**
     * @brief Called when drag enters the target
     */
    virtual DragOperation onDragEnter(float x, float y, DragOperation operation, const DragData& data) = 0;
    
    /**
     * @brief Called when drag moves over the target
     */
    virtual DragOperation onDragOver(float x, float y, DragOperation operation) = 0;
    
    /**
     * @brief Called when drag leaves the target
     */
    virtual void onDragLeave() = 0;
    
    /**
     * @brief Called when drop occurs
     */
    virtual bool onDrop(const DropEvent& event) = 0;
};

/**
 * @brief Callback-based drop target for easy integration
 */
class CallbackDropTarget : public IDropTarget {
public:
    using DragEnterCallback = std::function<DragOperation(float, float, DragOperation, const DragData&)>;
    using DragOverCallback = std::function<DragOperation(float, float, DragOperation)>;
    using DragLeaveCallback = std::function<void()>;
    using DropCallback = std::function<bool(const DropEvent&)>;
    
    CallbackDropTarget() = default;
    
    void setOnDragEnter(DragEnterCallback callback) { m_onDragEnter = std::move(callback); }
    void setOnDragOver(DragOverCallback callback) { m_onDragOver = std::move(callback); }
    void setOnDragLeave(DragLeaveCallback callback) { m_onDragLeave = std::move(callback); }
    void setOnDrop(DropCallback callback) { m_onDrop = std::move(callback); }
    
    DragOperation onDragEnter(float x, float y, DragOperation operation, const DragData& data) override {
        return m_onDragEnter ? m_onDragEnter(x, y, operation, data) : DragOperation::None;
    }
    
    DragOperation onDragOver(float x, float y, DragOperation operation) override {
        return m_onDragOver ? m_onDragOver(x, y, operation) : DragOperation::None;
    }
    
    void onDragLeave() override {
        if (m_onDragLeave) m_onDragLeave();
    }
    
    bool onDrop(const DropEvent& event) override {
        return m_onDrop ? m_onDrop(event) : false;
    }
    
private:
    DragEnterCallback m_onDragEnter;
    DragOverCallback m_onDragOver;
    DragLeaveCallback m_onDragLeave;
    DropCallback m_onDrop;
};

/**
 * @brief Drop target manager for registering windows as drop targets
 * 
 * Manages the registration of windows as OLE drop targets on Windows,
 * or equivalent functionality on other platforms.
 */
class IDropTargetManager {
public:
    virtual ~IDropTargetManager() = default;
    
    /**
     * @brief Register a window as a drop target
     * @param windowHandle Native window handle (HWND on Windows)
     * @param target Drop target implementation
     * @return true if registration succeeded
     */
    virtual bool registerDropTarget(void* windowHandle, IDropTarget* target) = 0;
    
    /**
     * @brief Unregister a window as a drop target
     * @param windowHandle Native window handle
     */
    virtual void unregisterDropTarget(void* windowHandle) = 0;
    
    /**
     * @brief Check if a window is registered as a drop target
     * @param windowHandle Native window handle
     * @return true if window is registered
     */
    virtual bool isRegistered(void* windowHandle) const = 0;
};

/**
 * @brief Internal drag manager for drag operations within the application
 * 
 * Handles drag-drop operations between widgets within the same application,
 * without involving the system drag-drop mechanism.
 */
class IInternalDragManager {
public:
    virtual ~IInternalDragManager() = default;
    
    /**
     * @brief Start an internal drag operation
     * @param data Data being dragged
     * @param sourceWidget Pointer to source widget (can be nullptr)
     * @return true if drag started successfully
     */
    virtual bool startDrag(const DragData& data, void* sourceWidget = nullptr) = 0;
    
    /**
     * @brief Cancel the current drag operation
     */
    virtual void cancelDrag() = 0;
    
    /**
     * @brief Check if a drag operation is in progress
     */
    virtual bool isDragging() const = 0;
    
    /**
     * @brief Get the current drag data
     */
    virtual const DragData* getDragData() const = 0;
    
    /**
     * @brief Update drag position (called during mouse move)
     * @param x Current mouse X position
     * @param y Current mouse Y position
     */
    virtual void updateDragPosition(float x, float y) = 0;
    
    /**
     * @brief Complete the drag operation at current position
     * @param x Drop X position
     * @param y Drop Y position
     * @return The operation that was performed
     */
    virtual DragOperation completeDrag(float x, float y) = 0;
    
    /**
     * @brief Register a drop zone for internal drag operations
     * @param id Unique identifier for the zone
     * @param bounds Bounding rectangle of the zone
     * @param target Drop target for the zone
     */
    virtual void registerDropZone(const std::string& id, const Rect& bounds, IDropTarget* target) = 0;
    
    /**
     * @brief Unregister a drop zone
     * @param id Zone identifier
     */
    virtual void unregisterDropZone(const std::string& id) = 0;
    
    /**
     * @brief Update bounds of a drop zone
     * @param id Zone identifier
     * @param bounds New bounding rectangle
     */
    virtual void updateDropZoneBounds(const std::string& id, const Rect& bounds) = 0;
};

// ============================================================================
// System Information (Requirements 14.5)
// ============================================================================

/**
 * @brief CPU information
 */
struct CPUInfo {
    std::string name;
    std::string vendor;
    int cores = 0;
    int threads = 0;
    int64_t frequencyHz = 0;
    std::string architecture;  // x86, x64, ARM, ARM64
};

/**
 * @brief Memory information
 */
struct MemoryInfo {
    uint64_t totalPhysical = 0;
    uint64_t availablePhysical = 0;
    uint64_t totalVirtual = 0;
    uint64_t availableVirtual = 0;
    float usagePercent = 0.0f;
};

/**
 * @brief GPU information
 */
struct GPUInfo {
    std::string name;
    std::string vendor;
    std::string driverVersion;
    uint64_t dedicatedMemory = 0;
    uint64_t sharedMemory = 0;
    bool supportsVulkan = false;
};

/**
 * @brief Display information (extended)
 */
struct DisplayInfoEx {
    std::string name;
    std::string deviceId;
    int width = 0;
    int height = 0;
    int bitsPerPixel = 0;
    int refreshRate = 0;
    float dpiScale = 1.0f;
    bool isPrimary = false;
    int positionX = 0;
    int positionY = 0;
    std::string orientation;  // Landscape, Portrait, etc.
};

/**
 * @brief Operating system information
 */
struct OSInfo {
    std::string name;           // Windows, Linux, macOS
    std::string version;        // 10.0.19041
    std::string buildNumber;
    std::string edition;        // Home, Pro, Enterprise
    std::string architecture;   // x64, ARM64
    std::string hostname;
    std::string username;
    std::string locale;
    std::string timezone;
};

/**
 * @brief Battery information
 */
struct BatteryInfo {
    bool hasBattery = false;
    bool isCharging = false;
    float chargePercent = 0.0f;
    int remainingMinutes = -1;  // -1 = unknown
    std::string status;         // Charging, Discharging, Full, etc.
};

/**
 * @brief System information interface
 */
class ISystemInfo {
public:
    virtual ~ISystemInfo() = default;
    
    // OS information
    virtual OSInfo getOSInfo() const = 0;
    
    // Hardware information
    virtual CPUInfo getCPUInfo() const = 0;
    virtual MemoryInfo getMemoryInfo() const = 0;
    virtual std::vector<GPUInfo> getGPUInfo() const = 0;
    
    // Display information
    virtual std::vector<DisplayInfoEx> getDisplays() const = 0;
    virtual DisplayInfoEx getPrimaryDisplay() const = 0;
    
    // Battery information
    virtual BatteryInfo getBatteryInfo() const = 0;
    
    // Feature detection
    virtual bool supportsFeature(const std::string& feature) const = 0;
    
    // Environment
    virtual std::string getEnvironmentVariable(const std::string& name) const = 0;
    virtual bool setEnvironmentVariable(const std::string& name, const std::string& value) = 0;
    
    // Paths
    virtual std::string getHomeDirectory() const = 0;
    virtual std::string getTempDirectory() const = 0;
    virtual std::string getAppDataDirectory() const = 0;
    virtual std::string getDocumentsDirectory() const = 0;
    virtual std::string getDesktopDirectory() const = 0;
};

// ============================================================================
// Factory Functions
// ============================================================================

/**
 * @brief Create a file dialog instance
 */
std::unique_ptr<IFileDialog> createFileDialog();

/**
 * @brief Create a system tray instance
 */
std::unique_ptr<ISystemTray> createSystemTray();

/**
 * @brief Create a notification manager instance
 */
std::unique_ptr<INotification> createNotificationManager();

/**
 * @brief Create a clipboard instance
 */
std::unique_ptr<IClipboard> createClipboard();

/**
 * @brief Create a drag source instance
 */
std::unique_ptr<IDragSource> createDragSource();

/**
 * @brief Create a drop target manager instance
 */
std::unique_ptr<IDropTargetManager> createDropTargetManager();

/**
 * @brief Create an internal drag manager instance
 */
std::unique_ptr<IInternalDragManager> createInternalDragManager();

/**
 * @brief Create a system info instance
 */
std::unique_ptr<ISystemInfo> createSystemInfo();

} // namespace KillerGK
