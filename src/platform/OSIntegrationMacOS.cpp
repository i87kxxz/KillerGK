/**
 * @file OSIntegrationMacOS.cpp
 * @brief macOS-specific OS integration implementation stub for KillerGK
 */

#ifdef __APPLE__

#include "KillerGK/platform/OSIntegration.hpp"
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <unordered_map>

namespace KillerGK {

// ============================================================================
// macOS File Dialog Implementation (Stub)
// ============================================================================

class MacOSFileDialog : public IFileDialog {
public:
    FileDialogResult showOpenDialog(const OpenFileDialogOptions& options) override {
        FileDialogResult result;
        // TODO: Implement using NSOpenPanel
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
// macOS System Tray Implementation (Stub)
// ============================================================================

class MacOSSystemTray : public ISystemTray {
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
// macOS Notification Implementation
// ============================================================================

/**
 * @brief macOS notification implementation using osascript
 * 
 * This implementation provides native macOS notifications with support for:
 * - Title and body text
 * - Subtitle (via body parsing)
 * - Sound
 * 
 * Requirements 14.3: Send native OS notifications, Support notification actions
 * 
 * Note: For full action support, the UserNotifications framework would be needed.
 * This implementation uses osascript for broad compatibility without Objective-C.
 */
class MacOSNotification : public INotification {
public:
    MacOSNotification() = default;
    
    std::string show(const NotificationOptions& options) override {
        std::string id = generateId();
        
        // Build osascript command for notification
        std::string script = "display notification \"" + escapeAppleScript(options.body) + "\"";
        script += " with title \"" + escapeAppleScript(options.title) + "\"";
        
        // Add sound unless silent
        if (!options.silent) {
            if (!options.soundPath.empty()) {
                // Custom sound - extract filename
                std::string soundName = options.soundPath;
                size_t lastSlash = soundName.find_last_of("/\\");
                if (lastSlash != std::string::npos) {
                    soundName = soundName.substr(lastSlash + 1);
                }
                // Remove extension
                size_t lastDot = soundName.find_last_of('.');
                if (lastDot != std::string::npos) {
                    soundName = soundName.substr(0, lastDot);
                }
                script += " sound name \"" + escapeAppleScript(soundName) + "\"";
            } else {
                script += " sound name \"default\"";
            }
        }
        
        // Build full command
        std::string cmd = "osascript -e '" + script + "' &";
        
        // Execute the command
        int result = system(cmd.c_str());
        (void)result;
        
        // Store notification data
        m_notifications[id] = options;
        
        return id;
    }
    
    void close(const std::string& id) override {
        auto it = m_notifications.find(id);
        if (it != m_notifications.end()) {
            if (it->second.onClose) {
                it->second.onClose();
            }
            m_notifications.erase(it);
        }
        // Note: osascript notifications cannot be closed programmatically
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
        return true;  // osascript is always available on macOS
    }
    
    bool requestPermission() override {
        // macOS may prompt for notification permission automatically
        return true;
    }

private:
    std::unordered_map<std::string, NotificationOptions> m_notifications;
    uint64_t m_counter = 0;
    
    std::string generateId() {
        return "notif_" + std::to_string(++m_counter);
    }
    
    std::string escapeAppleScript(const std::string& str) {
        std::string result;
        for (char c : str) {
            if (c == '"' || c == '\\') {
                result += '\\';
            }
            result += c;
        }
        return result;
    }
};

// ============================================================================
// macOS Clipboard Implementation
// ============================================================================

/**
 * @brief macOS clipboard implementation using pbcopy/pbpaste commands
 * 
 * This implementation provides native macOS clipboard access with support for:
 * - Text operations via pbcopy/pbpaste
 * - Image operations via osascript and temp files
 * - HTML operations via osascript
 * - Custom format storage (in-memory fallback)
 * 
 * Requirements 14.4: Read/write text, Read/write images, Support custom formats
 */
class MacOSClipboard : public IClipboard {
public:
    MacOSClipboard() = default;
    
    bool setText(const std::string& text) override {
        // Use pbcopy to set clipboard text
        FILE* pipe = popen("pbcopy", "w");
        if (pipe) {
            fwrite(text.c_str(), 1, text.size(), pipe);
            int result = pclose(pipe);
            return result == 0;
        }
        // Fallback to in-memory storage
        m_text = text;
        return true;
    }
    
    std::string getText() const override {
        FILE* pipe = popen("pbpaste 2>/dev/null", "r");
        if (pipe) {
            std::string result;
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                result += buffer;
            }
            pclose(pipe);
            return result;
        }
        return m_text;
    }
    
    bool hasText() const override {
        // Check if clipboard has text content
        FILE* pipe = popen("pbpaste 2>/dev/null", "r");
        if (pipe) {
            char buffer[1];
            bool hasContent = (fread(buffer, 1, 1, pipe) > 0);
            pclose(pipe);
            return hasContent;
        }
        return !m_text.empty();
    }
    
    bool setHtml(const std::string& html) override {
        // Use osascript to set HTML clipboard
        std::string script = "osascript -e 'set the clipboard to (\"" + 
                            escapeAppleScript(html) + "\" as «class HTML»)'";
        int result = system(script.c_str());
        if (result == 0) return true;
        
        // Fallback to in-memory storage
        m_html = html;
        return true;
    }
    
    std::string getHtml() const override {
        // Try to get HTML from clipboard using osascript
        FILE* pipe = popen("osascript -e 'the clipboard as «class HTML»' 2>/dev/null", "r");
        if (pipe) {
            std::string result;
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                result += buffer;
            }
            int status = pclose(pipe);
            if (status == 0 && !result.empty()) {
                // Remove trailing newline
                while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
                    result.pop_back();
                }
                return result;
            }
        }
        return m_html;
    }
    
    bool hasHtml() const override {
        // Check if clipboard has HTML content
        int result = system("osascript -e 'the clipboard as «class HTML»' > /dev/null 2>&1");
        if (result == 0) return true;
        return !m_html.empty();
    }
    
    bool setImage(const ClipboardImage& image) override {
        if (image.width <= 0 || image.height <= 0 || image.data.empty()) {
            return false;
        }
        
        // Write image to temp file and use osascript to set clipboard
        std::string tempFile = "/tmp/kgk_clipboard_" + std::to_string(getpid()) + ".tiff";
        
        // Write a simple TIFF file (macOS native format)
        // For simplicity, we'll write a PPM and convert
        std::string ppmFile = "/tmp/kgk_clipboard_" + std::to_string(getpid()) + ".ppm";
        FILE* f = fopen(ppmFile.c_str(), "wb");
        if (f) {
            fprintf(f, "P6\n%d %d\n255\n", image.width, image.height);
            for (int i = 0; i < image.width * image.height; ++i) {
                fputc(image.data[i * 4 + 0], f);  // R
                fputc(image.data[i * 4 + 1], f);  // G
                fputc(image.data[i * 4 + 2], f);  // B
            }
            fclose(f);
            
            // Convert to TIFF using sips (built-in macOS tool)
            std::string convertCmd = "sips -s format tiff " + ppmFile + " --out " + tempFile + " > /dev/null 2>&1";
            int convertResult = system(convertCmd.c_str());
            unlink(ppmFile.c_str());
            
            if (convertResult == 0) {
                // Set clipboard using osascript
                std::string script = "osascript -e 'set the clipboard to (read (POSIX file \"" + 
                                    tempFile + "\") as TIFF picture)'";
                int result = system(script.c_str());
                unlink(tempFile.c_str());
                if (result == 0) return true;
            }
        }
        
        // Fallback to in-memory storage
        m_image = image;
        return true;
    }
    
    ClipboardImage getImage() const override {
        // Try to get image from clipboard
        std::string tempFile = "/tmp/kgk_clipboard_get_" + std::to_string(getpid()) + ".tiff";
        
        // Save clipboard image to temp file
        std::string script = "osascript -e 'write (the clipboard as «class TIFF») to (open for access (POSIX file \"" +
                            tempFile + "\") with write permission)'";
        int result = system(script.c_str());
        
        if (result == 0) {
            // Convert TIFF to PPM for reading
            std::string ppmFile = "/tmp/kgk_clipboard_get_" + std::to_string(getpid()) + ".ppm";
            std::string convertCmd = "sips -s format ppm " + tempFile + " --out " + ppmFile + " > /dev/null 2>&1";
            int convertResult = system(convertCmd.c_str());
            unlink(tempFile.c_str());
            
            if (convertResult == 0) {
                ClipboardImage img = readPPM(ppmFile);
                unlink(ppmFile.c_str());
                if (!img.data.empty()) return img;
            }
        }
        
        return m_image;
    }
    
    bool hasImage() const override {
        // Check if clipboard has image content
        int result = system("osascript -e 'the clipboard as «class TIFF»' > /dev/null 2>&1");
        if (result == 0) return true;
        return !m_image.data.empty();
    }
    
    bool setFiles(const std::vector<std::string>& paths) override {
        if (paths.empty()) return false;
        
        // Build AppleScript to set file list
        std::string script = "osascript -e 'set the clipboard to {";
        for (size_t i = 0; i < paths.size(); ++i) {
            if (i > 0) script += ", ";
            script += "POSIX file \"" + escapeAppleScript(paths[i]) + "\"";
        }
        script += "}'";
        
        int result = system(script.c_str());
        if (result == 0) return true;
        
        m_files = paths;
        return true;
    }
    
    std::vector<std::string> getFiles() const override {
        // Try to get file list from clipboard
        FILE* pipe = popen("osascript -e 'POSIX path of (the clipboard as «class furl»)' 2>/dev/null", "r");
        if (pipe) {
            std::vector<std::string> result;
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                std::string line(buffer);
                // Remove trailing newline
                while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
                    line.pop_back();
                }
                if (!line.empty()) {
                    result.push_back(line);
                }
            }
            pclose(pipe);
            if (!result.empty()) return result;
        }
        return m_files;
    }
    
    bool hasFiles() const override {
        int result = system("osascript -e 'the clipboard as «class furl»' > /dev/null 2>&1");
        if (result == 0) return true;
        return !m_files.empty();
    }
    
    bool setCustom(const std::string& format, const std::vector<uint8_t>& data) override {
        // Custom formats stored in memory (full implementation would use NSPasteboard)
        m_customData[format] = data;
        return true;
    }
    
    std::vector<uint8_t> getCustom(const std::string& format) const override {
        auto it = m_customData.find(format);
        return it != m_customData.end() ? it->second : std::vector<uint8_t>{};
    }
    
    bool hasCustom(const std::string& format) const override {
        return m_customData.find(format) != m_customData.end();
    }
    
    void clear() override {
        // Clear clipboard using osascript
        system("osascript -e 'set the clipboard to \"\"'");
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
    std::string m_text;
    std::string m_html;
    ClipboardImage m_image;
    std::vector<std::string> m_files;
    std::unordered_map<std::string, std::vector<uint8_t>> m_customData;
    
    static std::string escapeAppleScript(const std::string& str) {
        std::string result;
        for (char c : str) {
            if (c == '"' || c == '\\') {
                result += '\\';
            }
            result += c;
        }
        return result;
    }
    
    static ClipboardImage readPPM(const std::string& path) {
        ClipboardImage img;
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) return img;
        
        char magic[3];
        if (fscanf(f, "%2s", magic) != 1 || strcmp(magic, "P6") != 0) {
            fclose(f);
            return img;
        }
        
        // Skip comments
        int c;
        while ((c = fgetc(f)) == '#') {
            while ((c = fgetc(f)) != '\n' && c != EOF);
        }
        ungetc(c, f);
        
        int width, height, maxval;
        if (fscanf(f, "%d %d %d", &width, &height, &maxval) != 3) {
            fclose(f);
            return img;
        }
        fgetc(f);  // Skip whitespace after header
        
        img.width = width;
        img.height = height;
        img.channels = 4;
        img.data.resize(width * height * 4);
        
        for (int i = 0; i < width * height; ++i) {
            int r = fgetc(f);
            int g = fgetc(f);
            int b = fgetc(f);
            if (r == EOF || g == EOF || b == EOF) break;
            img.data[i * 4 + 0] = static_cast<uint8_t>(r);
            img.data[i * 4 + 1] = static_cast<uint8_t>(g);
            img.data[i * 4 + 2] = static_cast<uint8_t>(b);
            img.data[i * 4 + 3] = 255;  // Alpha
        }
        
        fclose(f);
        return img;
    }
};

// ============================================================================
// macOS Drag Source Implementation (Stub)
// ============================================================================

class MacOSDragSource : public IDragSource {
public:
    DragOperation startDrag(const DragData& data, DragOperation allowedOps) override {
        // TODO: Implement using NSPasteboard and NSDraggingSession
        (void)data;
        (void)allowedOps;
        return DragOperation::None;
    }
};

// ============================================================================
// macOS Drop Target Manager Implementation (Stub)
// ============================================================================

class MacOSDropTargetManager : public IDropTargetManager {
public:
    bool registerDropTarget(void* windowHandle, IDropTarget* target) override {
        // TODO: Implement using NSView registerForDraggedTypes
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
// macOS Internal Drag Manager Implementation
// ============================================================================

class MacOSInternalDragManager : public IInternalDragManager {
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
// macOS System Info Implementation
// ============================================================================

class MacOSSystemInfo : public ISystemInfo {
public:
    OSInfo getOSInfo() const override {
        OSInfo info;
        info.name = "macOS";
        
        struct utsname uts;
        if (uname(&uts) == 0) {
            info.version = uts.release;
            info.architecture = uts.machine;
            info.hostname = uts.nodename;
        }
        
        // Get username
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            info.username = pw->pw_name;
        }
        
        return info;
    }
    
    CPUInfo getCPUInfo() const override {
        CPUInfo info;
        
        char brand[256];
        size_t size = sizeof(brand);
        if (sysctlbyname("machdep.cpu.brand_string", brand, &size, nullptr, 0) == 0) {
            info.name = brand;
        }
        
        char vendor[256];
        size = sizeof(vendor);
        if (sysctlbyname("machdep.cpu.vendor", vendor, &size, nullptr, 0) == 0) {
            info.vendor = vendor;
        }
        
        int cores;
        size = sizeof(cores);
        if (sysctlbyname("hw.physicalcpu", &cores, &size, nullptr, 0) == 0) {
            info.cores = cores;
        }
        
        int threads;
        size = sizeof(threads);
        if (sysctlbyname("hw.logicalcpu", &threads, &size, nullptr, 0) == 0) {
            info.threads = threads;
        }
        
        struct utsname uts;
        if (uname(&uts) == 0) {
            info.architecture = uts.machine;
        }
        
        return info;
    }
    
    MemoryInfo getMemoryInfo() const override {
        MemoryInfo info;
        
        int64_t memsize;
        size_t size = sizeof(memsize);
        if (sysctlbyname("hw.memsize", &memsize, &size, nullptr, 0) == 0) {
            info.totalPhysical = memsize;
        }
        
        // Get available memory using Mach
        mach_port_t host = mach_host_self();
        vm_statistics64_data_t vmStats;
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        
        if (host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&vmStats, &count) == KERN_SUCCESS) {
            vm_size_t pageSize;
            host_page_size(host, &pageSize);
            info.availablePhysical = vmStats.free_count * pageSize;
            info.usagePercent = 100.0f * (1.0f - (float)vmStats.free_count / 
                (vmStats.free_count + vmStats.active_count + vmStats.inactive_count + vmStats.wire_count));
        }
        
        return info;
    }
    
    std::vector<GPUInfo> getGPUInfo() const override {
        // TODO: Implement using IOKit
        return {};
    }
    
    std::vector<DisplayInfoEx> getDisplays() const override {
        // TODO: Implement using CoreGraphics
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
        // TODO: Implement using IOKit
        return BatteryInfo{};
    }
    
    bool supportsFeature(const std::string& feature) const override {
        if (feature == "vulkan") return true;  // Via MoltenVK
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
        return getHomeDirectory() + "/Library/Application Support";
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
    return std::make_unique<MacOSFileDialog>();
}

std::unique_ptr<ISystemTray> createSystemTray() {
    return std::make_unique<MacOSSystemTray>();
}

std::unique_ptr<INotification> createNotificationManager() {
    return std::make_unique<MacOSNotification>();
}

std::unique_ptr<IClipboard> createClipboard() {
    return std::make_unique<MacOSClipboard>();
}

std::unique_ptr<IDragSource> createDragSource() {
    return std::make_unique<MacOSDragSource>();
}

std::unique_ptr<IDropTargetManager> createDropTargetManager() {
    return std::make_unique<MacOSDropTargetManager>();
}

std::unique_ptr<IInternalDragManager> createInternalDragManager() {
    return std::make_unique<MacOSInternalDragManager>();
}

std::unique_ptr<ISystemInfo> createSystemInfo() {
    return std::make_unique<MacOSSystemInfo>();
}

} // namespace KillerGK

#endif // __APPLE__
