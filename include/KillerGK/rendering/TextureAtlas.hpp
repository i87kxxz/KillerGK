/**
 * @file TextureAtlas.hpp
 * @brief Texture atlas management for KillerGK
 * 
 * Provides texture packing and atlas management for efficient
 * batch rendering with minimal texture binds.
 */

#pragma once

#include "../core/Types.hpp"
#include "Texture.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace KillerGK {

/**
 * @brief Region within a texture atlas
 */
struct AtlasRegion {
    std::string name;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    
    // UV coordinates (normalized 0-1)
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
    
    [[nodiscard]] bool isValid() const {
        return width > 0 && height > 0;
    }
    
    [[nodiscard]] Rect toRect() const {
        return Rect(static_cast<float>(x), static_cast<float>(y), 
                    static_cast<float>(width), static_cast<float>(height));
    }
};

/**
 * @brief Configuration for texture atlas
 */
struct TextureAtlasConfig {
    int width = 2048;
    int height = 2048;
    int padding = 2;           // Padding between regions to prevent bleeding
    bool allowGrowth = false;  // Allow atlas to grow if full
    TextureFilter filter = TextureFilter::Linear;
};

/**
 * @brief Handle to a texture atlas
 */
using TextureAtlasHandle = std::shared_ptr<class TextureAtlas>;

/**
 * @class TextureAtlas
 * @brief Manages a texture atlas with automatic packing
 * 
 * Uses a binary tree packing algorithm to efficiently pack
 * multiple images into a single texture.
 */
class TextureAtlas {
public:
    ~TextureAtlas();
    
    /**
     * @brief Create a new texture atlas
     * @param config Atlas configuration
     * @return Atlas handle or nullptr on failure
     */
    static TextureAtlasHandle create(const TextureAtlasConfig& config = TextureAtlasConfig{});
    
    /**
     * @brief Add an image to the atlas from file
     * @param name Unique name for the region
     * @param path Image file path
     * @return true if added successfully
     */
    bool addImage(const std::string& name, const std::string& path);
    
    /**
     * @brief Add an image to the atlas from memory
     * @param name Unique name for the region
     * @param pixels RGBA pixel data
     * @param width Image width
     * @param height Image height
     * @return true if added successfully
     */
    bool addImage(const std::string& name, const uint8_t* pixels, int width, int height);
    
    /**
     * @brief Add an image to the atlas from ImageData
     * @param name Unique name for the region
     * @param imageData Image data
     * @return true if added successfully
     */
    bool addImage(const std::string& name, const ImageData& imageData);
    
    /**
     * @brief Build the atlas texture (must be called after adding all images)
     * @return true if build succeeded
     */
    bool build();
    
    /**
     * @brief Check if the atlas has been built
     */
    [[nodiscard]] bool isBuilt() const { return m_built; }
    
    /**
     * @brief Get a region by name
     * @param name Region name
     * @return Region or invalid region if not found
     */
    [[nodiscard]] AtlasRegion getRegion(const std::string& name) const;
    
    /**
     * @brief Check if a region exists
     * @param name Region name
     * @return true if region exists
     */
    [[nodiscard]] bool hasRegion(const std::string& name) const;
    
    /**
     * @brief Get all region names
     */
    [[nodiscard]] std::vector<std::string> getRegionNames() const;
    
    /**
     * @brief Get the atlas texture
     */
    [[nodiscard]] TextureHandle getTexture() const { return m_texture; }
    
    /**
     * @brief Get atlas dimensions
     */
    [[nodiscard]] int getWidth() const { return m_config.width; }
    [[nodiscard]] int getHeight() const { return m_config.height; }
    
    /**
     * @brief Get number of regions
     */
    [[nodiscard]] size_t getRegionCount() const { return m_regions.size(); }
    
    /**
     * @brief Clear all regions and reset the atlas
     */
    void clear();

private:
    TextureAtlas() = default;
    TextureAtlas(const TextureAtlas&) = delete;
    TextureAtlas& operator=(const TextureAtlas&) = delete;
    
    // Binary tree node for packing
    struct PackNode {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        bool used = false;
        std::unique_ptr<PackNode> left;
        std::unique_ptr<PackNode> right;
        
        PackNode() = default;
        PackNode(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
    };
    
    // Pending image to be packed
    struct PendingImage {
        std::string name;
        ImageData data;
    };
    
    PackNode* findNode(PackNode* node, int width, int height);
    PackNode* splitNode(PackNode* node, int width, int height);
    void calculateUVs();
    
    TextureAtlasConfig m_config;
    std::unique_ptr<PackNode> m_root;
    std::vector<PendingImage> m_pendingImages;
    std::unordered_map<std::string, AtlasRegion> m_regions;
    std::vector<uint8_t> m_atlasPixels;
    TextureHandle m_texture;
    bool m_built = false;
};

/**
 * @class TextureAtlasManager
 * @brief Manages multiple texture atlases
 */
class TextureAtlasManager {
public:
    /**
     * @brief Get the singleton instance
     */
    static TextureAtlasManager& instance();
    
    /**
     * @brief Create a new atlas
     * @param name Atlas name
     * @param config Atlas configuration
     * @return Atlas handle or nullptr on failure
     */
    TextureAtlasHandle createAtlas(const std::string& name, 
                                    const TextureAtlasConfig& config = TextureAtlasConfig{});
    
    /**
     * @brief Get an atlas by name
     * @param name Atlas name
     * @return Atlas handle or nullptr if not found
     */
    TextureAtlasHandle getAtlas(const std::string& name) const;
    
    /**
     * @brief Remove an atlas
     * @param name Atlas name
     */
    void removeAtlas(const std::string& name);
    
    /**
     * @brief Clear all atlases
     */
    void clear();
    
    /**
     * @brief Get all atlas names
     */
    std::vector<std::string> getAtlasNames() const;

private:
    TextureAtlasManager() = default;
    ~TextureAtlasManager() = default;
    TextureAtlasManager(const TextureAtlasManager&) = delete;
    TextureAtlasManager& operator=(const TextureAtlasManager&) = delete;
    
    std::unordered_map<std::string, TextureAtlasHandle> m_atlases;
};

} // namespace KillerGK
