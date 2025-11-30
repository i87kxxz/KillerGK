/**
 * @file Texture.cpp
 * @brief Texture loading and management implementation
 */

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"

#include "KillerGK/rendering/Texture.hpp"
#include "KillerGK/rendering/VulkanBackend.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

namespace KillerGK {

// Helper to find memory type
static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, 
                                VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    return 0;
}

// Helper to create a buffer
static bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                         VkMemoryPropertyFlags properties,
                         VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkDevice device = VulkanBackend::instance().getDevice();
    VkPhysicalDevice physicalDevice = VulkanBackend::instance().getPhysicalDevice();
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        return false;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        vkDestroyBuffer(device, buffer, nullptr);
        return false;
    }
    
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
    return true;
}

// ============================================================================
// Texture Implementation
// ============================================================================

Texture::~Texture() {
    VkDevice device = VulkanBackend::instance().getDevice();
    
    if (device != VK_NULL_HANDLE) {
        VulkanBackend::instance().waitIdle();
        
        if (m_sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device, m_sampler, nullptr);
        }
        if (m_imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, m_imageView, nullptr);
        }
        if (m_image != VK_NULL_HANDLE) {
            vkDestroyImage(device, m_image, nullptr);
        }
        if (m_imageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, m_imageMemory, nullptr);
        }
    }
}

TextureHandle Texture::loadFromFile(const std::string& path, const TextureConfig& config) {
    ImageData imageData = ImageLoader::loadFromFile(path, config.flipVertically);
    if (!imageData.isValid()) {
        std::cerr << "[Texture] Failed to load image: " << path << std::endl;
        return nullptr;
    }
    
    auto texture = std::shared_ptr<Texture>(new Texture());
    texture->m_path = path;
    
    if (!texture->createGPUResources(imageData, config)) {
        std::cerr << "[Texture] Failed to create GPU resources for: " << path << std::endl;
        return nullptr;
    }
    
    return texture;
}

TextureHandle Texture::loadFromMemory(const uint8_t* data, size_t size, const TextureConfig& config) {
    ImageData imageData = ImageLoader::loadFromMemory(data, size, config.flipVertically);
    if (!imageData.isValid()) {
        std::cerr << "[Texture] Failed to load image from memory" << std::endl;
        return nullptr;
    }
    
    auto texture = std::shared_ptr<Texture>(new Texture());
    
    if (!texture->createGPUResources(imageData, config)) {
        std::cerr << "[Texture] Failed to create GPU resources from memory" << std::endl;
        return nullptr;
    }
    
    return texture;
}

TextureHandle Texture::createFromPixels(const uint8_t* pixels, int width, int height,
                                         const TextureConfig& config) {
    if (!pixels || width <= 0 || height <= 0) {
        return nullptr;
    }
    
    ImageData imageData;
    imageData.width = width;
    imageData.height = height;
    imageData.channels = 4; // Assume RGBA
    imageData.pixels.resize(static_cast<size_t>(width) * height * 4);
    std::memcpy(imageData.pixels.data(), pixels, imageData.pixels.size());
    
    auto texture = std::shared_ptr<Texture>(new Texture());
    
    if (!texture->createGPUResources(imageData, config)) {
        return nullptr;
    }
    
    return texture;
}

bool Texture::createGPUResources(const ImageData& imageData, const TextureConfig& config) {
    m_width = imageData.width;
    m_height = imageData.height;
    m_channels = imageData.channels;
    
    if (!createImage(imageData)) {
        return false;
    }
    
    if (!createImageView()) {
        return false;
    }
    
    if (!createSampler(config)) {
        return false;
    }
    
    return true;
}

bool Texture::createImage(const ImageData& imageData) {
    VkDevice device = VulkanBackend::instance().getDevice();
    VkPhysicalDevice physicalDevice = VulkanBackend::instance().getPhysicalDevice();
    
    VkDeviceSize imageSize = imageData.sizeBytes();
    
    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    if (!createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      stagingBuffer, stagingBufferMemory)) {
        return false;
    }
    
    // Copy pixel data to staging buffer
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    std::memcpy(data, imageData.pixels.data(), imageSize);
    vkUnmapMemory(device, stagingBufferMemory);
    
    // Determine format based on channels
    VkFormat format;
    switch (imageData.channels) {
        case 1: format = VK_FORMAT_R8_UNORM; break;
        case 2: format = VK_FORMAT_R8G8_UNORM; break;
        case 3: format = VK_FORMAT_R8G8B8_UNORM; break;
        case 4: 
        default: format = VK_FORMAT_R8G8B8A8_UNORM; break;
    }
    
    // Create image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(imageData.width);
    imageInfo.extent.height = static_cast<uint32_t>(imageData.height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    
    if (vkCreateImage(device, &imageInfo, nullptr, &m_image) != VK_SUCCESS) {
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
        return false;
    }
    
    // Allocate image memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_image, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_imageMemory) != VK_SUCCESS) {
        vkDestroyImage(device, m_image, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
        return false;
    }
    
    vkBindImageMemory(device, m_image, m_imageMemory, 0);
    
    // Transition image layout and copy data
    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, static_cast<uint32_t>(imageData.width), 
                      static_cast<uint32_t>(imageData.height));
    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    // Cleanup staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
    
    return true;
}

bool Texture::createImageView() {
    VkDevice device = VulkanBackend::instance().getDevice();
    
    VkFormat format;
    switch (m_channels) {
        case 1: format = VK_FORMAT_R8_UNORM; break;
        case 2: format = VK_FORMAT_R8G8_UNORM; break;
        case 3: format = VK_FORMAT_R8G8B8_UNORM; break;
        case 4: 
        default: format = VK_FORMAT_R8G8B8A8_UNORM; break;
    }
    
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    return vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) == VK_SUCCESS;
}

bool Texture::createSampler(const TextureConfig& config) {
    VkDevice device = VulkanBackend::instance().getDevice();
    
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    
    // Filter modes
    samplerInfo.magFilter = (config.magFilter == TextureFilter::Nearest) ? 
                            VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    samplerInfo.minFilter = (config.minFilter == TextureFilter::Nearest) ? 
                            VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    
    // Wrap modes
    auto toVkWrap = [](TextureWrap wrap) -> VkSamplerAddressMode {
        switch (wrap) {
            case TextureWrap::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case TextureWrap::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case TextureWrap::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case TextureWrap::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    };
    
    samplerInfo.addressModeU = toVkWrap(config.wrapU);
    samplerInfo.addressModeV = toVkWrap(config.wrapV);
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    
    // Anisotropy
    const auto& deviceProps = VulkanBackend::instance().getDeviceProperties();
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = deviceProps.limits.maxSamplerAnisotropy;
    
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    
    return vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) == VK_SUCCESS;
}

void Texture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = VulkanBackend::instance().beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
    
    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);
    
    VulkanBackend::instance().endSingleTimeCommands(commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = VulkanBackend::instance().beginSingleTimeCommands();
    
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    
    vkCmdCopyBufferToImage(commandBuffer, buffer, m_image, 
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    VulkanBackend::instance().endSingleTimeCommands(commandBuffer);
}

VkDescriptorImageInfo Texture::getDescriptorInfo() const {
    VkDescriptorImageInfo info{};
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info.imageView = m_imageView;
    info.sampler = m_sampler;
    return info;
}


// ============================================================================
// ImageLoader Implementation
// ============================================================================

ImageData ImageLoader::loadFromFile(const std::string& path, bool flipVertically) {
    ImageFormat format = detectFormat(path);
    
    switch (format) {
        case ImageFormat::ICO:
            return loadICO(path, flipVertically);
        case ImageFormat::SVG:
            return loadSVG(path, flipVertically);
        case ImageFormat::PNG:
        case ImageFormat::JPG:
        case ImageFormat::BMP:
        case ImageFormat::TGA:
        default:
            // Use stb_image for common formats
            break;
    }
    
    // Load using stb_image
    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);
    
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    
    ImageData result;
    if (data) {
        result.width = width;
        result.height = height;
        result.channels = 4; // We requested RGBA
        result.format = format;
        result.pixels.resize(static_cast<size_t>(width) * height * 4);
        std::memcpy(result.pixels.data(), data, result.pixels.size());
        stbi_image_free(data);
    }
    
    return result;
}

ImageData ImageLoader::loadFromMemory(const uint8_t* data, size_t size, bool flipVertically) {
    ImageFormat format = detectFormatFromHeader(data, size);
    
    switch (format) {
        case ImageFormat::ICO:
            return loadICOFromMemory(data, size, flipVertically);
        case ImageFormat::SVG:
            return loadSVGFromMemory(data, size, flipVertically);
        default:
            return loadFromMemorySTB(data, size, flipVertically);
    }
}

ImageData ImageLoader::loadFromMemorySTB(const uint8_t* data, size_t size, bool flipVertically) {
    stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);
    
    int width, height, channels;
    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(size), 
                                                   &width, &height, &channels, STBI_rgb_alpha);
    
    ImageData result;
    if (pixels) {
        result.width = width;
        result.height = height;
        result.channels = 4;
        result.pixels.resize(static_cast<size_t>(width) * height * 4);
        std::memcpy(result.pixels.data(), pixels, result.pixels.size());
        stbi_image_free(pixels);
    }
    
    return result;
}

ImageFormat ImageLoader::detectFormat(const std::string& path) {
    // Get file extension
    size_t dotPos = path.rfind('.');
    if (dotPos == std::string::npos) {
        return ImageFormat::Unknown;
    }
    
    std::string ext = path.substr(dotPos + 1);
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "png") return ImageFormat::PNG;
    if (ext == "jpg" || ext == "jpeg") return ImageFormat::JPG;
    if (ext == "bmp") return ImageFormat::BMP;
    if (ext == "tga") return ImageFormat::TGA;
    if (ext == "ico") return ImageFormat::ICO;
    if (ext == "svg") return ImageFormat::SVG;
    
    return ImageFormat::Unknown;
}

ImageFormat ImageLoader::detectFormatFromHeader(const uint8_t* data, size_t size) {
    if (size < 8) {
        return ImageFormat::Unknown;
    }
    
    // PNG: 89 50 4E 47 0D 0A 1A 0A
    if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) {
        return ImageFormat::PNG;
    }
    
    // JPEG: FF D8 FF
    if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
        return ImageFormat::JPG;
    }
    
    // BMP: 42 4D (BM)
    if (data[0] == 0x42 && data[1] == 0x4D) {
        return ImageFormat::BMP;
    }
    
    // ICO: 00 00 01 00
    if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x01 && data[3] == 0x00) {
        return ImageFormat::ICO;
    }
    
    // SVG: Check for XML/SVG header
    if (size >= 5) {
        std::string header(reinterpret_cast<const char*>(data), 5);
        if (header == "<?xml" || header == "<svg ") {
            return ImageFormat::SVG;
        }
    }
    
    return ImageFormat::Unknown;
}

bool ImageLoader::isFormatSupported(ImageFormat format) {
    switch (format) {
        case ImageFormat::PNG:
        case ImageFormat::JPG:
        case ImageFormat::BMP:
        case ImageFormat::TGA:
        case ImageFormat::ICO:
            return true;
        case ImageFormat::SVG:
            return false; // SVG requires additional library (nanosvg)
        default:
            return false;
    }
}

// ICO format loading
ImageData ImageLoader::loadICO(const std::string& path, bool flipVertically) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return ImageData{};
    }
    
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageData{};
    }
    
    return loadICOFromMemory(buffer.data(), size, flipVertically);
}

ImageData ImageLoader::loadICOFromMemory(const uint8_t* data, size_t size, bool flipVertically) {
    // ICO file format:
    // Header: 6 bytes
    //   - Reserved: 2 bytes (must be 0)
    //   - Type: 2 bytes (1 for ICO)
    //   - Count: 2 bytes (number of images)
    // Directory entries: 16 bytes each
    //   - Width: 1 byte (0 means 256)
    //   - Height: 1 byte (0 means 256)
    //   - ColorCount: 1 byte
    //   - Reserved: 1 byte
    //   - Planes: 2 bytes
    //   - BitCount: 2 bytes
    //   - SizeInBytes: 4 bytes
    //   - FileOffset: 4 bytes
    
    if (size < 6) {
        return ImageData{};
    }
    
    // Verify ICO header
    if (data[0] != 0 || data[1] != 0 || data[2] != 1 || data[3] != 0) {
        return ImageData{};
    }
    
    uint16_t imageCount = data[4] | (data[5] << 8);
    if (imageCount == 0 || size < 6 + imageCount * 16) {
        return ImageData{};
    }
    
    // Find the largest image
    int bestIndex = 0;
    int bestSize = 0;
    
    for (int i = 0; i < imageCount; i++) {
        const uint8_t* entry = data + 6 + i * 16;
        int width = entry[0] == 0 ? 256 : entry[0];
        int height = entry[1] == 0 ? 256 : entry[1];
        int pixelCount = width * height;
        
        if (pixelCount > bestSize) {
            bestSize = pixelCount;
            bestIndex = i;
        }
    }
    
    // Get the best image entry
    const uint8_t* entry = data + 6 + bestIndex * 16;
    uint32_t imageSize = entry[8] | (entry[9] << 8) | (entry[10] << 16) | (entry[11] << 24);
    uint32_t imageOffset = entry[12] | (entry[13] << 8) | (entry[14] << 16) | (entry[15] << 24);
    
    if (imageOffset + imageSize > size) {
        return ImageData{};
    }
    
    // The image data is typically PNG or BMP
    const uint8_t* imageData = data + imageOffset;
    
    // Check if it's PNG (most modern ICO files use PNG)
    if (imageSize >= 8 && imageData[0] == 0x89 && imageData[1] == 0x50) {
        return loadFromMemorySTB(imageData, imageSize, flipVertically);
    }
    
    // Otherwise it's BMP format - use stb_image
    return loadFromMemorySTB(imageData, imageSize, flipVertically);
}

ImageData ImageLoader::loadPNG(const std::string& path, bool flipVertically) {
    return loadFromFile(path, flipVertically);
}

ImageData ImageLoader::loadJPG(const std::string& path, bool flipVertically) {
    return loadFromFile(path, flipVertically);
}

ImageData ImageLoader::loadBMP(const std::string& path, bool flipVertically) {
    return loadFromFile(path, flipVertically);
}

ImageData ImageLoader::loadTGA(const std::string& path, bool flipVertically) {
    return loadFromFile(path, flipVertically);
}

ImageData ImageLoader::loadSVG(const std::string& path, bool flipVertically) {
    // SVG loading requires nanosvg library
    // For now, return empty - SVG support can be added later
    std::cerr << "[ImageLoader] SVG format not yet supported: " << path << std::endl;
    return ImageData{};
}

ImageData ImageLoader::loadSVGFromMemory(const uint8_t* data, size_t size, bool flipVertically) {
    // SVG loading requires nanosvg library
    std::cerr << "[ImageLoader] SVG format not yet supported" << std::endl;
    return ImageData{};
}

} // namespace KillerGK
