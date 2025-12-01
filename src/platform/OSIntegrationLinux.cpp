/**
 * @file OSIntegrationLinux.cpp
 * @brief Linux-specific OS integration implementation stub for KillerGK
 */

#if defined(__linux__) && !defined(__ANDROID__)

#include "KillerGK/platform/OSIntegration.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unordered_map>

namespace KillerGK {

// ============================================================================
// Linux File Dialog Implementation (Stub)
// ============================================================================

class LinuxFileDialog : public IFileDialog {
public:
    FileDialogResult showOpenDialog(const OpenFileDialogOptions& options) override {
        FileDialogResult result;
        // TODO: Implement using GTK or Qt file dialogs
        (void)options;
        return result;
    }
    
    FileDialogResult showSaveDialog(const SaveFileDialogOptions& options) override {
        FileDialogResult result;
        (void)options;
        return result;
    }
    
    FileDialogResult showFolderDialog(const FolderDialogOptions& options) override {
        FileDialogResult result;
        (void)options;
        return result;
    }
};

// ============================================================================
// Linux System Tray Implementation (Stub)
// ============================================================================

class LinuxSystemTray : public ISystemTray {
public:
    bool setIcon(const std::string& iconPath) override {
        m_iconPath = iconPath;
        return true;
    }
    
    void setTooltip(const std::string& tooltip) override {
        m_tooltip = tooltip;
    }
    
    void setMenu(const std::vector<TrayMenuItem>& items) override {
        m_menuItems = items;
    }
    
    void show() override { m_visible = true; }
    void hide() override { m_visible = false; }
    bool isVisible() const override { return m_visible; }
    
    void setOnClick(std::function<void()> callback) override {
        m_onClick = std::move(callback);
    }
    
    void setOnDoubleClick(std::function<void()> callback) override {
        m_onDoubleClick = std::move(callback);
    }

private:
    std::string m_iconPath;
    std::string m_tooltip;
    std::vector<TrayMenuItem> m_menuItems;
    bool m_visible = false;
    std::function<void()> m_onClick;
    std::function<void()> m_onDoubleClick;
};

// ============================================================================
// Linux Notification Implementation
// ============================================================================

/**
 * @brief Linux notification implementation using notify-send command
 * 
 * This implementation provides native Linux notifications with support for:
 * - Title and body text
 * - Custom icons
 * - Timeout configuration
 * - Action buttons (via notify-send --action on supported systems)
 * 
 * Requirements 14.3: Send native OS notifications, Support notification actions
 * 
 * Note: For full action support, libnotify would be needed. This implementation
 * uses notify-send for broad compatibility.
 */
class LinuxNotification : public INotification {
public:
    LinuxNotification() {
        // Check if notify-send is available
        m_supported = (system("which notify-send > /dev/null 2>&1") == 0);
    }
    
    std::string show(const NotificationOptions& options) override {
        std::string id = generateId();
        
        if (!m_supported) {
            // Store notification data even if we can't show it
            m_notifications[id] = options;
            return id;
        }
        
        // Build notify-send command
        std::string cmd = "notify-send";
        
        // Add urgency level
        cmd += " -u normal";
        
        // Add timeout (in milliseconds)
        if (options.timeoutMs > 0) {
            cmd += " -t " + std::to_string(options.timeoutMs);
        }
        
        // Add icon if specified
        if (!options.iconPath.empty()) {
            cmd += " -i \"" + escapeShellArg(options.iconPath) + "\"";
        }
        
        // Add actions if supported (notify-send 0.7.9+)
        // Note: Actions require a notification daemon that supports them
        for (const auto& action : options.actions) {
            cmd += " --action=\"" + escapeShellArg(action.id) + "=" + 
                   escapeShellArg(action.label) + "\"";
        }
        
        // Add title and body
        cmd += " \"" + escapeShellArg(options.title) + "\"";
        cmd += " \"" + escapeShellArg(options.body) + "\"";
        
        // Run in background to not block
        cmd += " &";
        
        // Execute the command
        int result = system(cmd.c_str());
        (void)result;  // Ignore result for now
        
        // Store notification data
        m_notifications[id] = options;
        
        // Call onClick immediately for simple notifications without actions
        // (Linux notify-send doesn't provide click callbacks without libnotify)
        
        return id;
    }
    
    void close(const std::string& id) override {
        auto it = m_notifications.find(id);
        if (it != m_notifications.end()) {
            // Call onClose callback
            if (it->second.onClose) {
                it->second.onClose();
            }
            m_notifications.erase(it);
        }
        // Note: notify-send doesn't support closing notifications programmatically
        // Would need gdbus or libnotify for that functionality
    }
    
    void closeAll() override {
        for (auto& [id, options] : m_notifications) {
            if (options.onClose) {
                options.onClose();
            }
        }
        m_notifications.clear();
    }
    
    bool isSupported() const override {
        return m_supported;
    }
    
    bool requestPermission() override {
        // No permission needed on Linux for desktop notifications
        return true;
    }

private:
    bool m_supported = false;
    std::unordered_map<std::string, NotificationOptions> m_notifications;
    uint64_t m_counter = 0;
    
    std::string generateId() {
        return "notif_" + std::to_string(++m_counter);
    }
    
    std::string escapeShellArg(const std::string& arg) {
        std::string result;
        for (char c : arg) {
            if (c == '"' || c == '\\' || c == '$' || c == '`') {
                result += '\\';
            }
            result += c;
        }
        return result;
    }
};

// ============================================================================
// Linux Clipboard Implementation
// ============================================================================

/**
 * @brief Linux clipboard implementation using xclip/xsel commands
 * 
 * This implementation provides native Linux clipboard access with support for:
 * - Text operations via xclip or xsel
 * - Image operations via xclip with PNG format
 * - HTML operations via xclip with text/html target
 * - Custom format storage (in-memory fallback)
 * 
 * Requirements 14.4: Read/write text, Read/write images, Support custom formats
 */
class LinuxClipboard : public IClipboard {
public:
    LinuxClipboard() {
        // Check which clipboard tool is available
        m_hasXclip = (system("which xclip > /dev/null 2>&1") == 0);
        m_hasXsel = (system("which xsel > /dev/null 2>&1") == 0);
    }
    
    bool setText(const std::string& text) override {
        if (m_hasXclip) {
            // Use xclip to set clipboard text
            FILE* pipe = popen("xclip -selection clipboard", "w");
            if (pipe) {
                fwrite(text.c_str(), 1, text.size(), pipe);
                int result = pclose(pipe);
                return result == 0;
            }
        } else if (m_hasXsel) {
            // Use xsel as fallback
            FILE* pipe = popen("xsel --clipboard --input", "w");
            if (pipe) {
                fwrite(text.c_str(), 1, text.size(), pipe);
                int result = pclose(pipe);
                return result == 0;
            }
        }
        // Fallback to in-memory storage
        m_text = text;
        return true;
    }
    
    std::string getText() const override {
        if (m_hasXclip) {
            FILE* pipe = popen("xclip -selection clipboard -o 2>/dev/null", "r");
            if (pipe) {
                std::string result;
                char buffer[4096];
                while (fgets(buffer, sizeof(buffer), pipe)) {
                    result += buffer;
                }
                pclose(pipe);
                return result;
            }
        } else if (m_hasXsel) {
            FILE* pipe = popen("xsel --clipboard --output 2>/dev/null", "r");
            if (pipe) {
                std::string result;
                char buffer[4096];
                while (fgets(buffer, sizeof(buffer), pipe)) {
                    result += buffer;
                }
                pclose(pipe);
                return result;
            }
        }
        return m_text;
    }
    
    bool hasText() const override {
        if (m_hasXclip) {
            // Check if clipboard has text content
            int result = system("xclip -selection clipboard -o > /dev/null 2>&1");
            return result == 0;
        } else if (m_hasXsel) {
            int result = system("xsel --clipboard --output > /dev/null 2>&1");
            return result == 0;
        }
        return !m_text.empty();
    }
    
    bool setHtml(const std::string& html) override {
        if (m_hasXclip) {
            // Use xclip with text/html target
            FILE* pipe = popen("xclip -selection clipboard -t text/html", "w");
            if (pipe) {
                fwrite(html.c_str(), 1, html.size(), pipe);
                int result = pclose(pipe);
                if (result == 0) return true;
            }
        }
        // Fallback to in-memory storage
        m_html = html;
        return true;
    }
    
    std::string getHtml() const override {
        if (m_hasXclip) {
            FILE* pipe = popen("xclip -selection clipboard -t text/html -o 2>/dev/null", "r");
            if (pipe) {
                std::string result;
                char buffer[4096];
                while (fgets(buffer, sizeof(buffer), pipe)) {
                    result += buffer;
                }
                int status = pclose(pipe);
                if (status == 0 && !result.empty()) {
                    return result;
                }
            }
        }
        return m_html;
    }
    
    bool hasHtml() const override {
        if (m_hasXclip) {
            int result = system("xclip -selection clipboard -t text/html -o > /dev/null 2>&1");
            return result == 0;
        }
        return !m_html.empty();
    }
    
    bool setImage(const ClipboardImage& image) override {
        if (image.width <= 0 || image.height <= 0 || image.data.empty()) {
            return false;
        }
        
        if (m_hasXclip) {
            // Write image as PNG to a temp file, then use xclip
            std::string tempFile = "/tmp/kgk_clipboard_" + std::to_string(getpid()) + ".png";
            
            // Write PNG file (simplified - just store raw for now)
            // In a full implementation, we'd use libpng or stb_image_write
            FILE* f = fopen(tempFile.c_str(), "wb");
            if (f) {
                // Write a simple PPM file that xclip can handle
                fprintf(f, "P6\n%d %d\n255\n", image.width, image.height);
                for (int i = 0; i < image.width * image.height; ++i) {
                    fputc(image.data[i * 4 + 0], f);  // R
                    fputc(image.data[i * 4 + 1], f);  // G
                    fputc(image.data[i * 4 + 2], f);  // B
                }
                fclose(f);
                
                std::string cmd = "xclip -selection clipboard -t image/x-portable-pixmap -i " + tempFile;
                int result = system(cmd.c_str());
                unlink(tempFile.c_str());
                if (result == 0) return true;
            }
        }
        
        // Fallback to in-memory storage
        m_image = image;
        return true;
    }
    
    ClipboardImage getImage() const override {
        // For now, return in-memory image
        // Full implementation would use xclip -t image/png -o
        return m_image;
    }
    
    bool hasImage() const override {
        if (m_hasXclip) {
            int result = system("xclip -selection clipboard -t image/png -o > /dev/null 2>&1");
            if (result == 0) return true;
        }
        return !m_image.data.empty();
    }
    
    bool setFiles(const std::vector<std::string>& paths) override {
        if (paths.empty()) return false;
        
        if (m_hasXclip) {
            // Format as URI list for file manager compatibility
            std::string uriList;
            for (const auto& path : paths) {
                uriList += "file://" + path + "\n";
            }
            
            FILE* pipe = popen("xclip -selection clipboard -t text/uri-list", "w");
            if (pipe) {
                fwrite(uriList.c_str(), 1, uriList.size(), pipe);
                int result = pclose(pipe);
                if (result == 0) return true;
            }
        }
        
        m_files = paths;
        return true;
    }
    
    std::vector<std::string> getFiles() const override {
        if (m_hasXclip) {
            FILE* pipe = popen("xclip -selection clipboard -t text/uri-list -o 2>/dev/null", "r");
            if (pipe) {
                std::vector<std::string> result;
                char buffer[4096];
                while (fgets(buffer, sizeof(buffer), pipe)) {
                    std::string line(buffer);
                    // Remove trailing newline
                    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
                        line.pop_back();
                    }
                    // Remove file:// prefix
                    if (line.find("file://") == 0) {
                        line = line.substr(7);
                    }
                    if (!line.empty()) {
                        result.push_back(line);
                    }
                }
                pclose(pipe);
                if (!result.empty()) return result;
            }
        }
        return m_files;
    }
    
    bool hasFiles() const override {
        if (m_hasXclip) {
            int result = system("xclip -selection clipboard -t text/uri-list -o > /dev/null 2>&1");
            return result == 0;
        }
        return !m_files.empty();
    }
    
    bool setCustom(const std::string& format, const std::vector<uint8_t>& data) override {
        if (m_hasXclip && !format.empty()) {
            // Try to set custom format via xclip
            std::string cmd = "xclip -selection clipboard -t " + escapeShellArg(format);
            FILE* pipe = popen(cmd.c_str(), "w");
            if (pipe) {
                fwrite(data.data(), 1, data.size(), pipe);
                int result = pclose(pipe);
                if (result == 0) {
                    m_customData[format] = data;
                    return true;
                }
            }
        }
        m_customData[format] = data;
        return true;
    }
    
    std::vector<uint8_t> getCustom(const std::string& format) const override {
        if (m_hasXclip && !format.empty()) {
            std::string cmd = "xclip -selection clipboard -t " + escapeShellArg(format) + " -o 2>/dev/null";
            FILE* pipe = popen(cmd.c_str(), "r");
            if (pipe) {
                std::vector<uint8_t> result;
                char buffer[4096];
                size_t bytesRead;
                while ((bytesRead = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
                    result.insert(result.end(), buffer, buffer + bytesRead);
                }
                pclose(pipe);
                if (!result.empty()) return result;
            }
        }
        auto it = m_customData.find(format);
        return it != m_customData.end() ? it->second : std::vector<uint8_t>{};
    }
    
    bool hasCustom(const std::string& format) const override {
        if (m_hasXclip && !format.empty()) {
            std::string cmd = "xclip -selection clipboard -t " + escapeShellArg(format) + " -o > /dev/null 2>&1";
            int result = system(cmd.c_str());
            return result == 0;
        }
        return m_customData.find(format) != m_customData.end();
    }
    
    void clear() override {
        if (m_hasXclip) {
            // Clear clipboard by setting empty content
            FILE* pipe = popen("xclip -selection clipboard", "w");
            if (pipe) {
                pclose(pipe);
            }
        } else if (m_hasXsel) {
            system("xsel --clipboard --clear");
        }
        m_text.clear();
        m_html.clear();
        m_image = ClipboardImage{};
        m_files.clear();
        m_customData.clear();
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
    bool m_hasXclip = false;
    bool m_hasXsel = false;
    std::string m_text;
    std::string m_html;
    ClipboardImage m_image;
    std::vector<std::string> m_files;
    mutable std::unordered_map<std::string, std::vector<uint8_t>> m_customData;
    
    static std::string escapeShellArg(const std::string& arg) {
        std::string result = "'";
        for (char c : arg) {
            if (c == '\'') {
                result += "'\\''";
            } else {
                result += c;
            }
        }
        result += "'";
        return result;
    }
};

// ============================================================================
// Linux Drag Source Implementation (Stub)
// ============================================================================

class LinuxDragSource : public IDragSource {
public:
    DragOperation startDrag(const DragData& data, DragOperation allowedOps) override {
        // TODO: Implement using GTK or X11 drag-drop
        (void)data;
        (void)allowedOps;
        return DragOperation::None;
    }
};

// ============================================================================
// Linux Drop Target Manager Implementation (Stub)
// ============================================================================

class LinuxDropTargetManager : public IDropTargetManager {
public:
    bool registerDropTarget(void* windowHandle, IDropTarget* target) override {
        // TODO: Implement using GTK or X11 drag-drop
        if (!windowHandle || !target) return false;
        m_targets[windowHandle] = target;
        return true;
    }
    
    void unregisterDropTarget(void* windowHandle) override {
        m_targets.erase(windowHandle);
    }
    
    bool isRegistered(void* windowHandle) const override {
        return m_targets.find(windowHandle) != m_targets.end();
    }
    
private:
    std::unordered_map<void*, IDropTarget*> m_targets;
};

// ============================================================================
// Linux Internal Drag Manager Implementation
// ============================================================================

class LinuxInternalDragManager : public IInternalDragManager {
public:
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
        IDropTarget* newTarget = nullptr;
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
    
    void registerDropZone(const std::string& id, const Rect& bounds, IDropTarget* target) override {
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
        IDropTarget* target = nullptr;
    };
    
    bool m_isDragging = false;
    DragData m_dragData;
    void* m_sourceWidget = nullptr;
    IDropTarget* m_currentTarget = nullptr;
    std::unordered_map<std::string, DropZone> m_dropZones;
};

// ============================================================================
// Linux System Info Implementation
// ============================================================================

class LinuxSystemInfo : public ISystemInfo {
public:
    OSInfo getOSInfo() const override {
        OSInfo info;
        info.name = "Linux";
        
        struct utsname uts;
        if (uname(&uts) == 0) {
            info.version = uts.release;
            info.architecture = uts.machine;
            info.hostname = uts.nodename;
        }
        
        // Try to get distribution info
        std::ifstream osRelease("/etc/os-release");
        if (osRelease.is_open()) {
            std::string line;
            while (std::getline(osRelease, line)) {
                if (line.find("PRETTY_NAME=") == 0) {
                    info.edition = line.substr(13);
                    if (!info.edition.empty() && info.edition.back() == '"') {
                        info.edition.pop_back();
                    }
                    break;
                }
            }
        }
        
        // Get username
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            info.username = pw->pw_name;
        }
        
        // Get locale
        const char* locale = std::getenv("LANG");
        if (locale) {
            info.locale = locale;
        }
        
        return info;
    }
    
    CPUInfo getCPUInfo() const override {
        CPUInfo info;
        
        std::ifstream cpuinfo("/proc/cpuinfo");
        if (cpuinfo.is_open()) {
            std::string line;
            while (std::getline(cpuinfo, line)) {
                if (line.find("model name") == 0) {
                    size_t pos = line.find(':');
                    if (pos != std::string::npos) {
                        info.name = line.substr(pos + 2);
                    }
                } else if (line.find("vendor_id") == 0) {
                    size_t pos = line.find(':');
                    if (pos != std::string::npos) {
                        info.vendor = line.substr(pos + 2);
                    }
                } else if (line.find("cpu cores") == 0) {
                    size_t pos = line.find(':');
                    if (pos != std::string::npos) {
                        info.cores = std::stoi(line.substr(pos + 2));
                    }
                } else if (line.find("siblings") == 0) {
                    size_t pos = line.find(':');
                    if (pos != std::string::npos) {
                        info.threads = std::stoi(line.substr(pos + 2));
                    }
                }
            }
        }
        
        struct utsname uts;
        if (uname(&uts) == 0) {
            info.architecture = uts.machine;
        }
        
        return info;
    }
    
    MemoryInfo getMemoryInfo() const override {
        MemoryInfo info;
        
        struct sysinfo si;
        if (sysinfo(&si) == 0) {
            info.totalPhysical = si.totalram * si.mem_unit;
            info.availablePhysical = si.freeram * si.mem_unit;
            info.totalVirtual = (si.totalram + si.totalswap) * si.mem_unit;
            info.availableVirtual = (si.freeram + si.freeswap) * si.mem_unit;
            info.usagePercent = 100.0f * (1.0f - (float)si.freeram / si.totalram);
        }
        
        return info;
    }
    
    std::vector<GPUInfo> getGPUInfo() const override {
        // TODO: Implement using libdrm or similar
        return {};
    }
    
    std::vector<DisplayInfoEx> getDisplays() const override {
        // TODO: Implement using X11/Wayland
        return {};
    }
    
    DisplayInfoEx getPrimaryDisplay() const override {
        auto displays = getDisplays();
        for (const auto& d : displays) {
            if (d.isPrimary) return d;
        }
        return displays.empty() ? DisplayInfoEx{} : displays[0];
    }
    
    BatteryInfo getBatteryInfo() const override {
        BatteryInfo info;
        
        std::ifstream status("/sys/class/power_supply/BAT0/status");
        if (status.is_open()) {
            info.hasBattery = true;
            std::string statusStr;
            std::getline(status, statusStr);
            info.status = statusStr;
            info.isCharging = (statusStr == "Charging");
        }
        
        std::ifstream capacity("/sys/class/power_supply/BAT0/capacity");
        if (capacity.is_open()) {
            int cap;
            capacity >> cap;
            info.chargePercent = static_cast<float>(cap);
        }
        
        return info;
    }
    
    bool supportsFeature(const std::string& feature) const override {
        if (feature == "vulkan") return true;
        return false;
    }
    
    std::string getEnvironmentVariable(const std::string& name) const override {
        const char* val = std::getenv(name.c_str());
        return val ? val : "";
    }
    
    bool setEnvironmentVariable(const std::string& name, const std::string& value) override {
        return setenv(name.c_str(), value.c_str(), 1) == 0;
    }
    
    std::string getHomeDirectory() const override {
        const char* home = std::getenv("HOME");
        if (home) return home;
        struct passwd* pw = getpwuid(getuid());
        return pw ? pw->pw_dir : "";
    }
    
    std::string getTempDirectory() const override {
        const char* tmp = std::getenv("TMPDIR");
        return tmp ? tmp : "/tmp";
    }
    
    std::string getAppDataDirectory() const override {
        const char* xdg = std::getenv("XDG_DATA_HOME");
        if (xdg) return xdg;
        return getHomeDirectory() + "/.local/share";
    }
    
    std::string getDocumentsDirectory() const override {
        return getHomeDirectory() + "/Documents";
    }
    
    std::string getDesktopDirectory() const override {
        return getHomeDirectory() + "/Desktop";
    }
};

// ============================================================================
// Factory Functions
// ============================================================================

std::unique_ptr<IFileDialog> createFileDialog() {
    return std::make_unique<LinuxFileDialog>();
}

std::unique_ptr<ISystemTray> createSystemTray() {
    return std::make_unique<LinuxSystemTray>();
}

std::unique_ptr<INotification> createNotificationManager() {
    return std::make_unique<LinuxNotification>();
}

std::unique_ptr<IClipboard> createClipboard() {
    return std::make_unique<LinuxClipboard>();
}

std::unique_ptr<IDragSource> createDragSource() {
    return std::make_unique<LinuxDragSource>();
}

std::unique_ptr<IDropTargetManager> createDropTargetManager() {
    return std::make_unique<LinuxDropTargetManager>();
}

std::unique_ptr<IInternalDragManager> createInternalDragManager() {
    return std::make_unique<LinuxInternalDragManager>();
}

std::unique_ptr<ISystemInfo> createSystemInfo() {
    return std::make_unique<LinuxSystemInfo>();
}

} // namespace KillerGK

#endif // __linux__
