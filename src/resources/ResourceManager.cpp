/**
 * @file ResourceManager.cpp
 * @brief Resource management system implementation
 */

#include "KillerGK/resources/ResourceManager.hpp"
#include "KillerGK/core/Error.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iomanip>

namespace KillerGK {

// ============================================================================
// FileWatcher Implementation
// ============================================================================

FileWatcher::FileWatcher() = default;

FileWatcher::~FileWatcher() = default;

void FileWatcher::watchFile(const std::string& path, FileChangeCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        WatchEntry entry;
        entry.path = path;
        entry.callback = std::move(callback);
        entry.isDirectory = false;
        
        if (std::filesystem::exists(path)) {
            entry.lastModified = std::filesystem::last_write_time(path);
        }
        
        m_watches[path] = std::move(entry);
    } catch (const std::exception& e) {
        log(LogLevel::Warning, "Failed to watch file: " + path + " - " + e.what());
    }
}

void FileWatcher::watchDirectory(const std::string& path, FileChangeCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        WatchEntry entry;
        entry.path = path;
        entry.callback = std::move(callback);
        entry.isDirectory = true;
        
        if (std::filesystem::exists(path)) {
            entry.lastModified = std::filesystem::last_write_time(path);
        }
        
        m_watches[path] = std::move(entry);
    } catch (const std::exception& e) {
        log(LogLevel::Warning, "Failed to watch directory: " + path + " - " + e.what());
    }
}


void FileWatcher::unwatchFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_watches.erase(path);
}

void FileWatcher::unwatchDirectory(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_watches.erase(path);
    
    // Also remove any tracked file timestamps for this directory
    for (auto it = m_fileTimestamps.begin(); it != m_fileTimestamps.end();) {
        if (it->first.find(path + "|") == 0) {
            it = m_fileTimestamps.erase(it);
        } else {
            ++it;
        }
    }
}

void FileWatcher::clearAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_watches.clear();
    m_fileTimestamps.clear();
}

size_t FileWatcher::getWatchCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_watches.size();
}

std::vector<std::string> FileWatcher::getWatchedPaths() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> paths;
    paths.reserve(m_watches.size());
    for (const auto& [path, entry] : m_watches) {
        paths.push_back(path);
    }
    return paths;
}

bool FileWatcher::isWatching(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_watches.find(path) != m_watches.end();
}

void FileWatcher::update() {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& [path, entry] : m_watches) {
        try {
            if (!std::filesystem::exists(path)) continue;
            
            if (entry.isDirectory) {
                // For directories, check all files within for changes
                for (const auto& file : std::filesystem::recursive_directory_iterator(path)) {
                    if (file.is_regular_file()) {
                        auto filePath = file.path().string();
                        auto currentTime = std::filesystem::last_write_time(file);
                        
                        // Check if this file has been modified since last check
                        // We use a separate map to track individual file times within directories
                        auto fileKey = path + "|" + filePath;
                        auto it = m_fileTimestamps.find(fileKey);
                        
                        if (it == m_fileTimestamps.end()) {
                            // First time seeing this file, record its timestamp
                            m_fileTimestamps[fileKey] = currentTime;
                        } else if (currentTime != it->second) {
                            // File has been modified
                            it->second = currentTime;
                            entry.callback(filePath);
                        }
                    }
                }
            } else {
                // For individual files
                auto currentTime = std::filesystem::last_write_time(path);
                
                if (currentTime != entry.lastModified) {
                    entry.lastModified = currentTime;
                    entry.callback(path);
                }
            }
        } catch (const std::exception& e) {
            log(LogLevel::Warning, "Error checking file: " + path + " - " + e.what());
        }
    }
}

// ============================================================================
// AssetBundle Implementation
// ============================================================================

AssetBundle::~AssetBundle() = default;

// CRC32 lookup table for checksum calculation
static const uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD706B3,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

uint32_t AssetBundle::calculateCRC32(const std::vector<uint8_t>& data) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint8_t byte : data) {
        crc = crc32_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

std::vector<uint8_t> AssetBundle::compress(const std::vector<uint8_t>& data) {
    if (data.empty()) return {};
    
    // Simple RLE compression
    // Format: [count][byte] where count is 1-255
    // If count > 127, it means (count - 128) literal bytes follow
    std::vector<uint8_t> compressed;
    compressed.reserve(data.size());
    
    size_t i = 0;
    while (i < data.size()) {
        // Count consecutive identical bytes
        uint8_t currentByte = data[i];
        size_t runLength = 1;
        
        while (i + runLength < data.size() && 
               data[i + runLength] == currentByte && 
               runLength < 127) {
            runLength++;
        }
        
        if (runLength >= 3) {
            // Encode as run: [count | 0x80][byte]
            compressed.push_back(static_cast<uint8_t>(runLength | 0x80));
            compressed.push_back(currentByte);
            i += runLength;
        } else {
            // Collect literal bytes
            size_t literalStart = i;
            size_t literalCount = 0;
            
            while (i < data.size() && literalCount < 127) {
                // Check if next bytes form a run
                size_t nextRunLength = 1;
                if (i + 1 < data.size()) {
                    while (i + nextRunLength < data.size() && 
                           data[i + nextRunLength] == data[i] && 
                           nextRunLength < 127) {
                        nextRunLength++;
                    }
                }
                
                if (nextRunLength >= 3) {
                    break;  // Start a new run
                }
                
                literalCount++;
                i++;
            }
            
            if (literalCount > 0) {
                // Encode as literals: [count][byte1][byte2]...
                compressed.push_back(static_cast<uint8_t>(literalCount));
                for (size_t j = 0; j < literalCount; j++) {
                    compressed.push_back(data[literalStart + j]);
                }
            }
        }
    }
    
    // Only use compression if it actually reduces size
    if (compressed.size() >= data.size()) {
        return data;  // Return original if compression doesn't help
    }
    
    return compressed;
}

std::vector<uint8_t> AssetBundle::decompress(const std::vector<uint8_t>& data, size_t originalSize) {
    if (data.empty()) return {};
    
    std::vector<uint8_t> decompressed;
    decompressed.reserve(originalSize);
    
    size_t i = 0;
    while (i < data.size() && decompressed.size() < originalSize) {
        uint8_t control = data[i++];
        
        if (control & 0x80) {
            // Run-length encoded: repeat next byte (control & 0x7F) times
            size_t count = control & 0x7F;
            if (i < data.size()) {
                uint8_t byte = data[i++];
                for (size_t j = 0; j < count && decompressed.size() < originalSize; j++) {
                    decompressed.push_back(byte);
                }
            }
        } else {
            // Literal bytes: copy next 'control' bytes
            size_t count = control;
            for (size_t j = 0; j < count && i < data.size() && decompressed.size() < originalSize; j++) {
                decompressed.push_back(data[i++]);
            }
        }
    }
    
    return decompressed;
}

std::shared_ptr<AssetBundle> AssetBundle::load(const std::string& path) {
    auto bundle = std::shared_ptr<AssetBundle>(new AssetBundle());
    bundle->m_path = path;
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        log(LogLevel::Error, "Failed to open bundle: " + path);
        return nullptr;
    }
    
    // Read header
    file.read(reinterpret_cast<char*>(&bundle->m_header), sizeof(BundleHeader));
    
    // Validate magic number
    if (std::memcmp(bundle->m_header.magic, "KGKB", 4) != 0) {
        log(LogLevel::Error, "Invalid bundle format: " + path);
        return nullptr;
    }
    
    // Validate version
    if (bundle->m_header.version > 1) {
        log(LogLevel::Warning, "Bundle version " + std::to_string(bundle->m_header.version) + 
            " may not be fully supported");
    }
    
    // Read file entries
    for (uint32_t i = 0; i < bundle->m_header.fileCount; ++i) {
        BundleEntry entry;
        
        // Read path length and path
        uint32_t pathLen = 0;
        file.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen));
        
        if (pathLen > 4096) {  // Sanity check
            log(LogLevel::Error, "Invalid path length in bundle: " + path);
            return nullptr;
        }
        
        entry.path.resize(pathLen);
        file.read(entry.path.data(), pathLen);
        
        // Read entry metadata
        file.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
        file.read(reinterpret_cast<char*>(&entry.size), sizeof(entry.size));
        file.read(reinterpret_cast<char*>(&entry.originalSize), sizeof(entry.originalSize));
        file.read(reinterpret_cast<char*>(&entry.checksum), sizeof(entry.checksum));
        file.read(reinterpret_cast<char*>(&entry.compressed), sizeof(entry.compressed));
        
        bundle->m_entries[entry.path] = entry;
    }
    
    // Read all data
    size_t dataSize = bundle->m_header.totalSize - bundle->m_header.dataOffset;
    bundle->m_data.resize(dataSize);
    file.seekg(bundle->m_header.dataOffset);
    file.read(reinterpret_cast<char*>(bundle->m_data.data()), dataSize);
    
    log(LogLevel::Info, "Loaded bundle: " + path + " with " + 
        std::to_string(bundle->m_header.fileCount) + " files");
    
    return bundle;
}


std::shared_ptr<AssetBundle> AssetBundle::create() {
    return std::shared_ptr<AssetBundle>(new AssetBundle());
}

bool AssetBundle::addFile(const std::string& virtualPath, const std::string& realPath) {
    std::ifstream file(realPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        log(LogLevel::Error, "Failed to open file for bundling: " + realPath);
        return false;
    }
    
    size_t fileSize = file.tellg();
    file.seekg(0);
    
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    
    return addData(virtualPath, data);
}

bool AssetBundle::addData(const std::string& virtualPath, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if entry already exists
    if (m_entries.find(virtualPath) != m_entries.end()) {
        log(LogLevel::Warning, "Overwriting existing entry in bundle: " + virtualPath);
        // Remove old entry (we'll add new data at the end)
        // Note: This leaves orphaned data in m_data, but it's cleaned up on save
    }
    
    BundleEntry entry;
    entry.path = virtualPath;
    entry.offset = m_data.size();
    entry.originalSize = data.size();
    entry.compressed = false;  // Compression is applied during save
    entry.size = data.size();
    
    // Calculate CRC32 checksum
    entry.checksum = calculateCRC32(data);
    
    m_entries[virtualPath] = entry;
    m_data.insert(m_data.end(), data.begin(), data.end());
    m_header.fileCount = static_cast<uint32_t>(m_entries.size());
    
    return true;
}

int AssetBundle::addDirectory(const std::string& virtualBasePath, const std::string& realPath) {
    if (!std::filesystem::exists(realPath) || !std::filesystem::is_directory(realPath)) {
        log(LogLevel::Error, "Directory does not exist: " + realPath);
        return -1;
    }
    
    int filesAdded = 0;
    
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(realPath)) {
            if (entry.is_regular_file()) {
                // Calculate relative path
                std::string relativePath = std::filesystem::relative(entry.path(), realPath).string();
                
                // Normalize path separators
                std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
                
                // Construct virtual path
                std::string virtualPath = virtualBasePath;
                if (!virtualPath.empty() && virtualPath.back() != '/') {
                    virtualPath += '/';
                }
                virtualPath += relativePath;
                
                if (addFile(virtualPath, entry.path().string())) {
                    filesAdded++;
                }
            }
        }
    } catch (const std::exception& e) {
        log(LogLevel::Error, "Error adding directory to bundle: " + std::string(e.what()));
        return -1;
    }
    
    log(LogLevel::Info, "Added " + std::to_string(filesAdded) + " files from directory: " + realPath);
    return filesAdded;
}

bool AssetBundle::save(const std::string& path, bool compressData) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        log(LogLevel::Error, "Failed to create bundle: " + path);
        return false;
    }
    
    // Prepare data for saving - optionally compress each entry
    std::vector<uint8_t> outputData;
    std::unordered_map<std::string, BundleEntry> outputEntries;
    
    for (auto& [vpath, entry] : m_entries) {
        BundleEntry outputEntry = entry;
        outputEntry.offset = outputData.size();
        
        // Get original data
        std::vector<uint8_t> originalData(entry.size);
        if (entry.offset + entry.size <= m_data.size()) {
            std::memcpy(originalData.data(), m_data.data() + entry.offset, entry.size);
        }
        
        if (compressData && !originalData.empty()) {
            // Compress the data
            std::vector<uint8_t> compressedData = compress(originalData);
            
            if (compressedData.size() < originalData.size()) {
                // Compression was beneficial
                outputEntry.size = compressedData.size();
                outputEntry.originalSize = originalData.size();
                outputEntry.compressed = true;
                outputData.insert(outputData.end(), compressedData.begin(), compressedData.end());
            } else {
                // Compression didn't help, store uncompressed
                outputEntry.size = originalData.size();
                outputEntry.originalSize = originalData.size();
                outputEntry.compressed = false;
                outputData.insert(outputData.end(), originalData.begin(), originalData.end());
            }
        } else {
            // Store uncompressed
            outputEntry.size = originalData.size();
            outputEntry.originalSize = originalData.size();
            outputEntry.compressed = false;
            outputData.insert(outputData.end(), originalData.begin(), originalData.end());
        }
        
        outputEntries[vpath] = outputEntry;
    }
    
    // Calculate data offset (header + entries)
    size_t entriesSize = 0;
    for (const auto& [vpath, entry] : outputEntries) {
        entriesSize += sizeof(uint32_t) + vpath.size();  // Path length + path
        entriesSize += sizeof(entry.offset) + sizeof(entry.size) + 
                       sizeof(entry.originalSize) + sizeof(entry.checksum) + 
                       sizeof(entry.compressed);
    }
    
    // Update header
    BundleHeader outputHeader = m_header;
    outputHeader.fileCount = static_cast<uint32_t>(outputEntries.size());
    outputHeader.dataOffset = sizeof(BundleHeader) + entriesSize;
    outputHeader.totalSize = outputHeader.dataOffset + outputData.size();
    outputHeader.flags = compressData ? static_cast<uint32_t>(BundleFlags::Compressed) : 0;
    
    // Write header
    file.write(reinterpret_cast<const char*>(&outputHeader), sizeof(BundleHeader));
    
    // Write entries
    for (const auto& [vpath, entry] : outputEntries) {
        uint32_t pathLen = static_cast<uint32_t>(vpath.size());
        file.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
        file.write(vpath.data(), pathLen);
        file.write(reinterpret_cast<const char*>(&entry.offset), sizeof(entry.offset));
        file.write(reinterpret_cast<const char*>(&entry.size), sizeof(entry.size));
        file.write(reinterpret_cast<const char*>(&entry.originalSize), sizeof(entry.originalSize));
        file.write(reinterpret_cast<const char*>(&entry.checksum), sizeof(entry.checksum));
        file.write(reinterpret_cast<const char*>(&entry.compressed), sizeof(entry.compressed));
    }
    
    // Write data
    file.write(reinterpret_cast<const char*>(outputData.data()), outputData.size());
    
    m_path = path;
    
    log(LogLevel::Info, "Saved bundle: " + path + " with " + 
        std::to_string(outputEntries.size()) + " files, " +
        std::to_string(outputData.size()) + " bytes");
    
    return true;
}

bool AssetBundle::contains(const std::string& virtualPath) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_entries.find(virtualPath) != m_entries.end();
}

std::vector<uint8_t> AssetBundle::getData(const std::string& virtualPath) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_entries.find(virtualPath);
    if (it == m_entries.end()) {
        return {};
    }
    
    const auto& entry = it->second;
    
    // Bounds check
    if (entry.offset + entry.size > m_data.size()) {
        log(LogLevel::Error, "Invalid entry offset/size in bundle for: " + virtualPath);
        return {};
    }
    
    // Extract raw data
    std::vector<uint8_t> rawData(entry.size);
    std::memcpy(rawData.data(), m_data.data() + entry.offset, entry.size);
    
    // Decompress if needed
    if (entry.compressed && entry.size != entry.originalSize) {
        std::vector<uint8_t> decompressedData = decompress(rawData, entry.originalSize);
        
        // Verify checksum
        uint32_t checksum = calculateCRC32(decompressedData);
        if (checksum != entry.checksum) {
            log(LogLevel::Warning, "Checksum mismatch for: " + virtualPath);
        }
        
        return decompressedData;
    }
    
    // Verify checksum for uncompressed data
    uint32_t checksum = calculateCRC32(rawData);
    if (checksum != entry.checksum) {
        log(LogLevel::Warning, "Checksum mismatch for: " + virtualPath);
    }
    
    return rawData;
}

std::vector<std::string> AssetBundle::getFileList() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<std::string> files;
    files.reserve(m_entries.size());
    
    for (const auto& [path, entry] : m_entries) {
        files.push_back(path);
    }
    
    // Sort for consistent ordering
    std::sort(files.begin(), files.end());
    
    return files;
}

size_t AssetBundle::getTotalSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t total = 0;
    for (const auto& [path, entry] : m_entries) {
        total += entry.originalSize;
    }
    return total;
}

const BundleEntry* AssetBundle::getEntry(const std::string& virtualPath) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_entries.find(virtualPath);
    if (it == m_entries.end()) {
        return nullptr;
    }
    return &it->second;
}

int AssetBundle::extractAll(const std::string& outputDir) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    int extracted = 0;
    
    try {
        // Create output directory if it doesn't exist
        std::filesystem::create_directories(outputDir);
        
        for (const auto& [virtualPath, entry] : m_entries) {
            std::string outputPath = outputDir;
            if (!outputPath.empty() && outputPath.back() != '/' && outputPath.back() != '\\') {
                outputPath += '/';
            }
            outputPath += virtualPath;
            
            // Create parent directories
            std::filesystem::path filePath(outputPath);
            std::filesystem::create_directories(filePath.parent_path());
            
            // Get and write data (unlock mutex temporarily for getData)
            m_mutex.unlock();
            std::vector<uint8_t> data = getData(virtualPath);
            m_mutex.lock();
            
            if (!data.empty()) {
                std::ofstream file(outputPath, std::ios::binary);
                if (file.is_open()) {
                    file.write(reinterpret_cast<const char*>(data.data()), data.size());
                    extracted++;
                }
            }
        }
    } catch (const std::exception& e) {
        log(LogLevel::Error, "Error extracting bundle: " + std::string(e.what()));
        return -1;
    }
    
    log(LogLevel::Info, "Extracted " + std::to_string(extracted) + " files to: " + outputDir);
    return extracted;
}

bool AssetBundle::extractFile(const std::string& virtualPath, const std::string& outputPath) const {
    std::vector<uint8_t> data = getData(virtualPath);
    if (data.empty()) {
        return false;
    }
    
    try {
        // Create parent directories
        std::filesystem::path filePath(outputPath);
        std::filesystem::create_directories(filePath.parent_path());
        
        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            log(LogLevel::Error, "Failed to create output file: " + outputPath);
            return false;
        }
        
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    } catch (const std::exception& e) {
        log(LogLevel::Error, "Error extracting file: " + std::string(e.what()));
        return false;
    }
}


// ============================================================================
// ResourceManager Implementation
// ============================================================================

ResourceManager& ResourceManager::instance() {
    static ResourceManager instance;
    return instance;
}

ResourceManager::~ResourceManager() {
    shutdown();
}

bool ResourceManager::initialize() {
    if (m_initialized) return true;
    
    m_fileWatcher = std::make_unique<FileWatcher>();
    m_initialized = true;
    
    log(LogLevel::Info, "ResourceManager initialized");
    return true;
}

void ResourceManager::shutdown() {
    if (!m_initialized) return;
    
    clearCache();
    
    {
        std::lock_guard<std::mutex> lock(m_bundleMutex);
        m_mountedBundles.clear();
    }
    
    m_fileWatcher.reset();
    m_initialized = false;
    
    log(LogLevel::Info, "ResourceManager shutdown");
}

// ============================================================================
// Resource Loading
// ============================================================================

TextureHandle ResourceManager::loadImage(const std::string& path, const TextureConfig& config) {
    std::string key = getCacheKey(path);
    
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        auto it = m_textureCache.find(key);
        if (it != m_textureCache.end()) {
            m_stats.cacheHits++;
            // Update access time for LRU tracking
            updateResourceAccessTime(key);
            return it->second;
        }
    }
    
    m_stats.cacheMisses++;
    
    // Check if we need to enforce memory limit before loading
    size_t limit = m_memoryLimit.load();
    if (limit > 0) {
        // Estimate memory usage for the new resource (rough estimate)
        // We'll adjust after loading if needed
        enforceMemoryLimit();
    }
    
    auto startTime = std::chrono::steady_clock::now();
    
    TextureHandle texture;
    
    // Try loading from bundle first
    std::vector<uint8_t> bundleData;
    if (loadFromBundle(path, bundleData)) {
        texture = Texture::loadFromMemory(bundleData.data(), bundleData.size(), config);
    } else {
        texture = Texture::loadFromFile(path, config);
    }
    
    if (texture) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_textureCache[key] = texture;
        m_stats.loadedImageCount++;
        
        // Track memory (CPU and GPU - textures are uploaded to GPU)
        size_t memUsage = texture->getWidth() * texture->getHeight() * 4;
        trackMemoryUsage(memUsage);
        trackGPUMemoryUsage(memUsage);  // Textures are also stored on GPU
        
        // Track resource metadata for LRU/LFU eviction
        trackResourceMetadata(key, ResourceType::Image, memUsage);
        
        // Track load time
        auto endTime = std::chrono::steady_clock::now();
        float loadTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        m_loadTimes.push_back(loadTime);
        
        // Watch for hot reload
        if (m_hotReloadEnabled && !existsInBundle(path)) {
            m_fileWatcher->watchFile(path, [this](const std::string& p) {
                onFileChanged(p);
            });
        }
    }
    
    return texture;
}

FontHandle ResourceManager::loadFont(const std::string& path, const FontConfig& config) {
    std::string key = getCacheKey(path, "_" + std::to_string(static_cast<int>(config.size)));
    
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        auto it = m_fontCache.find(key);
        if (it != m_fontCache.end()) {
            m_stats.cacheHits++;
            // Update access time for LRU tracking
            updateResourceAccessTime(key);
            return it->second;
        }
    }
    
    m_stats.cacheMisses++;
    
    // Check if we need to enforce memory limit before loading
    size_t limit = m_memoryLimit.load();
    if (limit > 0) {
        enforceMemoryLimit();
    }
    
    auto startTime = std::chrono::steady_clock::now();
    
    FontHandle font;
    
    // Try loading from bundle first
    std::vector<uint8_t> bundleData;
    if (loadFromBundle(path, bundleData)) {
        font = Font::loadFromMemory(bundleData.data(), bundleData.size(), config);
    } else {
        font = Font::loadFromFile(path, config);
    }
    
    if (font) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_fontCache[key] = font;
        m_stats.loadedFontCount++;
        
        // Track memory (atlas size - both CPU and GPU)
        size_t memUsage = font->getAtlasWidth() * font->getAtlasHeight();
        trackMemoryUsage(memUsage);
        trackGPUMemoryUsage(memUsage);  // Font atlas is also stored on GPU
        
        // Track resource metadata for LRU/LFU eviction
        trackResourceMetadata(key, ResourceType::Font, memUsage);
        
        // Track load time
        auto endTime = std::chrono::steady_clock::now();
        float loadTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        m_loadTimes.push_back(loadTime);
        
        // Watch for hot reload
        if (m_hotReloadEnabled && !existsInBundle(path)) {
            m_fileWatcher->watchFile(path, [this](const std::string& p) {
                onFileChanged(p);
            });
        }
    }
    
    return font;
}


ShaderHandle ResourceManager::loadShader(const std::string& vertPath, const std::string& fragPath) {
    std::string key = getCacheKey(vertPath + "|" + fragPath);
    
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        auto it = m_shaderCache.find(key);
        if (it != m_shaderCache.end()) {
            m_stats.cacheHits++;
            return it->second;
        }
    }
    
    m_stats.cacheMisses++;
    
    // Shader loading would be implemented here
    // For now, return nullptr as ShaderProgram is not fully implemented
    log(LogLevel::Warning, "Shader loading not yet implemented");
    return nullptr;
}

ModelHandle ResourceManager::loadModel(const std::string& path) {
    std::string key = getCacheKey(path);
    
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        auto it = m_modelCache.find(key);
        if (it != m_modelCache.end()) {
            m_stats.cacheHits++;
            return it->second;
        }
    }
    
    m_stats.cacheMisses++;
    
    // Model loading would be implemented here
    // For now, return nullptr as Model is not fully implemented
    log(LogLevel::Warning, "Model loading not yet implemented");
    return nullptr;
}

AudioHandle ResourceManager::loadAudio(const std::string& path) {
    std::string key = getCacheKey(path);
    
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        auto it = m_audioCache.find(key);
        if (it != m_audioCache.end()) {
            m_stats.cacheHits++;
            return it->second;
        }
    }
    
    m_stats.cacheMisses++;
    
    // Audio loading would be implemented here
    // For now, return nullptr as AudioResource is not fully implemented
    log(LogLevel::Warning, "Audio loading not yet implemented");
    return nullptr;
}

void ResourceManager::preload(const std::vector<std::string>& paths, ResourceLoadCallback callback) {
    for (const auto& path : paths) {
        ResourceType type = detectResourceType(path);
        bool success = false;
        
        switch (type) {
            case ResourceType::Image:
                success = (loadImage(path) != nullptr);
                break;
            case ResourceType::Font:
                success = (loadFont(path) != nullptr);
                break;
            case ResourceType::Audio:
                success = (loadAudio(path) != nullptr);
                break;
            case ResourceType::Model:
                success = (loadModel(path) != nullptr);
                break;
            default:
                log(LogLevel::Warning, "Unknown resource type for: " + path);
                break;
        }
        
        if (callback) {
            callback(path, success);
        }
    }
}

void ResourceManager::preloadAsync(const std::vector<std::string>& paths, ResourceLoadCallback callback) {
    // For now, just call synchronous version
    // TODO: Implement async loading with thread pool
    preload(paths, callback);
}


// ============================================================================
// Cache Management
// ============================================================================

void ResourceManager::unload(const std::string& path) {
    std::string key = getCacheKey(path);
    
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    // Try to remove from all caches
    if (auto it = m_textureCache.find(key); it != m_textureCache.end()) {
        size_t memUsage = it->second->getWidth() * it->second->getHeight() * 4;
        untrackMemoryUsage(memUsage);
        untrackGPUMemoryUsage(memUsage);  // Textures also use GPU memory
        untrackResourceMetadata(key);
        m_stats.loadedImageCount--;
        m_textureCache.erase(it);
    }
    if (auto it = m_fontCache.find(key); it != m_fontCache.end()) {
        size_t memUsage = it->second->getAtlasWidth() * it->second->getAtlasHeight();
        untrackMemoryUsage(memUsage);
        untrackGPUMemoryUsage(memUsage);  // Font atlases also use GPU memory
        untrackResourceMetadata(key);
        m_stats.loadedFontCount--;
        m_fontCache.erase(it);
    }
    if (auto it = m_shaderCache.find(key); it != m_shaderCache.end()) {
        untrackResourceMetadata(key);
        m_stats.loadedShaderCount--;
        m_shaderCache.erase(it);
    }
    if (auto it = m_modelCache.find(key); it != m_modelCache.end()) {
        untrackResourceMetadata(key);
        m_stats.loadedModelCount--;
        m_modelCache.erase(it);
    }
    if (auto it = m_audioCache.find(key); it != m_audioCache.end()) {
        untrackResourceMetadata(key);
        m_stats.loadedAudioCount--;
        m_audioCache.erase(it);
    }
    
    // Stop watching file
    if (m_fileWatcher) {
        m_fileWatcher->unwatchFile(path);
    }
}

void ResourceManager::clearCache() {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    m_textureCache.clear();
    m_fontCache.clear();
    m_shaderCache.clear();
    m_modelCache.clear();
    m_audioCache.clear();
    
    // Clear resource metadata
    m_resourceMetadata.clear();
    
    m_currentMemoryUsage = 0;
    m_currentGPUMemoryUsage = 0;
    
    m_stats.loadedImageCount = 0;
    m_stats.loadedFontCount = 0;
    m_stats.loadedShaderCount = 0;
    m_stats.loadedModelCount = 0;
    m_stats.loadedAudioCount = 0;
    m_stats.cachedResourceCount = 0;
    
    log(LogLevel::Info, "Resource cache cleared");
}

bool ResourceManager::isCached(const std::string& path) const {
    std::string key = getCacheKey(path);
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    return m_textureCache.count(key) > 0 ||
           m_fontCache.count(key) > 0 ||
           m_shaderCache.count(key) > 0 ||
           m_modelCache.count(key) > 0 ||
           m_audioCache.count(key) > 0;
}

ResourceHandle ResourceManager::getCached(const std::string& path) const {
    // This would require a unified resource handle system
    // For now, return nullptr
    (void)path;  // Suppress unused parameter warning
    return nullptr;
}

// ============================================================================
// Memory Management
// ============================================================================

void ResourceManager::setMemoryLimit(size_t bytes) {
    m_memoryLimit = bytes;
    
    // If we're over the limit, evict resources
    if (bytes > 0 && m_currentMemoryUsage > bytes) {
        evictToLimit(bytes);
    }
}

size_t ResourceManager::getMemoryUsage() const {
    return m_currentMemoryUsage.load();
}

size_t ResourceManager::getGPUMemoryUsage() const {
    return m_currentGPUMemoryUsage.load();
}

int ResourceManager::gc() {
    int freed = 0;
    size_t freedBytes = 0;
    
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    // Remove textures with only one reference (the cache itself)
    for (auto it = m_textureCache.begin(); it != m_textureCache.end();) {
        if (it->second.use_count() == 1) {
            size_t memUsage = it->second->getWidth() * it->second->getHeight() * 4;
            untrackResourceMetadata(it->first);
            untrackMemoryUsage(memUsage);
            untrackGPUMemoryUsage(memUsage);  // Textures also use GPU memory
            freedBytes += memUsage;
            it = m_textureCache.erase(it);
            freed++;
            m_stats.loadedImageCount--;
        } else {
            ++it;
        }
    }
    
    // Remove fonts with only one reference
    for (auto it = m_fontCache.begin(); it != m_fontCache.end();) {
        if (it->second.use_count() == 1) {
            size_t memUsage = it->second->getAtlasWidth() * it->second->getAtlasHeight();
            untrackResourceMetadata(it->first);
            untrackMemoryUsage(memUsage);
            untrackGPUMemoryUsage(memUsage);  // Font atlases also use GPU memory
            freedBytes += memUsage;
            it = m_fontCache.erase(it);
            freed++;
            m_stats.loadedFontCount--;
        } else {
            ++it;
        }
    }
    
    // Remove shaders with only one reference
    for (auto it = m_shaderCache.begin(); it != m_shaderCache.end();) {
        if (it->second.use_count() == 1) {
            untrackResourceMetadata(it->first);
            it = m_shaderCache.erase(it);
            freed++;
            m_stats.loadedShaderCount--;
        } else {
            ++it;
        }
    }
    
    // Remove models with only one reference
    for (auto it = m_modelCache.begin(); it != m_modelCache.end();) {
        if (it->second.use_count() == 1) {
            untrackResourceMetadata(it->first);
            it = m_modelCache.erase(it);
            freed++;
            m_stats.loadedModelCount--;
        } else {
            ++it;
        }
    }
    
    // Remove audio with only one reference
    for (auto it = m_audioCache.begin(); it != m_audioCache.end();) {
        if (it->second.use_count() == 1) {
            untrackResourceMetadata(it->first);
            it = m_audioCache.erase(it);
            freed++;
            m_stats.loadedAudioCount--;
        } else {
            ++it;
        }
    }
    
    // Update GC freed bytes
    m_gcFreedBytes += freedBytes;
    
    // Update GC statistics
    m_stats.gcRunCount++;
    m_stats.gcFreedCount += freed;
    
    if (freed > 0) {
        log(LogLevel::Debug, "GC freed " + std::to_string(freed) + " resources");
    }
    
    return freed;
}


int ResourceManager::evictToLimit(size_t targetBytes) {
    int evicted = 0;
    size_t freedBytes = 0;
    
    // First try GC to free unreferenced resources
    evicted += gc();
    
    if (m_currentMemoryUsage <= targetBytes) {
        return evicted;
    }
    
    // Calculate how much memory we need to free
    size_t bytesToFree = m_currentMemoryUsage - targetBytes;
    
    // Get eviction candidates based on current policy
    auto candidates = getEvictionCandidates(bytesToFree);
    
    // Track bytes freed during eviction
    for (const auto& key : candidates) {
        freedBytes += getResourceMemoryUsage(key);
    }
    
    // Evict the candidates
    evicted += evictResources(candidates);
    
    // Update GC freed bytes statistic
    m_gcFreedBytes += freedBytes;
    
    return evicted;
}

void ResourceManager::setEvictionPolicy(EvictionPolicy policy) {
    m_evictionPolicy = policy;
    log(LogLevel::Info, "Eviction policy set to: " + 
        std::string(policy == EvictionPolicy::LRU ? "LRU" : 
                   (policy == EvictionPolicy::LFU ? "LFU" : "FIFO")));
}

float ResourceManager::getMemoryUsagePercent() const {
    size_t limit = m_memoryLimit.load();
    if (limit == 0) {
        return 0.0f;
    }
    return (static_cast<float>(m_currentMemoryUsage.load()) / static_cast<float>(limit)) * 100.0f;
}

int ResourceManager::enforceMemoryLimit() {
    size_t limit = m_memoryLimit.load();
    if (limit == 0 || m_currentMemoryUsage <= limit) {
        return 0;
    }
    return evictToLimit(limit);
}

void ResourceManager::trackResourceMetadata(const std::string& key, ResourceType type, size_t memoryUsage) {
    ResourceMetadata metadata;
    metadata.key = key;
    metadata.type = type;
    metadata.memoryUsage = memoryUsage;
    metadata.lastAccessTime = std::chrono::steady_clock::now();
    metadata.loadTime = std::chrono::steady_clock::now();
    metadata.accessCount = 1;
    
    m_resourceMetadata[key] = metadata;
}

void ResourceManager::untrackResourceMetadata(const std::string& key) {
    m_resourceMetadata.erase(key);
}

void ResourceManager::updateResourceAccessTime(const std::string& key) {
    auto it = m_resourceMetadata.find(key);
    if (it != m_resourceMetadata.end()) {
        it->second.lastAccessTime = std::chrono::steady_clock::now();
        it->second.accessCount++;
    }
}

std::vector<std::string> ResourceManager::getEvictionCandidates(size_t bytesToFree) const {
    std::vector<std::string> candidates;
    
    // Collect all resources with their metadata
    std::vector<std::pair<std::string, ResourceMetadata>> resources;
    for (const auto& [key, metadata] : m_resourceMetadata) {
        resources.emplace_back(key, metadata);
    }
    
    // Sort based on eviction policy
    switch (m_evictionPolicy) {
        case EvictionPolicy::LRU:
            // Sort by last access time (oldest first)
            std::sort(resources.begin(), resources.end(),
                [](const auto& a, const auto& b) {
                    return a.second.lastAccessTime < b.second.lastAccessTime;
                });
            break;
            
        case EvictionPolicy::LFU:
            // Sort by access count (least accessed first)
            std::sort(resources.begin(), resources.end(),
                [](const auto& a, const auto& b) {
                    return a.second.accessCount < b.second.accessCount;
                });
            break;
            
        case EvictionPolicy::FIFO:
            // Sort by load time (oldest loaded first)
            std::sort(resources.begin(), resources.end(),
                [](const auto& a, const auto& b) {
                    return a.second.loadTime < b.second.loadTime;
                });
            break;
    }
    
    // Select candidates until we have enough memory to free
    size_t totalToFree = 0;
    for (const auto& [key, metadata] : resources) {
        if (totalToFree >= bytesToFree) {
            break;
        }
        candidates.push_back(key);
        totalToFree += metadata.memoryUsage;
    }
    
    return candidates;
}

int ResourceManager::evictResources(const std::vector<std::string>& keys) {
    int evicted = 0;
    
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    for (const auto& key : keys) {
        bool found = false;
        
        // Try to find and remove from texture cache
        if (auto it = m_textureCache.find(key); it != m_textureCache.end()) {
            size_t memUsage = it->second->getWidth() * it->second->getHeight() * 4;
            m_textureCache.erase(it);
            untrackMemoryUsage(memUsage);
            untrackResourceMetadata(key);
            m_stats.loadedImageCount--;
            m_stats.evictionCount++;
            evicted++;
            found = true;
        }
        
        // Try to find and remove from font cache
        if (!found) {
            for (auto it = m_fontCache.begin(); it != m_fontCache.end(); ++it) {
                if (it->first.find(key) == 0 || it->first == key) {
                    size_t memUsage = it->second->getAtlasWidth() * it->second->getAtlasHeight();
                    m_fontCache.erase(it);
                    untrackMemoryUsage(memUsage);
                    untrackResourceMetadata(key);
                    m_stats.loadedFontCount--;
                    m_stats.evictionCount++;
                    evicted++;
                    found = true;
                    break;
                }
            }
        }
        
        // Try shader cache
        if (!found) {
            if (auto it = m_shaderCache.find(key); it != m_shaderCache.end()) {
                m_shaderCache.erase(it);
                untrackResourceMetadata(key);
                m_stats.loadedShaderCount--;
                m_stats.evictionCount++;
                evicted++;
                found = true;
            }
        }
        
        // Try model cache
        if (!found) {
            if (auto it = m_modelCache.find(key); it != m_modelCache.end()) {
                m_modelCache.erase(it);
                untrackResourceMetadata(key);
                m_stats.loadedModelCount--;
                m_stats.evictionCount++;
                evicted++;
                found = true;
            }
        }
        
        // Try audio cache
        if (!found) {
            if (auto it = m_audioCache.find(key); it != m_audioCache.end()) {
                m_audioCache.erase(it);
                untrackResourceMetadata(key);
                m_stats.loadedAudioCount--;
                m_stats.evictionCount++;
                evicted++;
            }
        }
    }
    
    if (evicted > 0) {
        log(LogLevel::Debug, "Evicted " + std::to_string(evicted) + " resources based on " +
            std::string(m_evictionPolicy == EvictionPolicy::LRU ? "LRU" : 
                       (m_evictionPolicy == EvictionPolicy::LFU ? "LFU" : "FIFO")) + " policy");
    }
    
    return evicted;
}

size_t ResourceManager::getResourceMemoryUsage(const std::string& key) const {
    auto it = m_resourceMetadata.find(key);
    if (it != m_resourceMetadata.end()) {
        return it->second.memoryUsage;
    }
    return 0;
}

// ============================================================================
// Hot Reload
// ============================================================================

void ResourceManager::enableHotReload(bool enabled) {
    m_hotReloadEnabled = enabled;
    
    if (m_fileWatcher) {
        m_fileWatcher->setEnabled(enabled);
    }
    
    log(LogLevel::Info, std::string("Hot reload ") + (enabled ? "enabled" : "disabled"));
}

void ResourceManager::watchDirectory(const std::string& path) {
    if (!m_fileWatcher) return;
    
    m_fileWatcher->watchDirectory(path, [this](const std::string& p) {
        onFileChanged(p);
    });
}

void ResourceManager::unwatchDirectory(const std::string& path) {
    if (!m_fileWatcher) return;
    
    m_fileWatcher->unwatchDirectory(path);
}

void ResourceManager::updateHotReload() {
    if (!m_hotReloadEnabled || !m_fileWatcher) return;
    
    m_fileWatcher->update();
}

void ResourceManager::setReloadCallback(ResourceLoadCallback callback) {
    m_reloadCallback = std::move(callback);
}

std::vector<std::string> ResourceManager::getWatchedDirectories() const {
    if (!m_fileWatcher) return {};
    return m_fileWatcher->getWatchedPaths();
}

bool ResourceManager::isPathWatched(const std::string& path) const {
    if (!m_fileWatcher) return false;
    return m_fileWatcher->isWatching(path);
}

bool ResourceManager::reloadResource(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        log(LogLevel::Warning, "Cannot reload resource - file not found: " + path);
        return false;
    }
    
    // Trigger the file change handler to reload the resource
    onFileChanged(path);
    
    // Check if the resource was successfully reloaded by verifying it's in cache
    return isCached(path);
}

void ResourceManager::onFileChanged(const std::string& path) {
    log(LogLevel::Info, "File changed: " + path);
    
    std::string key = getCacheKey(path);
    bool reloaded = false;
    ResourceType resourceType = detectResourceType(path);
    
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        switch (resourceType) {
            case ResourceType::Image: {
                // Check if it's a texture
                if (auto it = m_textureCache.find(key); it != m_textureCache.end()) {
                    // Reload texture
                    auto newTexture = Texture::loadFromFile(path);
                    if (newTexture) {
                        // Update memory tracking
                        size_t oldMemUsage = it->second->getWidth() * it->second->getHeight() * 4;
                        size_t newMemUsage = newTexture->getWidth() * newTexture->getHeight() * 4;
                        
                        it->second = newTexture;
                        
                        // Adjust memory usage
                        if (newMemUsage > oldMemUsage) {
                            m_currentMemoryUsage += (newMemUsage - oldMemUsage);
                        } else {
                            m_currentMemoryUsage -= (oldMemUsage - newMemUsage);
                        }
                        
                        // Update metadata
                        if (auto metaIt = m_resourceMetadata.find(key); metaIt != m_resourceMetadata.end()) {
                            metaIt->second.memoryUsage = newMemUsage;
                            metaIt->second.lastAccessTime = std::chrono::steady_clock::now();
                        }
                        
                        reloaded = true;
                        log(LogLevel::Info, "Reloaded texture: " + path);
                    } else {
                        log(LogLevel::Warning, "Failed to reload texture: " + path);
                    }
                }
                break;
            }
            
            case ResourceType::Font: {
                // Check fonts (need to match with size suffix)
                for (auto& [cacheKey, font] : m_fontCache) {
                    if (cacheKey.find(key) == 0) {
                        // Reload font with same config
                        FontConfig config;
                        config.size = font->getSize();
                        
                        size_t oldMemUsage = font->getAtlasWidth() * font->getAtlasHeight();
                        
                        auto newFont = Font::loadFromFile(path, config);
                        if (newFont) {
                            size_t newMemUsage = newFont->getAtlasWidth() * newFont->getAtlasHeight();
                            
                            font = newFont;
                            
                            // Adjust memory usage
                            if (newMemUsage > oldMemUsage) {
                                m_currentMemoryUsage += (newMemUsage - oldMemUsage);
                            } else {
                                m_currentMemoryUsage -= (oldMemUsage - newMemUsage);
                            }
                            
                            // Update metadata
                            if (auto metaIt = m_resourceMetadata.find(cacheKey); metaIt != m_resourceMetadata.end()) {
                                metaIt->second.memoryUsage = newMemUsage;
                                metaIt->second.lastAccessTime = std::chrono::steady_clock::now();
                            }
                            
                            reloaded = true;
                            log(LogLevel::Info, "Reloaded font: " + path);
                        } else {
                            log(LogLevel::Warning, "Failed to reload font: " + path);
                        }
                    }
                }
                break;
            }
            
            case ResourceType::Shader: {
                // Shader reload would be handled here when shader system is fully implemented
                log(LogLevel::Debug, "Shader hot reload not yet implemented for: " + path);
                break;
            }
            
            case ResourceType::Model: {
                // Model reload would be handled here when model system is fully implemented
                log(LogLevel::Debug, "Model hot reload not yet implemented for: " + path);
                break;
            }
            
            case ResourceType::Audio: {
                // Audio reload would be handled here when audio system is fully implemented
                log(LogLevel::Debug, "Audio hot reload not yet implemented for: " + path);
                break;
            }
            
            default:
                log(LogLevel::Debug, "Unknown resource type for hot reload: " + path);
                break;
        }
    }
    
    // Update hot reload statistics
    if (reloaded) {
        m_hotReloadCount++;
    }
    
    if (m_reloadCallback) {
        m_reloadCallback(path, reloaded);
    }
}


// ============================================================================
// Asset Bundling
// ============================================================================

BundleHandle ResourceManager::loadBundle(const std::string& path) {
    return AssetBundle::load(path);
}

BundleHandle ResourceManager::createBundle() {
    return AssetBundle::create();
}

void ResourceManager::mountBundle(BundleHandle bundle, const std::string& mountPoint) {
    if (!bundle) return;
    
    std::lock_guard<std::mutex> lock(m_bundleMutex);
    m_mountedBundles.emplace_back(mountPoint, bundle);
    
    log(LogLevel::Info, "Mounted bundle: " + bundle->getPath() + " at " + mountPoint);
}

void ResourceManager::unmountBundle(BundleHandle bundle) {
    if (!bundle) return;
    
    std::lock_guard<std::mutex> lock(m_bundleMutex);
    
    m_mountedBundles.erase(
        std::remove_if(m_mountedBundles.begin(), m_mountedBundles.end(),
            [&bundle](const auto& pair) { return pair.second == bundle; }),
        m_mountedBundles.end()
    );
    
    log(LogLevel::Info, "Unmounted bundle: " + bundle->getPath());
}

bool ResourceManager::existsInBundle(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_bundleMutex);
    
    for (const auto& [mountPoint, bundle] : m_mountedBundles) {
        std::string virtualPath = path;
        if (!mountPoint.empty() && path.find(mountPoint) == 0) {
            virtualPath = path.substr(mountPoint.length());
        }
        
        if (bundle->contains(virtualPath)) {
            return true;
        }
    }
    
    return false;
}

std::vector<uint8_t> ResourceManager::getBundleData(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_bundleMutex);
    
    for (const auto& [mountPoint, bundle] : m_mountedBundles) {
        std::string virtualPath = path;
        if (!mountPoint.empty() && path.find(mountPoint) == 0) {
            virtualPath = path.substr(mountPoint.length());
        }
        
        if (bundle->contains(virtualPath)) {
            return bundle->getData(virtualPath);
        }
    }
    
    return {};
}

bool ResourceManager::loadFromBundle(const std::string& path, std::vector<uint8_t>& data) const {
    data = getBundleData(path);
    return !data.empty();
}

// ============================================================================
// Statistics
// ============================================================================

ResourceStats ResourceManager::stats() const {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    
    ResourceStats result = m_stats;
    
    // Memory statistics
    result.totalMemoryUsage = m_currentMemoryUsage.load();
    result.gpuMemoryUsage = m_currentGPUMemoryUsage.load();
    result.peakMemoryUsage = m_peakMemoryUsage.load();
    result.peakGPUMemoryUsage = m_peakGPUMemoryUsage.load();
    result.memoryLimit = m_memoryLimit.load();
    
    // Calculate memory usage percentage
    if (result.memoryLimit > 0) {
        result.memoryUsagePercent = (static_cast<float>(result.totalMemoryUsage) / 
                                     static_cast<float>(result.memoryLimit)) * 100.0f;
    }
    
    // Resource counts
    result.cachedResourceCount = m_stats.loadedImageCount + m_stats.loadedFontCount +
                                  m_stats.loadedShaderCount + m_stats.loadedModelCount +
                                  m_stats.loadedAudioCount;
    
    // Cache hit rate
    size_t totalCacheAccesses = m_stats.cacheHits + m_stats.cacheMisses;
    if (totalCacheAccesses > 0) {
        result.cacheHitRate = static_cast<float>(m_stats.cacheHits) / 
                              static_cast<float>(totalCacheAccesses);
    }
    
    // GC statistics
    result.gcFreedBytes = m_gcFreedBytes.load();
    
    // Calculate load time statistics
    if (!m_loadTimes.empty()) {
        float sum = 0.0f;
        for (float t : m_loadTimes) {
            sum += t;
        }
        result.averageLoadTime = sum / m_loadTimes.size();
        result.totalLoadTime = sum;
        result.totalLoadsCount = m_loadTimes.size();
    }
    
    // Hot reload statistics
    result.hotReloadEnabled = m_hotReloadEnabled;
    if (m_fileWatcher) {
        result.watchedPathCount = m_fileWatcher->getWatchCount();
    }
    result.hotReloadCount = m_hotReloadCount.load();
    
    // Bundle statistics
    {
        std::lock_guard<std::mutex> bundleLock(m_bundleMutex);
        result.mountedBundleCount = m_mountedBundles.size();
        result.bundleTotalSize = 0;
        for (const auto& [mountPoint, bundle] : m_mountedBundles) {
            result.bundleTotalSize += bundle->getTotalSize();
        }
    }
    
    return result;
}

void ResourceManager::resetStats() {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    
    m_stats.cacheHits = 0;
    m_stats.cacheMisses = 0;
    m_stats.evictionCount = 0;
    m_loadTimes.clear();
}

std::unordered_map<ResourceType, size_t> ResourceManager::getMemoryBreakdown() const {
    std::unordered_map<ResourceType, size_t> breakdown;
    
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    // Calculate texture memory
    size_t textureMemory = 0;
    for (const auto& [key, texture] : m_textureCache) {
        textureMemory += texture->getWidth() * texture->getHeight() * 4;
    }
    breakdown[ResourceType::Image] = textureMemory;
    
    // Calculate font memory
    size_t fontMemory = 0;
    for (const auto& [key, font] : m_fontCache) {
        fontMemory += font->getAtlasWidth() * font->getAtlasHeight();
    }
    breakdown[ResourceType::Font] = fontMemory;
    
    // Other types would be calculated similarly
    breakdown[ResourceType::Shader] = 0;
    breakdown[ResourceType::Model] = 0;
    breakdown[ResourceType::Audio] = 0;
    
    return breakdown;
}

ResourceTypeStats ResourceManager::getResourceTypeStats(ResourceType type) const {
    ResourceTypeStats stats;
    
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    switch (type) {
        case ResourceType::Image: {
            stats.count = m_textureCache.size();
            for (const auto& [key, texture] : m_textureCache) {
                // CPU memory for texture data
                size_t cpuMem = texture->getWidth() * texture->getHeight() * 4;
                stats.cpuMemoryUsage += cpuMem;
                // GPU memory is typically the same for textures uploaded to GPU
                stats.gpuMemoryUsage += cpuMem;
            }
            stats.totalSize = stats.cpuMemoryUsage + stats.gpuMemoryUsage;
            break;
        }
        case ResourceType::Font: {
            stats.count = m_fontCache.size();
            for (const auto& [key, font] : m_fontCache) {
                // Font atlas memory
                size_t atlasMem = font->getAtlasWidth() * font->getAtlasHeight();
                stats.cpuMemoryUsage += atlasMem;
                // Font atlas is also uploaded to GPU
                stats.gpuMemoryUsage += atlasMem;
            }
            stats.totalSize = stats.cpuMemoryUsage + stats.gpuMemoryUsage;
            break;
        }
        case ResourceType::Shader: {
            stats.count = m_shaderCache.size();
            // Shader memory is primarily on GPU
            // Estimate based on typical shader sizes
            stats.gpuMemoryUsage = m_shaderCache.size() * 4096; // ~4KB per shader program
            stats.totalSize = stats.gpuMemoryUsage;
            break;
        }
        case ResourceType::Model: {
            stats.count = m_modelCache.size();
            // Model memory would be calculated from vertex/index buffers
            // For now, use metadata if available
            for (const auto& [key, metadata] : m_resourceMetadata) {
                if (metadata.type == ResourceType::Model) {
                    stats.cpuMemoryUsage += metadata.memoryUsage;
                }
            }
            stats.totalSize = stats.cpuMemoryUsage;
            break;
        }
        case ResourceType::Audio: {
            stats.count = m_audioCache.size();
            // Audio memory is typically CPU-side
            for (const auto& [key, metadata] : m_resourceMetadata) {
                if (metadata.type == ResourceType::Audio) {
                    stats.cpuMemoryUsage += metadata.memoryUsage;
                }
            }
            stats.totalSize = stats.cpuMemoryUsage;
            break;
        }
        default:
            break;
    }
    
    return stats;
}

std::unordered_map<ResourceType, ResourceTypeStats> ResourceManager::getAllResourceTypeStats() const {
    std::unordered_map<ResourceType, ResourceTypeStats> allStats;
    
    allStats[ResourceType::Image] = getResourceTypeStats(ResourceType::Image);
    allStats[ResourceType::Font] = getResourceTypeStats(ResourceType::Font);
    allStats[ResourceType::Shader] = getResourceTypeStats(ResourceType::Shader);
    allStats[ResourceType::Model] = getResourceTypeStats(ResourceType::Model);
    allStats[ResourceType::Audio] = getResourceTypeStats(ResourceType::Audio);
    
    return allStats;
}

void ResourceManager::resetPeakMemory() {
    m_peakMemoryUsage = m_currentMemoryUsage.load();
    m_peakGPUMemoryUsage = m_currentGPUMemoryUsage.load();
    log(LogLevel::Debug, "Peak memory tracking reset");
}

std::string ResourceManager::getStatsString() const {
    ResourceStats s = stats();
    std::ostringstream oss;
    
    oss << "=== Resource Manager Statistics ===\n";
    
    // Memory section
    oss << "\n[Memory]\n";
    oss << "  CPU Memory: " << (s.totalMemoryUsage / 1024) << " KB";
    if (s.memoryLimit > 0) {
        oss << " / " << (s.memoryLimit / 1024) << " KB (" 
            << std::fixed << std::setprecision(1) << s.memoryUsagePercent << "%)";
    }
    oss << "\n";
    oss << "  GPU Memory: " << (s.gpuMemoryUsage / 1024) << " KB\n";
    oss << "  Peak CPU: " << (s.peakMemoryUsage / 1024) << " KB\n";
    oss << "  Peak GPU: " << (s.peakGPUMemoryUsage / 1024) << " KB\n";
    
    // Resources section
    oss << "\n[Resources] (Total: " << s.cachedResourceCount << ")\n";
    oss << "  Images: " << s.loadedImageCount << "\n";
    oss << "  Fonts: " << s.loadedFontCount << "\n";
    oss << "  Shaders: " << s.loadedShaderCount << "\n";
    oss << "  Models: " << s.loadedModelCount << "\n";
    oss << "  Audio: " << s.loadedAudioCount << "\n";
    
    // Cache section
    oss << "\n[Cache]\n";
    oss << "  Hits: " << s.cacheHits << "\n";
    oss << "  Misses: " << s.cacheMisses << "\n";
    oss << "  Hit Rate: " << std::fixed << std::setprecision(1) 
        << (s.cacheHitRate * 100.0f) << "%\n";
    oss << "  Evictions: " << s.evictionCount << "\n";
    
    // GC section
    oss << "\n[Garbage Collection]\n";
    oss << "  GC Runs: " << s.gcRunCount << "\n";
    oss << "  Resources Freed: " << s.gcFreedCount << "\n";
    oss << "  Bytes Freed: " << (s.gcFreedBytes / 1024) << " KB\n";
    
    // Performance section
    oss << "\n[Performance]\n";
    oss << "  Total Loads: " << s.totalLoadsCount << "\n";
    oss << "  Avg Load Time: " << std::fixed << std::setprecision(2) 
        << s.averageLoadTime << " ms\n";
    oss << "  Total Load Time: " << std::fixed << std::setprecision(2) 
        << s.totalLoadTime << " ms\n";
    
    // Hot Reload section
    oss << "\n[Hot Reload]\n";
    oss << "  Enabled: " << (s.hotReloadEnabled ? "Yes" : "No") << "\n";
    oss << "  Watched Paths: " << s.watchedPathCount << "\n";
    oss << "  Reloads: " << s.hotReloadCount << "\n";
    
    // Bundles section
    oss << "\n[Bundles]\n";
    oss << "  Mounted: " << s.mountedBundleCount << "\n";
    oss << "  Total Size: " << (s.bundleTotalSize / 1024) << " KB\n";
    
    return oss.str();
}


// ============================================================================
// Internal Helpers
// ============================================================================

ResourceType ResourceManager::detectResourceType(const std::string& path) const {
    std::string ext = path;
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        ext = path.substr(dotPos + 1);
    }
    
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // Image formats
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || 
        ext == "tga" || ext == "ico" || ext == "svg") {
        return ResourceType::Image;
    }
    
    // Font formats
    if (ext == "ttf" || ext == "otf" || ext == "woff" || ext == "woff2") {
        return ResourceType::Font;
    }
    
    // Audio formats
    if (ext == "wav" || ext == "mp3" || ext == "ogg" || ext == "flac") {
        return ResourceType::Audio;
    }
    
    // Shader formats
    if (ext == "vert" || ext == "frag" || ext == "glsl" || ext == "spv") {
        return ResourceType::Shader;
    }
    
    // Model formats
    if (ext == "obj" || ext == "fbx" || ext == "gltf" || ext == "glb") {
        return ResourceType::Model;
    }
    
    // Bundle format
    if (ext == "kgkb") {
        return ResourceType::Bundle;
    }
    
    return ResourceType::Unknown;
}

std::string ResourceManager::normalizePath(const std::string& path) const {
    std::string normalized = path;
    
    // Replace backslashes with forward slashes
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    // Remove duplicate slashes
    std::string result;
    bool lastWasSlash = false;
    for (char c : normalized) {
        if (c == '/') {
            if (!lastWasSlash) {
                result += c;
                lastWasSlash = true;
            }
        } else {
            result += c;
            lastWasSlash = false;
        }
    }
    
    return result;
}

std::string ResourceManager::getCacheKey(const std::string& path, const std::string& suffix) const {
    return normalizePath(path) + suffix;
}

void ResourceManager::trackMemoryUsage(size_t bytes) {
    m_currentMemoryUsage += bytes;
    
    // Update peak memory usage
    size_t current = m_currentMemoryUsage.load();
    size_t peak = m_peakMemoryUsage.load();
    while (current > peak && !m_peakMemoryUsage.compare_exchange_weak(peak, current)) {
        // Retry if another thread updated peak
    }
    
    // Check if we need to evict
    size_t limit = m_memoryLimit.load();
    if (limit > 0 && m_currentMemoryUsage > limit) {
        evictToLimit(limit);
    }
}

void ResourceManager::trackGPUMemoryUsage(size_t bytes) {
    m_currentGPUMemoryUsage += bytes;
    
    // Update peak GPU memory usage
    size_t current = m_currentGPUMemoryUsage.load();
    size_t peak = m_peakGPUMemoryUsage.load();
    while (current > peak && !m_peakGPUMemoryUsage.compare_exchange_weak(peak, current)) {
        // Retry if another thread updated peak
    }
}

void ResourceManager::untrackGPUMemoryUsage(size_t bytes) {
    if (bytes > m_currentGPUMemoryUsage) {
        m_currentGPUMemoryUsage = 0;
    } else {
        m_currentGPUMemoryUsage -= bytes;
    }
}

void ResourceManager::untrackMemoryUsage(size_t bytes) {
    if (bytes > m_currentMemoryUsage) {
        m_currentMemoryUsage = 0;
    } else {
        m_currentMemoryUsage -= bytes;
    }
}

} // namespace KillerGK
