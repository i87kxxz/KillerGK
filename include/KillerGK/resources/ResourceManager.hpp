/**
 * @file ResourceManager.hpp
 * @brief Resource management system for KillerGK
 * 
 * Provides resource loading, caching, reference counting, memory management,
 * hot reload, and asset bundling capabilities.
 */

#pragma once

#include "../core/Types.hpp"
#include "../core/Error.hpp"
#include "../rendering/Texture.hpp"
#include "../text/Font.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include <chrono>
#include <filesystem>

namespace KillerGK {

// Forward declarations
class ShaderProgram;
using ShaderHandle = std::shared_ptr<ShaderProgram>;
using ModelHandle = std::shared_ptr<class Model>;
using AudioHandle = std::shared_ptr<class AudioResource>;

/**
 * @brief Resource type enumeration
 */
enum class ResourceType {
    Unknown,
    Image,
    Font,
    Audio,
    Shader,
    Model,
    Bundle
};

/**
 * @brief Resource loading priority
 */
enum class LoadPriority {
    Low,
    Normal,
    High,
    Immediate
};

/**
 * @brief Memory management policy for cache eviction
 */
enum class EvictionPolicy {
    LRU,        ///< Least Recently Used - evict oldest accessed resources first
    LFU,        ///< Least Frequently Used - evict least accessed resources first
    FIFO        ///< First In First Out - evict oldest loaded resources first
};

/**
 * @brief Per-resource-type memory statistics
 */
struct ResourceTypeStats {
    size_t count = 0;                   ///< Number of resources of this type
    size_t cpuMemoryUsage = 0;          ///< CPU memory usage in bytes
    size_t gpuMemoryUsage = 0;          ///< GPU memory usage in bytes
    size_t totalSize = 0;               ///< Total size (CPU + GPU) in bytes
};

/**
 * @brief Resource statistics
 */
struct ResourceStats {
    // Memory statistics
    size_t totalMemoryUsage = 0;        ///< Total CPU memory usage in bytes
    size_t gpuMemoryUsage = 0;          ///< Total GPU memory usage in bytes
    size_t peakMemoryUsage = 0;         ///< Peak CPU memory usage in bytes
    size_t peakGPUMemoryUsage = 0;      ///< Peak GPU memory usage in bytes
    size_t memoryLimit = 0;             ///< Configured memory limit in bytes
    float memoryUsagePercent = 0.0f;    ///< Memory usage as percentage of limit
    
    // Resource counts
    size_t cachedResourceCount = 0;     ///< Number of cached resources
    size_t loadedImageCount = 0;        ///< Number of loaded images
    size_t loadedFontCount = 0;         ///< Number of loaded fonts
    size_t loadedAudioCount = 0;        ///< Number of loaded audio files
    size_t loadedShaderCount = 0;       ///< Number of loaded shaders
    size_t loadedModelCount = 0;        ///< Number of loaded 3D models
    size_t loadedBundleCount = 0;       ///< Number of loaded bundles
    
    // Cache statistics
    size_t cacheHits = 0;               ///< Number of cache hits
    size_t cacheMisses = 0;             ///< Number of cache misses
    float cacheHitRate = 0.0f;          ///< Cache hit rate (0.0 - 1.0)
    size_t evictionCount = 0;           ///< Number of resources evicted
    
    // Garbage collection statistics
    size_t gcRunCount = 0;              ///< Number of garbage collection runs
    size_t gcFreedCount = 0;            ///< Total resources freed by GC
    size_t gcFreedBytes = 0;            ///< Total bytes freed by GC
    
    // Performance statistics
    float averageLoadTime = 0.0f;       ///< Average resource load time in ms
    float totalLoadTime = 0.0f;         ///< Total time spent loading resources in ms
    size_t totalLoadsCount = 0;         ///< Total number of resource loads
    
    // Hot reload statistics
    bool hotReloadEnabled = false;      ///< Whether hot reload is enabled
    size_t watchedPathCount = 0;        ///< Number of watched paths
    size_t hotReloadCount = 0;          ///< Number of hot reloads performed
    
    // Bundle statistics
    size_t mountedBundleCount = 0;      ///< Number of mounted bundles
    size_t bundleTotalSize = 0;         ///< Total size of all mounted bundles
};


/**
 * @brief Base class for all managed resources
 */
class Resource {
public:
    virtual ~Resource() = default;
    
    /**
     * @brief Get the resource path
     */
    [[nodiscard]] const std::string& getPath() const { return m_path; }
    
    /**
     * @brief Get the resource type
     */
    [[nodiscard]] ResourceType getType() const { return m_type; }
    
    /**
     * @brief Get memory usage in bytes
     */
    [[nodiscard]] virtual size_t getMemoryUsage() const = 0;
    
    /**
     * @brief Get GPU memory usage in bytes
     */
    [[nodiscard]] virtual size_t getGPUMemoryUsage() const { return 0; }
    
    /**
     * @brief Get reference count
     */
    [[nodiscard]] int getRefCount() const { return m_refCount.load(); }
    
    /**
     * @brief Get last access time
     */
    [[nodiscard]] std::chrono::steady_clock::time_point getLastAccessTime() const { 
        return m_lastAccessTime; 
    }
    
    /**
     * @brief Check if resource is loaded
     */
    [[nodiscard]] bool isLoaded() const { return m_loaded; }
    
    /**
     * @brief Reload the resource from disk
     */
    virtual bool reload() = 0;

protected:
    friend class ResourceManager;
    
    std::string m_path;
    ResourceType m_type = ResourceType::Unknown;
    std::atomic<int> m_refCount{0};
    std::chrono::steady_clock::time_point m_lastAccessTime;
    std::chrono::steady_clock::time_point m_loadTime;
    bool m_loaded = false;
    
    void addRef() { m_refCount++; }
    void release() { m_refCount--; }
    void updateAccessTime() { m_lastAccessTime = std::chrono::steady_clock::now(); }
};

using ResourceHandle = std::shared_ptr<Resource>;

/**
 * @brief File change callback type
 */
using FileChangeCallback = std::function<void(const std::string& path)>;

/**
 * @brief Resource load callback type
 */
using ResourceLoadCallback = std::function<void(const std::string& path, bool success)>;


/**
 * @brief Asset bundle header
 */
struct BundleHeader {
    char magic[4] = {'K', 'G', 'K', 'B'};  ///< Magic number "KGKB"
    uint32_t version = 1;                   ///< Bundle format version
    uint32_t fileCount = 0;                 ///< Number of files in bundle
    uint64_t dataOffset = 0;                ///< Offset to file data
    uint64_t totalSize = 0;                 ///< Total bundle size
    uint32_t flags = 0;                     ///< Bundle flags (bit 0: compressed)
};

/**
 * @brief Bundle flags
 */
enum class BundleFlags : uint32_t {
    None = 0,
    Compressed = 1 << 0,    ///< Bundle data is compressed
    Encrypted = 1 << 1      ///< Bundle data is encrypted (reserved for future)
};

/**
 * @brief Asset bundle file entry
 */
struct BundleEntry {
    std::string path;           ///< Virtual path within bundle
    uint64_t offset = 0;        ///< Offset in bundle data
    uint64_t size = 0;          ///< Compressed size
    uint64_t originalSize = 0;  ///< Original uncompressed size
    uint32_t checksum = 0;      ///< CRC32 checksum
    bool compressed = false;    ///< Whether data is compressed
};

/**
 * @class AssetBundle
 * @brief Represents a packaged asset bundle
 * 
 * Asset bundles provide a way to package multiple resources into a single
 * archive file for efficient distribution and loading. The bundle format
 * supports optional compression using RLE (Run-Length Encoding) for
 * simple compression that works well with many asset types.
 * 
 * Bundle Format (KGKB):
 * - Header: Magic number, version, file count, data offset, total size, flags
 * - Entry Table: For each file - path, offset, size, original size, checksum, compressed flag
 * - Data Section: Raw or compressed file data
 */
class AssetBundle {
public:
    ~AssetBundle();
    
    /**
     * @brief Load bundle from file
     * @param path Path to the bundle file
     * @return Bundle handle or nullptr on failure
     */
    static std::shared_ptr<AssetBundle> load(const std::string& path);
    
    /**
     * @brief Create a new empty bundle
     * @return New bundle handle
     */
    static std::shared_ptr<AssetBundle> create();
    
    /**
     * @brief Add file to bundle from disk
     * @param virtualPath Virtual path within the bundle
     * @param realPath Actual file path on disk
     * @return true if file was added successfully
     */
    bool addFile(const std::string& virtualPath, const std::string& realPath);
    
    /**
     * @brief Add data to bundle from memory
     * @param virtualPath Virtual path within the bundle
     * @param data Raw data to add
     * @return true if data was added successfully
     */
    bool addData(const std::string& virtualPath, const std::vector<uint8_t>& data);
    
    /**
     * @brief Add directory recursively to bundle
     * @param virtualBasePath Base virtual path for the directory
     * @param realPath Actual directory path on disk
     * @return Number of files added, or -1 on error
     */
    int addDirectory(const std::string& virtualBasePath, const std::string& realPath);
    
    /**
     * @brief Save bundle to file
     * @param path Output file path
     * @param compress Whether to compress the data
     * @return true if bundle was saved successfully
     */
    bool save(const std::string& path, bool compress = true);
    
    /**
     * @brief Check if bundle contains a file
     * @param virtualPath Virtual path to check
     * @return true if file exists in bundle
     */
    [[nodiscard]] bool contains(const std::string& virtualPath) const;
    
    /**
     * @brief Get file data from bundle
     * @param virtualPath Virtual path of the file
     * @return File data or empty vector if not found
     */
    [[nodiscard]] std::vector<uint8_t> getData(const std::string& virtualPath) const;
    
    /**
     * @brief Get list of all files in bundle
     * @return Vector of virtual paths
     */
    [[nodiscard]] std::vector<std::string> getFileList() const;
    
    /**
     * @brief Get bundle file path
     * @return Path to the bundle file
     */
    [[nodiscard]] const std::string& getPath() const { return m_path; }
    
    /**
     * @brief Get number of files in bundle
     * @return File count
     */
    [[nodiscard]] size_t getFileCount() const { return m_entries.size(); }
    
    /**
     * @brief Get total uncompressed size of all files
     * @return Total size in bytes
     */
    [[nodiscard]] size_t getTotalSize() const;
    
    /**
     * @brief Get compressed size of bundle data
     * @return Compressed size in bytes
     */
    [[nodiscard]] size_t getCompressedSize() const { return m_data.size(); }
    
    /**
     * @brief Check if bundle is compressed
     * @return true if bundle uses compression
     */
    [[nodiscard]] bool isCompressed() const { 
        return (m_header.flags & static_cast<uint32_t>(BundleFlags::Compressed)) != 0; 
    }
    
    /**
     * @brief Get file entry information
     * @param virtualPath Virtual path of the file
     * @return Pointer to entry or nullptr if not found
     */
    [[nodiscard]] const BundleEntry* getEntry(const std::string& virtualPath) const;
    
    /**
     * @brief Extract all files to a directory
     * @param outputDir Output directory path
     * @return Number of files extracted, or -1 on error
     */
    int extractAll(const std::string& outputDir) const;
    
    /**
     * @brief Extract a single file to disk
     * @param virtualPath Virtual path of the file
     * @param outputPath Output file path
     * @return true if extraction was successful
     */
    bool extractFile(const std::string& virtualPath, const std::string& outputPath) const;

private:
    AssetBundle() = default;
    
    // Compression utilities
    static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, size_t originalSize);
    static uint32_t calculateCRC32(const std::vector<uint8_t>& data);
    
    std::string m_path;
    BundleHeader m_header;
    std::unordered_map<std::string, BundleEntry> m_entries;
    std::vector<uint8_t> m_data;
    mutable std::mutex m_mutex;
};

using BundleHandle = std::shared_ptr<AssetBundle>;


/**
 * @class FileWatcher
 * @brief Watches files for changes (hot reload support)
 */
class FileWatcher {
public:
    FileWatcher();
    ~FileWatcher();
    
    /**
     * @brief Start watching a file
     */
    void watchFile(const std::string& path, FileChangeCallback callback);
    
    /**
     * @brief Start watching a directory
     */
    void watchDirectory(const std::string& path, FileChangeCallback callback);
    
    /**
     * @brief Stop watching a file
     */
    void unwatchFile(const std::string& path);
    
    /**
     * @brief Stop watching a directory
     */
    void unwatchDirectory(const std::string& path);
    
    /**
     * @brief Check for file changes (call periodically)
     */
    void update();
    
    /**
     * @brief Enable/disable watching
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    /**
     * @brief Check if watching is enabled
     */
    [[nodiscard]] bool isEnabled() const { return m_enabled; }

    /**
     * @brief Clear all watches
     */
    void clearAll();
    
    /**
     * @brief Get number of watched paths
     */
    [[nodiscard]] size_t getWatchCount() const;
    
    /**
     * @brief Get list of watched paths
     */
    [[nodiscard]] std::vector<std::string> getWatchedPaths() const;
    
    /**
     * @brief Check if a specific path is being watched
     */
    [[nodiscard]] bool isWatching(const std::string& path) const;

private:
    struct WatchEntry {
        std::string path;
        FileChangeCallback callback;
        std::filesystem::file_time_type lastModified;
        bool isDirectory = false;
    };
    
    std::unordered_map<std::string, WatchEntry> m_watches;
    std::unordered_map<std::string, std::filesystem::file_time_type> m_fileTimestamps; ///< Track file timestamps within watched directories
    mutable std::mutex m_mutex;
    bool m_enabled = true;
};


/**
 * @class ResourceManager
 * @brief Central resource management system
 * 
 * Provides:
 * - Resource loading with caching
 * - Reference counting for automatic cleanup
 * - Memory limit enforcement with automatic eviction
 * - Hot reload support for development
 * - Asset bundling for distribution
 * - Detailed statistics tracking
 */
class ResourceManager {
public:
    /**
     * @brief Get singleton instance
     */
    static ResourceManager& instance();
    
    /**
     * @brief Initialize the resource manager
     */
    bool initialize();
    
    /**
     * @brief Shutdown and release all resources
     */
    void shutdown();
    
    // ========== Resource Loading ==========
    
    /**
     * @brief Load an image (cached)
     * @param path Image file path
     * @param config Texture configuration
     * @return Texture handle or nullptr on failure
     */
    TextureHandle loadImage(const std::string& path, 
                            const TextureConfig& config = TextureConfig{});
    
    /**
     * @brief Load a font (cached)
     * @param path Font file path
     * @param config Font configuration
     * @return Font handle or nullptr on failure
     */
    FontHandle loadFont(const std::string& path, 
                        const FontConfig& config = FontConfig{});
    
    /**
     * @brief Load a shader (cached)
     * @param vertPath Vertex shader path
     * @param fragPath Fragment shader path
     * @return Shader handle or nullptr on failure
     */
    ShaderHandle loadShader(const std::string& vertPath, const std::string& fragPath);
    
    /**
     * @brief Load a 3D model (cached)
     * @param path Model file path
     * @return Model handle or nullptr on failure
     */
    ModelHandle loadModel(const std::string& path);
    
    /**
     * @brief Load audio (cached)
     * @param path Audio file path
     * @return Audio handle or nullptr on failure
     */
    AudioHandle loadAudio(const std::string& path);
    
    /**
     * @brief Preload multiple resources
     * @param paths List of file paths to preload
     * @param callback Optional callback for progress
     */
    void preload(const std::vector<std::string>& paths, 
                 ResourceLoadCallback callback = nullptr);
    
    /**
     * @brief Preload resources asynchronously
     * @param paths List of file paths to preload
     * @param callback Optional callback for each resource
     */
    void preloadAsync(const std::vector<std::string>& paths,
                      ResourceLoadCallback callback = nullptr);

    
    // ========== Cache Management ==========
    
    /**
     * @brief Unload a specific resource from cache
     * @param path Resource path
     */
    void unload(const std::string& path);
    
    /**
     * @brief Clear all cached resources
     */
    void clearCache();
    
    /**
     * @brief Check if a resource is cached
     * @param path Resource path
     * @return true if resource is in cache
     */
    [[nodiscard]] bool isCached(const std::string& path) const;
    
    /**
     * @brief Get a cached resource
     * @param path Resource path
     * @return Resource handle or nullptr if not cached
     */
    [[nodiscard]] ResourceHandle getCached(const std::string& path) const;
    
    // ========== Memory Management ==========
    
    /**
     * @brief Set memory limit for resources
     * @param bytes Maximum memory usage in bytes (0 = unlimited)
     */
    void setMemoryLimit(size_t bytes);
    
    /**
     * @brief Get current memory limit
     * @return Memory limit in bytes
     */
    [[nodiscard]] size_t getMemoryLimit() const { return m_memoryLimit; }
    
    /**
     * @brief Get current memory usage
     * @return Memory usage in bytes
     */
    [[nodiscard]] size_t getMemoryUsage() const;
    
    /**
     * @brief Get current GPU memory usage
     * @return GPU memory usage in bytes
     */
    [[nodiscard]] size_t getGPUMemoryUsage() const;
    
    /**
     * @brief Force garbage collection
     * @return Number of resources freed
     */
    int gc();
    
    /**
     * @brief Evict resources to meet memory limit
     * @param targetBytes Target memory usage
     * @return Number of resources evicted
     */
    int evictToLimit(size_t targetBytes);
    
    /**
     * @brief Set the cache eviction policy
     * @param policy The eviction policy to use (LRU, LFU, or FIFO)
     */
    void setEvictionPolicy(EvictionPolicy policy);
    
    /**
     * @brief Get the current eviction policy
     * @return Current eviction policy
     */
    [[nodiscard]] EvictionPolicy getEvictionPolicy() const { return m_evictionPolicy; }
    
    /**
     * @brief Check if memory limit is enforced
     * @return true if memory limit is set and enforced
     */
    [[nodiscard]] bool isMemoryLimitEnforced() const { return m_memoryLimit > 0; }
    
    /**
     * @brief Get memory usage as percentage of limit
     * @return Percentage (0-100+) or 0 if no limit set
     */
    [[nodiscard]] float getMemoryUsagePercent() const;
    
    /**
     * @brief Trigger automatic eviction if over memory limit
     * @return Number of resources evicted
     */
    int enforceMemoryLimit();
    
    // ========== Hot Reload ==========
    
    /**
     * @brief Enable/disable hot reload
     * @param enabled Whether to enable hot reload
     */
    void enableHotReload(bool enabled);
    
    /**
     * @brief Check if hot reload is enabled
     */
    [[nodiscard]] bool isHotReloadEnabled() const { return m_hotReloadEnabled; }
    
    /**
     * @brief Watch a directory for changes
     * @param path Directory path
     */
    void watchDirectory(const std::string& path);
    
    /**
     * @brief Stop watching a directory
     * @param path Directory path
     */
    void unwatchDirectory(const std::string& path);
    
    /**
     * @brief Update file watcher (call each frame)
     */
    void updateHotReload();
    
    /**
     * @brief Set callback for resource reload
     */
    void setReloadCallback(ResourceLoadCallback callback);
    
    /**
     * @brief Get list of watched directories
     */
    [[nodiscard]] std::vector<std::string> getWatchedDirectories() const;
    
    /**
     * @brief Check if a path is being watched
     */
    [[nodiscard]] bool isPathWatched(const std::string& path) const;
    
    /**
     * @brief Force reload a specific resource
     * @param path Resource path to reload
     * @return true if reload was successful
     */
    bool reloadResource(const std::string& path);

    
    // ========== Asset Bundling ==========
    
    /**
     * @brief Load an asset bundle
     * @param path Bundle file path
     * @return Bundle handle or nullptr on failure
     */
    BundleHandle loadBundle(const std::string& path);
    
    /**
     * @brief Create a new asset bundle
     * @return New bundle handle
     */
    BundleHandle createBundle();
    
    /**
     * @brief Mount a bundle for resource loading
     * @param bundle Bundle to mount
     * @param mountPoint Virtual mount point (e.g., "assets/")
     */
    void mountBundle(BundleHandle bundle, const std::string& mountPoint = "");
    
    /**
     * @brief Unmount a bundle
     * @param bundle Bundle to unmount
     */
    void unmountBundle(BundleHandle bundle);
    
    /**
     * @brief Check if a path exists in mounted bundles
     * @param path Virtual path
     * @return true if path exists in a bundle
     */
    [[nodiscard]] bool existsInBundle(const std::string& path) const;
    
    /**
     * @brief Get data from mounted bundles
     * @param path Virtual path
     * @return File data or empty vector if not found
     */
    [[nodiscard]] std::vector<uint8_t> getBundleData(const std::string& path) const;
    
    // ========== Statistics ==========
    
    /**
     * @brief Get resource statistics
     * @return Current statistics
     */
    [[nodiscard]] ResourceStats stats() const;
    
    /**
     * @brief Reset statistics counters
     */
    void resetStats();
    
    /**
     * @brief Get detailed memory breakdown
     * @return Map of resource type to memory usage
     */
    [[nodiscard]] std::unordered_map<ResourceType, size_t> getMemoryBreakdown() const;
    
    /**
     * @brief Get detailed statistics for a specific resource type
     * @param type The resource type to query
     * @return Statistics for the specified resource type
     */
    [[nodiscard]] ResourceTypeStats getResourceTypeStats(ResourceType type) const;
    
    /**
     * @brief Get statistics for all resource types
     * @return Map of resource type to detailed statistics
     */
    [[nodiscard]] std::unordered_map<ResourceType, ResourceTypeStats> getAllResourceTypeStats() const;
    
    /**
     * @brief Get peak memory usage (CPU)
     * @return Peak memory usage in bytes
     */
    [[nodiscard]] size_t getPeakMemoryUsage() const { return m_peakMemoryUsage.load(); }
    
    /**
     * @brief Get peak GPU memory usage
     * @return Peak GPU memory usage in bytes
     */
    [[nodiscard]] size_t getPeakGPUMemoryUsage() const { return m_peakGPUMemoryUsage.load(); }
    
    /**
     * @brief Reset peak memory tracking
     */
    void resetPeakMemory();
    
    /**
     * @brief Get formatted statistics string for debugging
     * @return Human-readable statistics string
     */
    [[nodiscard]] std::string getStatsString() const;

private:
    ResourceManager() = default;
    ~ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    // Internal helpers
    ResourceType detectResourceType(const std::string& path) const;
    std::string normalizePath(const std::string& path) const;
    std::string getCacheKey(const std::string& path, const std::string& suffix = "") const;
    void trackMemoryUsage(size_t bytes);
    void untrackMemoryUsage(size_t bytes);
    void trackGPUMemoryUsage(size_t bytes);
    void untrackGPUMemoryUsage(size_t bytes);
    void onFileChanged(const std::string& path);
    bool loadFromBundle(const std::string& path, std::vector<uint8_t>& data) const;
    
    // Memory management helpers
    void trackResourceMetadata(const std::string& key, ResourceType type, size_t memoryUsage);
    void untrackResourceMetadata(const std::string& key);
    void updateResourceAccessTime(const std::string& key);
    std::vector<std::string> getEvictionCandidates(size_t bytesToFree) const;
    int evictResources(const std::vector<std::string>& keys);
    size_t getResourceMemoryUsage(const std::string& key) const;
    
    // State
    bool m_initialized = false;
    
    // Cache
    mutable std::mutex m_cacheMutex;
    std::unordered_map<std::string, TextureHandle> m_textureCache;
    std::unordered_map<std::string, FontHandle> m_fontCache;
    std::unordered_map<std::string, ShaderHandle> m_shaderCache;
    std::unordered_map<std::string, ModelHandle> m_modelCache;
    std::unordered_map<std::string, AudioHandle> m_audioCache;
    
    // Memory management
    std::atomic<size_t> m_memoryLimit{0};
    std::atomic<size_t> m_currentMemoryUsage{0};
    std::atomic<size_t> m_currentGPUMemoryUsage{0};
    std::atomic<size_t> m_peakMemoryUsage{0};
    std::atomic<size_t> m_peakGPUMemoryUsage{0};
    EvictionPolicy m_evictionPolicy = EvictionPolicy::LRU;
    
    // Resource metadata for LRU/LFU tracking
    struct ResourceMetadata {
        std::string key;
        ResourceType type;
        size_t memoryUsage = 0;
        std::chrono::steady_clock::time_point lastAccessTime;
        std::chrono::steady_clock::time_point loadTime;
        size_t accessCount = 0;
    };
    std::unordered_map<std::string, ResourceMetadata> m_resourceMetadata;
    
    // Hot reload
    bool m_hotReloadEnabled = false;
    std::unique_ptr<FileWatcher> m_fileWatcher;
    ResourceLoadCallback m_reloadCallback;
    
    // Bundles
    mutable std::mutex m_bundleMutex;
    std::vector<std::pair<std::string, BundleHandle>> m_mountedBundles;
    
    // Statistics
    mutable std::mutex m_statsMutex;
    mutable ResourceStats m_stats;
    std::vector<float> m_loadTimes;
    std::atomic<size_t> m_hotReloadCount{0};
    std::atomic<size_t> m_gcFreedBytes{0};
};

} // namespace KillerGK
