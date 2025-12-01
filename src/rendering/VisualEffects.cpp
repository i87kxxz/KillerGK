/**
 * @file VisualEffects.cpp
 * @brief Visual effects system implementation
 */

#include "KillerGK/rendering/VisualEffects.hpp"
#include "KillerGK/rendering/VulkanBackend.hpp"
#include "KillerGK/rendering/ShaderSystem.hpp"
#include "KillerGK/rendering/Renderer2D.hpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <iostream>
#include <array>

namespace KillerGK {

// ============================================================================
// Constants
// ============================================================================

static constexpr float MAX_BLUR_RADIUS = 100.0f;
static constexpr int MAX_KERNEL_SIZE = 31;
static constexpr uint32_t NOISE_TEXTURE_SIZE = 256;

// ============================================================================
// Built-in Blur Shaders (SPIR-V)
// ============================================================================

// Horizontal Gaussian blur vertex shader
// Simple pass-through that outputs position and texture coordinates
static const std::vector<uint32_t> blurVertexShader = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000030, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000a000f, 0x00000000,
    0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000012, 0x0000001c,
    0x00000026, 0x00000029, 0x00030003, 0x00000002, 0x000001c2, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x0000000b, 0x505f6c67,
    0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000,
    0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b, 0x00000001,
    0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b,
    0x00000002, 0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006,
    0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369, 0x0065636e,
    0x00030005, 0x0000000d, 0x00000000, 0x00060005, 0x00000010, 0x68737550,
    0x736e6f43, 0x746e6174, 0x00000073, 0x00060006, 0x00000010, 0x00000000,
    0x6e617274, 0x726f6673, 0x0000006d, 0x00030005, 0x00000012, 0x00006370,
    0x00050005, 0x00000016, 0x6f506e69, 0x69746973, 0x00006e6f, 0x00050005,
    0x0000001c, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00040005, 0x0000001e,
    0x6f436e69, 0x00726f6c, 0x00060005, 0x00000026, 0x67617266, 0x43786554,
    0x64726f6f, 0x00000000, 0x00050005, 0x00000029, 0x65546e69, 0x6f6f4378,
    0x00006472, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000,
    0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048,
    0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048, 0x0000000b,
    0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b, 0x00000002,
    0x00040048, 0x00000010, 0x00000000, 0x00000005, 0x00050048, 0x00000010,
    0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x00000010, 0x00000000,
    0x00000007, 0x00000010, 0x00030047, 0x00000010, 0x00000002, 0x00040047,
    0x00000016, 0x0000001e, 0x00000000, 0x00040047, 0x0000001c, 0x0000001e,
    0x00000000, 0x00040047, 0x0000001e, 0x0000001e, 0x00000001, 0x00040047,
    0x00000026, 0x0000001e, 0x00000001, 0x00040047, 0x00000029, 0x0000001e,
    0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
    0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006,
    0x00000004, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b,
    0x00000008, 0x00000009, 0x00000001, 0x0004001c, 0x0000000a, 0x00000006,
    0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a,
    0x0000000a, 0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b,
    0x0000000c, 0x0000000d, 0x00000003, 0x00040015, 0x0000000e, 0x00000020,
    0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040018,
    0x00000011, 0x00000007, 0x00000004, 0x0003001e, 0x00000010, 0x00000011,
    0x00040020, 0x00000013, 0x00000009, 0x00000010, 0x0004003b, 0x00000013,
    0x00000012, 0x00000009, 0x00040020, 0x00000014, 0x00000009, 0x00000011,
    0x00040017, 0x00000015, 0x00000006, 0x00000002, 0x00040020, 0x00000017,
    0x00000001, 0x00000015, 0x0004003b, 0x00000017, 0x00000016, 0x00000001,
    0x0004002b, 0x00000006, 0x00000019, 0x00000000, 0x0004002b, 0x00000006,
    0x0000001a, 0x3f800000, 0x00040020, 0x0000001b, 0x00000003, 0x00000007,
    0x0004003b, 0x0000001b, 0x0000001c, 0x00000003, 0x00040020, 0x0000001d,
    0x00000001, 0x00000007, 0x0004003b, 0x0000001d, 0x0000001e, 0x00000001,
    0x00040020, 0x00000025, 0x00000003, 0x00000015, 0x0004003b, 0x00000025,
    0x00000026, 0x00000003, 0x0004003b, 0x00000017, 0x00000029, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x00050041, 0x00000014, 0x00000018, 0x00000012, 0x0000000f,
    0x0004003d, 0x00000011, 0x0000001f, 0x00000018, 0x0004003d, 0x00000015,
    0x00000020, 0x00000016, 0x00050051, 0x00000006, 0x00000021, 0x00000020,
    0x00000000, 0x00050051, 0x00000006, 0x00000022, 0x00000020, 0x00000001,
    0x00070050, 0x00000007, 0x00000023, 0x00000021, 0x00000022, 0x00000019,
    0x0000001a, 0x00050091, 0x00000007, 0x00000024, 0x0000001f, 0x00000023,
    0x00050041, 0x0000001b, 0x00000027, 0x0000000d, 0x0000000f, 0x0003003e,
    0x00000027, 0x00000024, 0x0004003d, 0x00000007, 0x00000028, 0x0000001e,
    0x0003003e, 0x0000001c, 0x00000028, 0x0004003d, 0x00000015, 0x0000002a,
    0x00000029, 0x0003003e, 0x00000026, 0x0000002a, 0x000100fd, 0x00010038
};

// Simple fragment shader that outputs vertex color (used for shadow/effect rendering)
static const std::vector<uint32_t> effectFragmentShader = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000020, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00000014,
    0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2,
    0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009,
    0x4374756f, 0x726f6c6f, 0x00000000, 0x00050005, 0x0000000b, 0x67617266,
    0x6f6c6f43, 0x00000072, 0x00050005, 0x00000014, 0x67617266, 0x43786554,
    0x64726f6f, 0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000,
    0x00040047, 0x0000000b, 0x0000001e, 0x00000000, 0x00040047, 0x00000014,
    0x0000001e, 0x00000001, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
    0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007,
    0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a,
    0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001,
    0x00040017, 0x00000013, 0x00000006, 0x00000002, 0x00040020, 0x00000015,
    0x00000001, 0x00000013, 0x0004003b, 0x00000015, 0x00000014, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x0004003d, 0x00000007, 0x0000000c, 0x0000000b, 0x0003003e,
    0x00000009, 0x0000000c, 0x000100fd, 0x00010038
};

// ============================================================================
// Implementation Structure
// ============================================================================

struct VisualEffects::Impl {
    bool initialized = false;
    float quality = 1.0f;
    
    // Blur resources
    VkPipeline blurPipeline = VK_NULL_HANDLE;
    ShaderModule blurVertShader;
    ShaderModule blurFragShader;
    
    // Effect pipeline (for shadows, etc.)
    VkPipeline effectPipeline = VK_NULL_HANDLE;
    ShaderModule effectVertShader;
    ShaderModule effectFragShader;
    
    // Noise texture for acrylic effect
    VkImage noiseImage = VK_NULL_HANDLE;
    VkDeviceMemory noiseMemory = VK_NULL_HANDLE;
    VkImageView noiseImageView = VK_NULL_HANDLE;
    VkSampler noiseSampler = VK_NULL_HANDLE;
    bool noiseTextureCreated = false;
    
    // Current effect state
    bool inGlassRegion = false;
    bool inAcrylicRegion = false;
    Rect currentEffectRect;
    GlassmorphismConfig currentGlassConfig;
    AcrylicConfig currentAcrylicConfig;
    
    // Gaussian kernel cache
    std::vector<float> cachedKernel;
    float cachedSigma = 0.0f;
    int cachedKernelSize = 0;
};

// ============================================================================
// Singleton Instance
// ============================================================================

VisualEffects& VisualEffects::instance() {
    static VisualEffects instance;
    return instance;
}

VisualEffects::VisualEffects() : m_impl(std::make_unique<Impl>()) {}

VisualEffects::~VisualEffects() {
    if (m_impl && m_impl->initialized) {
        shutdown();
    }
}

// ============================================================================
// Initialization and Shutdown
// ============================================================================

bool VisualEffects::initialize() {
    if (m_impl->initialized) {
        return true;
    }
    
    if (!VulkanBackend::instance().isInitialized()) {
        std::cerr << "[VisualEffects] VulkanBackend not initialized" << std::endl;
        return false;
    }
    
    if (!ShaderSystem::instance().isInitialized()) {
        std::cerr << "[VisualEffects] ShaderSystem not initialized" << std::endl;
        return false;
    }
    
    // Create blur pipelines
    if (!createBlurPipelines()) {
        std::cerr << "[VisualEffects] Failed to create blur pipelines" << std::endl;
        // Continue without blur - effects will fall back to simpler rendering
    }
    
    // Create noise texture for acrylic effect
    if (!createNoiseTexture()) {
        std::cerr << "[VisualEffects] Failed to create noise texture" << std::endl;
        // Continue without noise - acrylic will work without noise overlay
    }
    
    m_impl->initialized = true;
    return true;
}

void VisualEffects::shutdown() {
    if (!m_impl->initialized) {
        return;
    }
    
    VkDevice device = VulkanBackend::instance().getDevice();
    VulkanBackend::instance().waitIdle();
    
    // Destroy pipelines
    if (m_impl->blurPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, m_impl->blurPipeline, nullptr);
        m_impl->blurPipeline = VK_NULL_HANDLE;
    }
    
    if (m_impl->effectPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, m_impl->effectPipeline, nullptr);
        m_impl->effectPipeline = VK_NULL_HANDLE;
    }
    
    // Destroy shaders
    ShaderSystem::instance().destroyShader(m_impl->blurVertShader);
    ShaderSystem::instance().destroyShader(m_impl->blurFragShader);
    ShaderSystem::instance().destroyShader(m_impl->effectVertShader);
    ShaderSystem::instance().destroyShader(m_impl->effectFragShader);
    
    // Destroy noise texture resources
    if (m_impl->noiseSampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, m_impl->noiseSampler, nullptr);
        m_impl->noiseSampler = VK_NULL_HANDLE;
    }
    if (m_impl->noiseImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, m_impl->noiseImageView, nullptr);
        m_impl->noiseImageView = VK_NULL_HANDLE;
    }
    if (m_impl->noiseImage != VK_NULL_HANDLE) {
        vkDestroyImage(device, m_impl->noiseImage, nullptr);
        m_impl->noiseImage = VK_NULL_HANDLE;
    }
    if (m_impl->noiseMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_impl->noiseMemory, nullptr);
        m_impl->noiseMemory = VK_NULL_HANDLE;
    }
    
    m_impl->initialized = false;
}

bool VisualEffects::isInitialized() const {
    return m_impl->initialized;
}


// ============================================================================
// Pipeline Creation
// ============================================================================

bool VisualEffects::createBlurPipelines() {
    // Load blur shaders from built-in SPIR-V
    m_impl->blurVertShader = ShaderSystem::instance().loadShaderFromMemory(
        blurVertexShader, ShaderStage::Vertex);
    if (m_impl->blurVertShader.module == VK_NULL_HANDLE) {
        return false;
    }
    
    m_impl->effectFragShader = ShaderSystem::instance().loadShaderFromMemory(
        effectFragmentShader, ShaderStage::Fragment);
    if (m_impl->effectFragShader.module == VK_NULL_HANDLE) {
        return false;
    }
    
    // Create effect pipeline for shadow/glass rendering
    Pipeline2DConfig config;
    config.enableBlending = true;
    
    m_impl->effectPipeline = ShaderSystem::instance().createPipeline2D(
        m_impl->blurVertShader, m_impl->effectFragShader, config);
    
    return m_impl->effectPipeline != VK_NULL_HANDLE;
}

bool VisualEffects::createBlurResources() {
    // Blur resources are created on-demand
    return true;
}

bool VisualEffects::createNoiseTexture() {
    return generateNoiseTexture(NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE);
}

bool VisualEffects::createShadowResources() {
    // Shadow resources use the effect pipeline
    return true;
}

// ============================================================================
// Gaussian Kernel Generation
// ============================================================================

std::vector<float> VisualEffects::generateGaussianKernel(float sigma, int size) {
    // Check cache
    if (std::abs(sigma - m_impl->cachedSigma) < 0.001f && size == m_impl->cachedKernelSize) {
        return m_impl->cachedKernel;
    }
    
    // Ensure odd size
    if (size % 2 == 0) {
        size++;
    }
    size = std::min(size, MAX_KERNEL_SIZE);
    
    std::vector<float> kernel(size);
    float sum = 0.0f;
    int halfSize = size / 2;
    
    // Calculate Gaussian values
    float twoSigmaSquared = 2.0f * sigma * sigma;
    for (int i = 0; i < size; i++) {
        float x = static_cast<float>(i - halfSize);
        kernel[i] = std::exp(-(x * x) / twoSigmaSquared);
        sum += kernel[i];
    }
    
    // Normalize
    for (int i = 0; i < size; i++) {
        kernel[i] /= sum;
    }
    
    // Cache the result
    m_impl->cachedKernel = kernel;
    m_impl->cachedSigma = sigma;
    m_impl->cachedKernelSize = size;
    
    return kernel;
}

// ============================================================================
// Glassmorphism Effect
// ============================================================================

void VisualEffects::beginGlassmorphism(const Rect& rect, const GlassmorphismConfig& config) {
    if (!m_impl->initialized || !config.enabled) {
        return;
    }
    
    m_impl->inGlassRegion = true;
    m_impl->currentEffectRect = rect;
    m_impl->currentGlassConfig = config;
    
    // In a full implementation, this would:
    // 1. Capture the current framebuffer content behind the rect
    // 2. Apply Gaussian blur to the captured region
    // 3. Set up rendering state for the glass panel
}

void VisualEffects::endGlassmorphism() {
    if (!m_impl->inGlassRegion) {
        return;
    }
    
    m_impl->inGlassRegion = false;
    
    // In a full implementation, this would composite the blurred background
    // with the content rendered on top
}

void VisualEffects::renderGlassPanel(const Rect& rect, float cornerRadius, 
                                      const GlassmorphismConfig& config) {
    if (!m_impl->initialized || !config.enabled) {
        // Fall back to simple semi-transparent panel
        Renderer2D::instance().drawRoundRect(rect, cornerRadius, 
            Color(1.0f, 1.0f, 1.0f, config.opacity));
        return;
    }
    
    // Render the glassmorphism effect
    // This is a simplified implementation that creates the visual appearance
    // without actual real-time blur (which requires framebuffer capture)
    
    // 1. Render semi-transparent background with tint
    Color bgColor = config.tintColor;
    bgColor.a = config.opacity;
    Renderer2D::instance().drawRoundRect(rect, cornerRadius, bgColor);
    
    // 2. Add subtle gradient overlay for depth
    Color gradientTop = Color(1.0f, 1.0f, 1.0f, 0.1f);
    Color gradientBottom = Color(1.0f, 1.0f, 1.0f, 0.02f);
    
    // Create gradient effect using multiple layers
    Rect topHalf = rect;
    topHalf.height = rect.height * 0.5f;
    Renderer2D::instance().drawRoundRect(topHalf, cornerRadius, gradientTop);
    
    // 3. Add border highlight for glass edge effect
    Color borderColor = Color(1.0f, 1.0f, 1.0f, 0.2f);
    // Note: In a full implementation, we'd draw a rounded rect outline here
}

// ============================================================================
// Acrylic Effect
// ============================================================================

void VisualEffects::beginAcrylic(const Rect& rect, const AcrylicConfig& config) {
    if (!m_impl->initialized || !config.enabled) {
        return;
    }
    
    m_impl->inAcrylicRegion = true;
    m_impl->currentEffectRect = rect;
    m_impl->currentAcrylicConfig = config;
    
    // In a full implementation with framebuffer capture:
    // 1. Capture the current framebuffer content behind the rect
    // 2. Store it for blur processing in endAcrylic()
}

void VisualEffects::endAcrylic() {
    if (!m_impl->inAcrylicRegion) {
        return;
    }
    
    m_impl->inAcrylicRegion = false;
    
    // In a full implementation:
    // 1. Apply blur to captured background
    // 2. Composite with tint and noise
    // 3. Render the final result
}

void VisualEffects::renderAcrylicPanel(const Rect& rect, float cornerRadius,
                                        const AcrylicConfig& config) {
    if (!m_impl->initialized || !config.enabled) {
        // Fall back to simple semi-transparent panel
        Renderer2D::instance().drawRoundRect(rect, cornerRadius, config.tintColor);
        return;
    }
    
    // Windows 11 Acrylic Effect Implementation
    // The acrylic effect consists of multiple layers:
    // 1. Blurred background (simulated with multiple semi-transparent layers)
    // 2. Luminosity blend layer
    // 3. Tint color layer
    // 4. Noise texture overlay
    // 5. Optional exclusion blend for saturation
    
    // Clamp blur radius to valid range
    float blurRadius = std::clamp(config.blurRadius, 0.0f, MAX_BLUR_RADIUS);
    
    // Layer 1: Simulated blur background
    // Create a soft blur effect using multiple semi-transparent layers
    // This approximates the Gaussian blur without actual framebuffer capture
    renderAcrylicBlurLayers(rect, cornerRadius, blurRadius, config);
    
    // Layer 2: Luminosity blend layer
    // This creates the characteristic "glow" of acrylic
    renderAcrylicLuminosityLayer(rect, cornerRadius, config);
    
    // Layer 3: Tint color layer
    // Apply the main tint color with proper opacity
    renderAcrylicTintLayer(rect, cornerRadius, config);
    
    // Layer 4: Noise texture overlay
    // Adds the subtle grain texture characteristic of Windows 11 acrylic
    if (m_impl->noiseTextureCreated && config.noiseOpacity > 0.001f) {
        renderAcrylicNoiseLayer(rect, cornerRadius, config);
    }
    
    // Layer 5: Border highlight for depth
    renderAcrylicBorder(rect, cornerRadius, config);
}

void VisualEffects::renderAcrylicBlurLayers(const Rect& rect, float cornerRadius,
                                             float blurRadius, const AcrylicConfig& config) {
    // Simulate blur using multiple expanding semi-transparent layers
    // This creates a soft, diffused appearance similar to actual blur
    
    // Use tint color's luminance to influence blur color
    float tintLuminance = config.tintColor.r * 0.299f + config.tintColor.g * 0.587f + config.tintColor.b * 0.114f;
    
    int numLayers = static_cast<int>(std::ceil(blurRadius / 5.0f));
    numLayers = std::clamp(numLayers, 3, 12);
    
    float baseAlpha = 0.03f * m_impl->quality;
    float layerStep = blurRadius / static_cast<float>(numLayers);
    
    for (int i = numLayers - 1; i >= 0; i--) {
        float offset = layerStep * static_cast<float>(i);
        float layerAlpha = baseAlpha * (1.0f - static_cast<float>(i) / static_cast<float>(numLayers));
        
        Rect layerRect;
        layerRect.x = rect.x - offset;
        layerRect.y = rect.y - offset;
        layerRect.width = rect.width + offset * 2.0f;
        layerRect.height = rect.height + offset * 2.0f;
        
        // Use a color influenced by the tint for the blur base
        Color blurColor(tintLuminance, tintLuminance, tintLuminance, layerAlpha);
        
        float layerRadius = cornerRadius + offset * 0.5f;
        Renderer2D::instance().drawRoundRect(layerRect, layerRadius, blurColor);
    }
}

void VisualEffects::renderAcrylicLuminosityLayer(const Rect& rect, float cornerRadius,
                                                  const AcrylicConfig& config) {
    // Luminosity layer creates the characteristic bright/light appearance
    // Windows 11 uses this to make the acrylic feel "lit from within"
    
    // Top gradient (brighter at top)
    Rect topRect = rect;
    topRect.height = rect.height * 0.4f;
    Color topLuminosity(1.0f, 1.0f, 1.0f, config.luminosityOpacity * 0.15f);
    Renderer2D::instance().drawRoundRect(topRect, cornerRadius, topLuminosity);
    
    // Center luminosity (subtle overall glow)
    Color centerLuminosity(1.0f, 1.0f, 1.0f, config.luminosityOpacity * 0.05f);
    Renderer2D::instance().drawRoundRect(rect, cornerRadius, centerLuminosity);
    
    // Bottom subtle shadow for depth
    Rect bottomRect = rect;
    bottomRect.y = rect.y + rect.height * 0.7f;
    bottomRect.height = rect.height * 0.3f;
    Color bottomShadow(0.0f, 0.0f, 0.0f, config.luminosityOpacity * 0.03f);
    Renderer2D::instance().drawRoundRect(bottomRect, cornerRadius, bottomShadow);
}

void VisualEffects::renderAcrylicTintLayer(const Rect& rect, float cornerRadius,
                                            const AcrylicConfig& config) {
    // Main tint layer - this gives the acrylic its color
    // Windows 11 typically uses dark tints for dark mode, light for light mode
    
    Color tintColor = config.tintColor;
    // Ensure the tint has proper transparency for the acrylic effect
    tintColor.a = std::clamp(tintColor.a, 0.0f, 0.9f);
    
    Renderer2D::instance().drawRoundRect(rect, cornerRadius, tintColor);
}

void VisualEffects::renderAcrylicNoiseLayer(const Rect& rect, float cornerRadius,
                                             const AcrylicConfig& config) {
    // Noise texture overlay - characteristic of Windows 11 acrylic
    // This adds a subtle grain that makes the surface feel more physical
    
    if (!m_impl->noiseTextureCreated) {
        return;
    }
    
    // In a full implementation with texture sampling, we would:
    // 1. Bind the noise texture
    // 2. Render a quad with UV coordinates tiled across the rect
    // 3. Use multiply or overlay blend mode
    
    // For now, simulate noise with a dithered pattern using multiple small rects
    // This creates a similar visual effect without texture sampling
    
    float noiseAlpha = config.noiseOpacity;
    if (noiseAlpha < 0.001f) {
        return;
    }
    
    // Create a pseudo-random noise pattern using deterministic positions
    // This simulates the noise texture overlay
    std::mt19937 gen(42); // Fixed seed for consistent pattern
    std::uniform_real_distribution<float> posDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> valueDist(0.3f, 0.7f);
    
    // Calculate number of noise points based on rect size and quality
    int noisePoints = static_cast<int>((rect.width * rect.height) / 100.0f * m_impl->quality);
    noisePoints = std::clamp(noisePoints, 50, 500);
    
    float pointSize = 1.5f;
    
    for (int i = 0; i < noisePoints; i++) {
        float nx = rect.x + posDist(gen) * rect.width;
        float ny = rect.y + posDist(gen) * rect.height;
        float nv = valueDist(gen);
        
        // Check if point is within rounded corner bounds
        if (isPointInRoundedRect(nx, ny, rect, cornerRadius)) {
            Color noiseColor(nv, nv, nv, noiseAlpha);
            Rect noiseRect{nx, ny, pointSize, pointSize};
            Renderer2D::instance().drawRect(noiseRect, noiseColor);
        }
    }
}

void VisualEffects::renderAcrylicBorder(const Rect& rect, float cornerRadius,
                                         const AcrylicConfig& config) {
    // Subtle border highlight for depth and definition
    // Windows 11 acrylic panels often have a thin bright border at the top
    
    // Adjust border opacity based on luminosity setting
    float borderOpacity = 0.1f * config.luminosityOpacity;
    
    // Top edge highlight
    Color topBorder(1.0f, 1.0f, 1.0f, borderOpacity);
    Rect topEdge{rect.x + cornerRadius, rect.y, rect.width - cornerRadius * 2.0f, 1.0f};
    Renderer2D::instance().drawRect(topEdge, topBorder);
    
    // Left edge highlight (subtle)
    Color leftBorder(1.0f, 1.0f, 1.0f, borderOpacity * 0.5f);
    Rect leftEdge{rect.x, rect.y + cornerRadius, 1.0f, rect.height - cornerRadius * 2.0f};
    Renderer2D::instance().drawRect(leftEdge, leftBorder);
}

bool VisualEffects::isPointInRoundedRect(float px, float py, const Rect& rect, float radius) {
    // Check if point is inside the rounded rectangle
    // Used for noise texture clipping
    
    // First check if inside the main rect
    if (px < rect.x || px > rect.x + rect.width ||
        py < rect.y || py > rect.y + rect.height) {
        return false;
    }
    
    // Check corners
    float cornerCenterX, cornerCenterY;
    
    // Top-left corner
    if (px < rect.x + radius && py < rect.y + radius) {
        cornerCenterX = rect.x + radius;
        cornerCenterY = rect.y + radius;
        float dx = px - cornerCenterX;
        float dy = py - cornerCenterY;
        return (dx * dx + dy * dy) <= (radius * radius);
    }
    
    // Top-right corner
    if (px > rect.x + rect.width - radius && py < rect.y + radius) {
        cornerCenterX = rect.x + rect.width - radius;
        cornerCenterY = rect.y + radius;
        float dx = px - cornerCenterX;
        float dy = py - cornerCenterY;
        return (dx * dx + dy * dy) <= (radius * radius);
    }
    
    // Bottom-left corner
    if (px < rect.x + radius && py > rect.y + rect.height - radius) {
        cornerCenterX = rect.x + radius;
        cornerCenterY = rect.y + rect.height - radius;
        float dx = px - cornerCenterX;
        float dy = py - cornerCenterY;
        return (dx * dx + dy * dy) <= (radius * radius);
    }
    
    // Bottom-right corner
    if (px > rect.x + rect.width - radius && py > rect.y + rect.height - radius) {
        cornerCenterX = rect.x + rect.width - radius;
        cornerCenterY = rect.y + rect.height - radius;
        float dx = px - cornerCenterX;
        float dy = py - cornerCenterY;
        return (dx * dx + dy * dy) <= (radius * radius);
    }
    
    return true;
}

// ============================================================================
// Shadow Effect
// ============================================================================

void VisualEffects::renderShadow(const Rect& rect, const ShadowConfig& config) {
    renderRoundedShadow(rect, 0.0f, config);
}

void VisualEffects::renderRoundedShadow(const Rect& rect, float cornerRadius,
                                         const ShadowConfig& config) {
    if (!m_impl->initialized || !config.enabled) {
        return;
    }
    
    // Calculate shadow bounds with offset and spread
    Rect shadowRect;
    shadowRect.x = rect.x + config.offsetX - config.spread - config.blurRadius;
    shadowRect.y = rect.y + config.offsetY - config.spread - config.blurRadius;
    shadowRect.width = rect.width + (config.spread + config.blurRadius) * 2.0f;
    shadowRect.height = rect.height + (config.spread + config.blurRadius) * 2.0f;
    
    // Render shadow using multiple layers for soft blur effect
    // This creates a convincing shadow without actual blur shader
    int layers = static_cast<int>(std::ceil(config.blurRadius / 3.0f));
    layers = std::clamp(layers, 1, 10);
    
    float alphaStep = config.color.a / static_cast<float>(layers);
    float sizeStep = config.blurRadius / static_cast<float>(layers);
    
    for (int i = layers - 1; i >= 0; i--) {
        float layerOffset = sizeStep * static_cast<float>(i);
        float layerAlpha = alphaStep * static_cast<float>(layers - i);
        
        Rect layerRect;
        layerRect.x = rect.x + config.offsetX - config.spread - layerOffset;
        layerRect.y = rect.y + config.offsetY - config.spread - layerOffset;
        layerRect.width = rect.width + (config.spread + layerOffset) * 2.0f;
        layerRect.height = rect.height + (config.spread + layerOffset) * 2.0f;
        
        Color layerColor = config.color;
        layerColor.a = layerAlpha;
        
        float layerRadius = cornerRadius + layerOffset;
        Renderer2D::instance().drawRoundRect(layerRect, layerRadius, layerColor);
    }
}

void VisualEffects::renderCircleShadow(float cx, float cy, float radius,
                                        const ShadowConfig& config) {
    if (!m_impl->initialized || !config.enabled) {
        return;
    }
    
    // Render circular shadow using multiple layers
    int layers = static_cast<int>(std::ceil(config.blurRadius / 3.0f));
    layers = std::clamp(layers, 1, 10);
    
    float alphaStep = config.color.a / static_cast<float>(layers);
    float sizeStep = config.blurRadius / static_cast<float>(layers);
    
    float shadowCx = cx + config.offsetX;
    float shadowCy = cy + config.offsetY;
    
    for (int i = layers - 1; i >= 0; i--) {
        float layerOffset = sizeStep * static_cast<float>(i);
        float layerAlpha = alphaStep * static_cast<float>(layers - i);
        float layerRadius = radius + config.spread + layerOffset;
        
        Color layerColor = config.color;
        layerColor.a = layerAlpha;
        
        Renderer2D::instance().drawCircle(shadowCx, shadowCy, layerRadius, layerColor);
    }
}

// ============================================================================
// Low-level Blur Operations
// ============================================================================

void VisualEffects::applyBlur(const Rect& rect, float radius) {
    if (!m_impl->initialized || radius <= 0.0f) {
        return;
    }
    
    // Clamp radius
    radius = std::min(radius, MAX_BLUR_RADIUS);
    
    // In a full implementation, this would:
    // 1. Capture the framebuffer region
    // 2. Apply horizontal Gaussian blur pass
    // 3. Apply vertical Gaussian blur pass
    // 4. Composite the result back
    
    // For now, this is a placeholder that would be implemented
    // with proper framebuffer operations
    
    // Suppress unused parameter warnings
    (void)rect;
    (void)radius;
}

void VisualEffects::applyBoxBlur(const Rect& rect, float radius) {
    if (!m_impl->initialized || radius <= 0.0f) {
        return;
    }
    
    // Box blur is faster but lower quality than Gaussian
    // Implementation would be similar to applyBlur but with
    // a simple averaging kernel instead of Gaussian weights
    
    // Suppress unused parameter warnings
    (void)rect;
    (void)radius;
}

// ============================================================================
// Utility Methods
// ============================================================================

bool VisualEffects::generateNoiseTexture(uint32_t width, uint32_t height) {
    VkDevice device = VulkanBackend::instance().getDevice();
    VkPhysicalDevice physicalDevice = VulkanBackend::instance().getPhysicalDevice();
    
    // Generate noise data
    std::vector<uint8_t> noiseData(width * height * 4);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < noiseData.size(); i += 4) {
        uint8_t noise = static_cast<uint8_t>(dis(gen));
        noiseData[i] = noise;     // R
        noiseData[i + 1] = noise; // G
        noiseData[i + 2] = noise; // B
        noiseData[i + 3] = 255;   // A
    }
    
    // Create image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    
    if (vkCreateImage(device, &imageInfo, nullptr, &m_impl->noiseImage) != VK_SUCCESS) {
        return false;
    }
    
    // Allocate memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_impl->noiseImage, &memRequirements);
    
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    uint32_t memoryTypeIndex = 0;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            memoryTypeIndex = i;
            break;
        }
    }
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_impl->noiseMemory) != VK_SUCCESS) {
        vkDestroyImage(device, m_impl->noiseImage, nullptr);
        m_impl->noiseImage = VK_NULL_HANDLE;
        return false;
    }
    
    vkBindImageMemory(device, m_impl->noiseImage, m_impl->noiseMemory, 0);
    
    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_impl->noiseImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    if (vkCreateImageView(device, &viewInfo, nullptr, &m_impl->noiseImageView) != VK_SUCCESS) {
        vkFreeMemory(device, m_impl->noiseMemory, nullptr);
        vkDestroyImage(device, m_impl->noiseImage, nullptr);
        m_impl->noiseImage = VK_NULL_HANDLE;
        m_impl->noiseMemory = VK_NULL_HANDLE;
        return false;
    }
    
    // Create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    
    if (vkCreateSampler(device, &samplerInfo, nullptr, &m_impl->noiseSampler) != VK_SUCCESS) {
        vkDestroyImageView(device, m_impl->noiseImageView, nullptr);
        vkFreeMemory(device, m_impl->noiseMemory, nullptr);
        vkDestroyImage(device, m_impl->noiseImage, nullptr);
        m_impl->noiseImage = VK_NULL_HANDLE;
        m_impl->noiseMemory = VK_NULL_HANDLE;
        m_impl->noiseImageView = VK_NULL_HANDLE;
        return false;
    }
    
    m_impl->noiseTextureCreated = true;
    return true;
}

bool VisualEffects::isBlurSupported() const {
    // Blur is supported if we have the effect pipeline
    return m_impl->initialized && m_impl->effectPipeline != VK_NULL_HANDLE;
}

float VisualEffects::getMaxBlurRadius() const {
    return MAX_BLUR_RADIUS;
}

void VisualEffects::setQuality(float quality) {
    m_impl->quality = std::clamp(quality, 0.0f, 1.0f);
}

float VisualEffects::getQuality() const {
    return m_impl->quality;
}

void VisualEffects::renderBlurredRect(const Rect& rect, float blurRadius, 
                                       const Color& tint, float opacity) {
    // Helper method for rendering a blurred rectangle
    // This would be used internally by glassmorphism and acrylic effects
    
    // Suppress unused parameter warning (blur not yet implemented)
    (void)blurRadius;
    
    Color finalColor = tint;
    finalColor.a *= opacity;
    Renderer2D::instance().drawRect(rect, finalColor);
}

void VisualEffects::renderShadowGeometry(const Rect& rect, float cornerRadius,
                                          const ShadowConfig& config) {
    // Internal helper for shadow geometry rendering
    renderRoundedShadow(rect, cornerRadius, config);
}

void VisualEffects::renderWidgetShadow(const Rect& bounds, float cornerRadius,
                                        float blur, float offsetX, float offsetY,
                                        const Color& color, float spread) {
    // Create shadow config from parameters
    ShadowConfig config;
    config.blurRadius = blur;
    config.offsetX = offsetX;
    config.offsetY = offsetY;
    config.color = color;
    config.spread = spread;
    config.enabled = true;
    
    // Render the shadow
    if (cornerRadius > 0.0f) {
        renderRoundedShadow(bounds, cornerRadius, config);
    } else {
        renderShadow(bounds, config);
    }
}

} // namespace KillerGK
