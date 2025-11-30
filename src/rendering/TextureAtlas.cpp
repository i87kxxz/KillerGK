/**
 * @file TextureAtlas.cpp
 * @brief Texture atlas management implementation
 */

#include "KillerGK/rendering/TextureAtlas.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace KillerGK {

// ============================================================================
// TextureAtlas Implementation
// ============================================================================

TextureAtlas::~TextureAtlas() {
    clear();
}

TextureAtlasHandle TextureAtlas::create(const TextureAtlasConfig& config) {
    if (config.width <= 0 || config.height <= 0) {
        std::cerr << "[TextureAtlas] Invalid atlas dimensions" << std::endl;
        return nullptr;
    }
    
    auto atlas = std::shared_ptr<TextureAtlas>(new TextureAtlas());
    atlas->m_config = config;
    atlas->m_root = std::make_unique<PackNode>(0, 0, config.width, config.height);
    atlas->m_atlasPixels.resize(static_cast<size_t>(config.width) * config.height * 4, 0);
    
    return atlas;
}

bool TextureAtlas::addImage(const std::string& name, const std::string& path) {
    if (m_built) {
        std::cerr << "[TextureAtlas] Cannot add images after atlas is built" << std::endl;
        return false;
    }
    
    if (m_regions.find(name) != m_regions.end()) {
        std::cerr << "[TextureAtlas] Region already exists: " << name << std::endl;
        return false;
    }
    
    ImageData imageData = ImageLoader::loadFromFile(path, false);
    if (!imageData.isValid()) {
        std::cerr << "[TextureAtlas] Failed to load image: " << path << std::endl;
        return false;
    }
    
    return addImage(name, imageData);
}

bool TextureAtlas::addImage(const std::string& name, const uint8_t* pixels, int width, int height) {
    if (m_built) {
        std::cerr << "[TextureAtlas] Cannot add images after atlas is built" << std::endl;
        return false;
    }
    
    if (!pixels || width <= 0 || height <= 0) {
        return false;
    }
    
    ImageData imageData;
    imageData.width = width;
    imageData.height = height;
    imageData.channels = 4;
    imageData.pixels.resize(static_cast<size_t>(width) * height * 4);
    std::memcpy(imageData.pixels.data(), pixels, imageData.pixels.size());
    
    return addImage(name, imageData);
}

bool TextureAtlas::addImage(const std::string& name, const ImageData& imageData) {
    if (m_built) {
        std::cerr << "[TextureAtlas] Cannot add images after atlas is built" << std::endl;
        return false;
    }
    
    if (m_regions.find(name) != m_regions.end()) {
        std::cerr << "[TextureAtlas] Region already exists: " << name << std::endl;
        return false;
    }
    
    if (!imageData.isValid()) {
        return false;
    }
    
    // Store pending image for later packing
    PendingImage pending;
    pending.name = name;
    pending.data = imageData;
    
    // Convert to RGBA if needed
    if (imageData.channels != 4) {
        pending.data.pixels.resize(static_cast<size_t>(imageData.width) * imageData.height * 4);
        pending.data.channels = 4;
        
        for (int y = 0; y < imageData.height; y++) {
            for (int x = 0; x < imageData.width; x++) {
                size_t srcIdx = (y * imageData.width + x) * imageData.channels;
                size_t dstIdx = (y * imageData.width + x) * 4;
                
                switch (imageData.channels) {
                    case 1:
                        pending.data.pixels[dstIdx + 0] = imageData.pixels[srcIdx];
                        pending.data.pixels[dstIdx + 1] = imageData.pixels[srcIdx];
                        pending.data.pixels[dstIdx + 2] = imageData.pixels[srcIdx];
                        pending.data.pixels[dstIdx + 3] = 255;
                        break;
                    case 2:
                        pending.data.pixels[dstIdx + 0] = imageData.pixels[srcIdx];
                        pending.data.pixels[dstIdx + 1] = imageData.pixels[srcIdx];
                        pending.data.pixels[dstIdx + 2] = imageData.pixels[srcIdx];
                        pending.data.pixels[dstIdx + 3] = imageData.pixels[srcIdx + 1];
                        break;
                    case 3:
                        pending.data.pixels[dstIdx + 0] = imageData.pixels[srcIdx + 0];
                        pending.data.pixels[dstIdx + 1] = imageData.pixels[srcIdx + 1];
                        pending.data.pixels[dstIdx + 2] = imageData.pixels[srcIdx + 2];
                        pending.data.pixels[dstIdx + 3] = 255;
                        break;
                    default:
                        std::memcpy(&pending.data.pixels[dstIdx], &imageData.pixels[srcIdx], 4);
                        break;
                }
            }
        }
    }
    
    m_pendingImages.push_back(std::move(pending));
    return true;
}

bool TextureAtlas::build() {
    if (m_built) {
        std::cerr << "[TextureAtlas] Atlas already built" << std::endl;
        return false;
    }
    
    if (m_pendingImages.empty()) {
        std::cerr << "[TextureAtlas] No images to pack" << std::endl;
        return false;
    }
    
    // Sort images by height (descending) for better packing
    std::sort(m_pendingImages.begin(), m_pendingImages.end(),
              [](const PendingImage& a, const PendingImage& b) {
                  return a.data.height > b.data.height;
              });
    
    // Reset the packing tree
    m_root = std::make_unique<PackNode>(0, 0, m_config.width, m_config.height);
    
    // Clear atlas pixels
    std::fill(m_atlasPixels.begin(), m_atlasPixels.end(), 0);
    
    // Pack each image
    for (auto& pending : m_pendingImages) {
        int paddedWidth = pending.data.width + m_config.padding * 2;
        int paddedHeight = pending.data.height + m_config.padding * 2;
        
        PackNode* node = findNode(m_root.get(), paddedWidth, paddedHeight);
        if (!node) {
            std::cerr << "[TextureAtlas] Failed to pack image: " << pending.name 
                      << " (" << pending.data.width << "x" << pending.data.height << ")" << std::endl;
            return false;
        }
        
        node = splitNode(node, paddedWidth, paddedHeight);
        
        // Create region (without padding in the region coordinates)
        AtlasRegion region;
        region.name = pending.name;
        region.x = node->x + m_config.padding;
        region.y = node->y + m_config.padding;
        region.width = pending.data.width;
        region.height = pending.data.height;
        
        m_regions[pending.name] = region;
        
        // Copy pixels to atlas
        for (int y = 0; y < pending.data.height; y++) {
            int srcOffset = y * pending.data.width * 4;
            int dstOffset = ((region.y + y) * m_config.width + region.x) * 4;
            std::memcpy(&m_atlasPixels[dstOffset], 
                       &pending.data.pixels[srcOffset], 
                       pending.data.width * 4);
        }
    }
    
    // Calculate UV coordinates
    calculateUVs();
    
    // Create GPU texture
    TextureConfig texConfig;
    texConfig.minFilter = m_config.filter;
    texConfig.magFilter = m_config.filter;
    texConfig.wrapU = TextureWrap::ClampToEdge;
    texConfig.wrapV = TextureWrap::ClampToEdge;
    texConfig.generateMipmaps = false;
    
    m_texture = Texture::createFromPixels(m_atlasPixels.data(), 
                                           m_config.width, m_config.height, texConfig);
    if (!m_texture) {
        std::cerr << "[TextureAtlas] Failed to create atlas texture" << std::endl;
        return false;
    }
    
    // Clear pending images to free memory
    m_pendingImages.clear();
    m_pendingImages.shrink_to_fit();
    
    m_built = true;
    return true;
}

TextureAtlas::PackNode* TextureAtlas::findNode(PackNode* node, int width, int height) {
    if (!node) {
        return nullptr;
    }
    
    if (node->used) {
        // Try right child first, then down
        PackNode* result = findNode(node->right.get(), width, height);
        if (result) return result;
        return findNode(node->left.get(), width, height);
    }
    
    // Check if image fits
    if (width <= node->width && height <= node->height) {
        return node;
    }
    
    return nullptr;
}

TextureAtlas::PackNode* TextureAtlas::splitNode(PackNode* node, int width, int height) {
    node->used = true;
    
    // Create child nodes
    // Right node: remaining space to the right
    node->right = std::make_unique<PackNode>(
        node->x + width, node->y,
        node->width - width, height
    );
    
    // Down node: remaining space below
    node->left = std::make_unique<PackNode>(
        node->x, node->y + height,
        node->width, node->height - height
    );
    
    return node;
}

void TextureAtlas::calculateUVs() {
    float invWidth = 1.0f / static_cast<float>(m_config.width);
    float invHeight = 1.0f / static_cast<float>(m_config.height);
    
    for (auto& [name, region] : m_regions) {
        region.u0 = static_cast<float>(region.x) * invWidth;
        region.v0 = static_cast<float>(region.y) * invHeight;
        region.u1 = static_cast<float>(region.x + region.width) * invWidth;
        region.v1 = static_cast<float>(region.y + region.height) * invHeight;
    }
}

AtlasRegion TextureAtlas::getRegion(const std::string& name) const {
    auto it = m_regions.find(name);
    if (it != m_regions.end()) {
        return it->second;
    }
    return AtlasRegion{};
}

bool TextureAtlas::hasRegion(const std::string& name) const {
    return m_regions.find(name) != m_regions.end();
}

std::vector<std::string> TextureAtlas::getRegionNames() const {
    std::vector<std::string> names;
    names.reserve(m_regions.size());
    for (const auto& [name, region] : m_regions) {
        names.push_back(name);
    }
    return names;
}

void TextureAtlas::clear() {
    m_regions.clear();
    m_pendingImages.clear();
    m_atlasPixels.clear();
    m_texture.reset();
    m_root.reset();
    m_built = false;
    
    // Reinitialize
    if (m_config.width > 0 && m_config.height > 0) {
        m_root = std::make_unique<PackNode>(0, 0, m_config.width, m_config.height);
        m_atlasPixels.resize(static_cast<size_t>(m_config.width) * m_config.height * 4, 0);
    }
}

// ============================================================================
// TextureAtlasManager Implementation
// ============================================================================

TextureAtlasManager& TextureAtlasManager::instance() {
    static TextureAtlasManager instance;
    return instance;
}

TextureAtlasHandle TextureAtlasManager::createAtlas(const std::string& name, 
                                                     const TextureAtlasConfig& config) {
    if (m_atlases.find(name) != m_atlases.end()) {
        std::cerr << "[TextureAtlasManager] Atlas already exists: " << name << std::endl;
        return nullptr;
    }
    
    auto atlas = TextureAtlas::create(config);
    if (atlas) {
        m_atlases[name] = atlas;
    }
    return atlas;
}

TextureAtlasHandle TextureAtlasManager::getAtlas(const std::string& name) const {
    auto it = m_atlases.find(name);
    if (it != m_atlases.end()) {
        return it->second;
    }
    return nullptr;
}

void TextureAtlasManager::removeAtlas(const std::string& name) {
    m_atlases.erase(name);
}

void TextureAtlasManager::clear() {
    m_atlases.clear();
}

std::vector<std::string> TextureAtlasManager::getAtlasNames() const {
    std::vector<std::string> names;
    names.reserve(m_atlases.size());
    for (const auto& [name, atlas] : m_atlases) {
        names.push_back(name);
    }
    return names;
}

} // namespace KillerGK
