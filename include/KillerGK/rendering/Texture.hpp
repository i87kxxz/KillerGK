/**
 * @file Texture.hpp
 * @brief Texture loading and management for KillerGK
 * 
 * Provides texture loading from various image formats (PNG, JPG, BMP, ICO, SVG)
 * and GPU texture creation for Vulkan rendering.
 */

#pragma once

#include "../core/Types.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace KillerGK {

/**
 * @brief Supported image formats
 */
enum class ImageFormat {
    Unknown,
    PNG,
    JPG,
    BMP,
    TGA,
    ICO,
    SVG
};

/**
 * @brief Texture filtering mode
 */
enum class TextureFilter {
    Nearest,
    Linear
};

/**
 * @brief Texture wrapping mode
 */
enum class TextureWrap {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

/**
 * @brief Texture configuration
 */
struct TextureConfig {
    TextureFilter minFilter = TextureFilter::Linear;
    TextureFilter magFilter = TextureFilter::Linear;
    TextureWrap wrapU = TextureWrap::Repeat;
    TextureWrap wrapV = TextureWrap::Repeat;
    bool generateMipmaps = true;
    bool flipVertically = false;
};

/**
 * @brief Raw image data loaded from file
 */
struct ImageData {
    std::vector<uint8_t> pixels;
    int width = 0;
    int height = 0;
    int channels = 0;
    ImageFormat format = ImageFormat::Unknown;
    
    [[nodiscard]] bool isValid() const {
        return !pixels.empty() && width > 0 && height > 0 && channels > 0;
    }
    
    [[nodiscard]] size_t sizeBytes() const {
        return static_cast<size_t>(width) * height * channels;
    }
};

/**
 * @brief Handle to a texture
 */
using TextureHandle = std::shared_ptr<class Texture>;

/**
 * @class Texture
 * @brief GPU texture with Vulkan resources
 */
class Texture {
public:
    ~Texture();
    
    /**
     * @brief Load texture from file
     * @param path File path
     * @param config Texture configuration
     * @return Texture handle or nullptr on failure
     */
    static TextureHandle loadFromFile(const std::string& path, 
                                       const TextureConfig& config = TextureConfig{});
    
    /**
     * @brief Load texture from memory
     * @param data Raw file data
     * @param size Data size in bytes
     * @param config Texture configuration
     * @return Texture handle or nullptr on failure
     */
    static TextureHandle loadFromMemory(const uint8_t* data, size_t size,
                                         const TextureConfig& config = TextureConfig{});
    
    /**
     * @brief Create texture from raw pixel data
     * @param pixels RGBA pixel data
     * @param width Image width
     * @param height Image height
     * @param config Texture configuration
     * @return Texture handle or nullptr on failure
     */
    static TextureHandle createFromPixels(const uint8_t* pixels, int width, int height,
                                           const TextureConfig& config = TextureConfig{});
    
    // Getters
    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }
    [[nodiscard]] int getChannels() const { return m_channels; }
    [[nodiscard]] VkImage getImage() const { return m_image; }
    [[nodiscard]] VkImageView getImageView() const { return m_imageView; }
    [[nodiscard]] VkSampler getSampler() const { return m_sampler; }
    [[nodiscard]] VkDescriptorImageInfo getDescriptorInfo() const;
    [[nodiscard]] const std::string& getPath() const { return m_path; }
    
private:
    Texture() = default;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    
    bool createGPUResources(const ImageData& imageData, const TextureConfig& config);
    bool createImage(const ImageData& imageData);
    bool createImageView();
    bool createSampler(const TextureConfig& config);
    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);
    
    std::string m_path;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
    
    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
};

/**
 * @class ImageLoader
 * @brief Static utility class for loading images from various formats
 */
class ImageLoader {
public:
    /**
     * @brief Load image from file
     * @param path File path
     * @param flipVertically Whether to flip the image vertically
     * @return ImageData with loaded pixels
     */
    static ImageData loadFromFile(const std::string& path, bool flipVertically = false);
    
    /**
     * @brief Load image from memory
     * @param data Raw file data
     * @param size Data size in bytes
     * @param flipVertically Whether to flip the image vertically
     * @return ImageData with loaded pixels
     */
    static ImageData loadFromMemory(const uint8_t* data, size_t size, bool flipVertically = false);
    
    /**
     * @brief Detect image format from file extension
     * @param path File path
     * @return Detected format
     */
    static ImageFormat detectFormat(const std::string& path);
    
    /**
     * @brief Detect image format from file header
     * @param data File data
     * @param size Data size
     * @return Detected format
     */
    static ImageFormat detectFormatFromHeader(const uint8_t* data, size_t size);
    
    /**
     * @brief Check if a format is supported
     * @param format Image format
     * @return true if supported
     */
    static bool isFormatSupported(ImageFormat format);
    
private:
    static ImageData loadPNG(const std::string& path, bool flipVertically);
    static ImageData loadJPG(const std::string& path, bool flipVertically);
    static ImageData loadBMP(const std::string& path, bool flipVertically);
    static ImageData loadTGA(const std::string& path, bool flipVertically);
    static ImageData loadICO(const std::string& path, bool flipVertically);
    static ImageData loadSVG(const std::string& path, bool flipVertically);
    
    static ImageData loadFromMemorySTB(const uint8_t* data, size_t size, bool flipVertically);
    static ImageData loadICOFromMemory(const uint8_t* data, size_t size, bool flipVertically);
    static ImageData loadSVGFromMemory(const uint8_t* data, size_t size, bool flipVertically);
};

} // namespace KillerGK
