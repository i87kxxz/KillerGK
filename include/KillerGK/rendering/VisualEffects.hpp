/**
 * @file VisualEffects.hpp
 * @brief Visual effects system for KillerGK including glassmorphism, acrylic, and shadows
 * 
 * Provides GPU-accelerated visual effects for modern UI design including:
 * - Glassmorphism (frosted glass effect with blur and transparency)
 * - Acrylic effect (Windows 11-style blur with noise texture)
 * - Configurable shadows with blur, offset, and color
 */

#pragma once

#include "../core/Types.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <algorithm>

namespace KillerGK {

/**
 * @struct GlassmorphismConfig
 * @brief Configuration for glassmorphism effect
 */
struct GlassmorphismConfig {
    float blurRadius = 20.0f;       // Blur intensity (0-100)
    float opacity = 0.7f;           // Background opacity (0-1)
    Color tintColor{1.0f, 1.0f, 1.0f, 0.1f};  // Tint overlay color
    float saturation = 1.2f;        // Color saturation boost
    bool enabled = true;
};

/**
 * @struct AcrylicConfig
 * @brief Configuration for Windows 11-style acrylic effect
 */
struct AcrylicConfig {
    float blurRadius = 30.0f;       // Blur intensity (0-100)
    float noiseOpacity = 0.02f;     // Noise texture opacity (0-1)
    Color tintColor{0.0f, 0.0f, 0.0f, 0.5f};  // Tint overlay color
    float luminosityOpacity = 0.8f; // Luminosity blend opacity
    bool enabled = true;
};

/**
 * @struct ShadowConfig
 * @brief Configuration for shadow rendering
 * 
 * Supports configurable blur, offset, color, and spread for
 * creating realistic drop shadows behind UI elements.
 */
struct ShadowConfig {
    float blurRadius = 10.0f;       ///< Shadow blur radius (0 = sharp edge)
    float offsetX = 0.0f;           ///< Horizontal offset from element
    float offsetY = 4.0f;           ///< Vertical offset from element
    Color color{0.0f, 0.0f, 0.0f, 0.3f};  ///< Shadow color with alpha
    float spread = 0.0f;            ///< Shadow spread (positive = expand, negative = contract)
    bool enabled = true;            ///< Whether shadow is enabled
    
    /**
     * @brief Default constructor with standard shadow settings
     */
    constexpr ShadowConfig() = default;
    
    /**
     * @brief Construct with all parameters
     * @param blur Blur radius
     * @param ox Horizontal offset
     * @param oy Vertical offset
     * @param c Shadow color
     * @param s Spread value
     */
    constexpr ShadowConfig(float blur, float ox, float oy, const Color& c, float s = 0.0f)
        : blurRadius(blur), offsetX(ox), offsetY(oy), color(c), spread(s), enabled(true) {}
    
    /**
     * @brief Create a subtle shadow preset
     * @return ShadowConfig with subtle shadow settings
     */
    static constexpr ShadowConfig subtle() {
        return ShadowConfig(4.0f, 0.0f, 2.0f, Color(0.0f, 0.0f, 0.0f, 0.1f));
    }
    
    /**
     * @brief Create a medium shadow preset
     * @return ShadowConfig with medium shadow settings
     */
    static constexpr ShadowConfig medium() {
        return ShadowConfig(8.0f, 0.0f, 4.0f, Color(0.0f, 0.0f, 0.0f, 0.2f));
    }
    
    /**
     * @brief Create a strong shadow preset
     * @return ShadowConfig with strong shadow settings
     */
    static constexpr ShadowConfig strong() {
        return ShadowConfig(16.0f, 0.0f, 8.0f, Color(0.0f, 0.0f, 0.0f, 0.3f));
    }
    
    /**
     * @brief Create an elevated shadow preset (Material Design style)
     * @param elevation Elevation level (1-24)
     * @return ShadowConfig with elevation-based shadow
     */
    static ShadowConfig elevation(int level) {
        level = std::max(1, std::min(24, level));
        float blur = static_cast<float>(level) * 1.5f;
        float offset = static_cast<float>(level) * 0.5f;
        float alpha = 0.1f + static_cast<float>(level) * 0.01f;
        return ShadowConfig(blur, 0.0f, offset, Color(0.0f, 0.0f, 0.0f, alpha));
    }
};

/**
 * @struct BlurPassConfig
 * @brief Internal configuration for blur passes
 */
struct BlurPassConfig {
    int passes = 2;                 // Number of blur passes
    float sigma = 5.0f;             // Gaussian sigma
    int kernelSize = 9;             // Blur kernel size (odd number)
};

/**
 * @class VisualEffects
 * @brief Manages visual effects rendering including blur, glassmorphism, and shadows
 */
class VisualEffects {
public:
    /**
     * @brief Get the singleton instance
     */
    static VisualEffects& instance();
    
    /**
     * @brief Initialize the visual effects system
     * @return true if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Shutdown and cleanup resources
     */
    void shutdown();
    
    /**
     * @brief Check if initialized
     */
    [[nodiscard]] bool isInitialized() const;
    
    // ========================================================================
    // Glassmorphism Effect
    // ========================================================================
    
    /**
     * @brief Begin rendering a glassmorphism region
     * @param rect The region to apply the effect
     * @param config Effect configuration
     * 
     * This captures the background behind the rect and applies blur.
     * Call endGlassmorphism() after rendering content on top.
     */
    void beginGlassmorphism(const Rect& rect, const GlassmorphismConfig& config = GlassmorphismConfig{});
    
    /**
     * @brief End glassmorphism region and composite the result
     */
    void endGlassmorphism();
    
    /**
     * @brief Render a glassmorphism panel (convenience method)
     * @param rect Panel bounds
     * @param cornerRadius Corner radius for rounded rectangle
     * @param config Effect configuration
     */
    void renderGlassPanel(const Rect& rect, float cornerRadius, const GlassmorphismConfig& config = GlassmorphismConfig{});
    
    // ========================================================================
    // Acrylic Effect
    // ========================================================================
    
    /**
     * @brief Begin rendering an acrylic region
     * @param rect The region to apply the effect
     * @param config Effect configuration
     */
    void beginAcrylic(const Rect& rect, const AcrylicConfig& config = AcrylicConfig{});
    
    /**
     * @brief End acrylic region and composite the result
     */
    void endAcrylic();
    
    /**
     * @brief Render an acrylic panel (convenience method)
     * @param rect Panel bounds
     * @param cornerRadius Corner radius for rounded rectangle
     * @param config Effect configuration
     */
    void renderAcrylicPanel(const Rect& rect, float cornerRadius, const AcrylicConfig& config = AcrylicConfig{});
    
    // ========================================================================
    // Shadow Effect
    // ========================================================================
    
    /**
     * @brief Render a shadow for a rectangle
     * @param rect Rectangle bounds (shadow will be rendered behind this)
     * @param config Shadow configuration
     */
    void renderShadow(const Rect& rect, const ShadowConfig& config = ShadowConfig{});
    
    /**
     * @brief Render a shadow for a rounded rectangle
     * @param rect Rectangle bounds
     * @param cornerRadius Corner radius
     * @param config Shadow configuration
     */
    void renderRoundedShadow(const Rect& rect, float cornerRadius, const ShadowConfig& config = ShadowConfig{});
    
    /**
     * @brief Render a shadow for a circle
     * @param cx Center X
     * @param cy Center Y
     * @param radius Circle radius
     * @param config Shadow configuration
     */
    void renderCircleShadow(float cx, float cy, float radius, const ShadowConfig& config = ShadowConfig{});
    
    /**
     * @brief Render a shadow for a widget
     * @param bounds Widget bounds
     * @param cornerRadius Widget corner radius
     * @param blur Shadow blur radius
     * @param offsetX Horizontal offset
     * @param offsetY Vertical offset
     * @param color Shadow color
     * @param spread Shadow spread
     * 
     * Convenience method for rendering shadows behind widgets.
     */
    void renderWidgetShadow(const Rect& bounds, float cornerRadius,
                            float blur, float offsetX, float offsetY,
                            const Color& color, float spread = 0.0f);
    
    // ========================================================================
    // Low-level Blur Operations
    // ========================================================================
    
    /**
     * @brief Apply Gaussian blur to a region
     * @param rect Region to blur
     * @param radius Blur radius
     */
    void applyBlur(const Rect& rect, float radius);
    
    /**
     * @brief Apply box blur (faster but lower quality)
     * @param rect Region to blur
     * @param radius Blur radius
     */
    void applyBoxBlur(const Rect& rect, float radius);
    
    // ========================================================================
    // Utility Methods
    // ========================================================================
    
    /**
     * @brief Generate noise texture for acrylic effect
     * @param width Texture width
     * @param height Texture height
     * @return true if generation succeeded
     */
    bool generateNoiseTexture(uint32_t width, uint32_t height);
    
    /**
     * @brief Check if blur effects are supported on current hardware
     */
    [[nodiscard]] bool isBlurSupported() const;
    
    /**
     * @brief Get the maximum supported blur radius
     */
    [[nodiscard]] float getMaxBlurRadius() const;
    
    /**
     * @brief Set global effect quality (0.0 = lowest, 1.0 = highest)
     * @param quality Quality level affecting blur passes and resolution
     */
    void setQuality(float quality);
    
    /**
     * @brief Get current quality setting
     */
    [[nodiscard]] float getQuality() const;

private:
    VisualEffects();
    ~VisualEffects();
    VisualEffects(const VisualEffects&) = delete;
    VisualEffects& operator=(const VisualEffects&) = delete;
    
    // Internal methods
    bool createBlurPipelines();
    bool createBlurResources();
    bool createNoiseTexture();
    bool createShadowResources();
    
    void renderBlurredRect(const Rect& rect, float blurRadius, const Color& tint, float opacity);
    void renderShadowGeometry(const Rect& rect, float cornerRadius, const ShadowConfig& config);
    
    // Acrylic effect layer rendering helpers
    void renderAcrylicBlurLayers(const Rect& rect, float cornerRadius, float blurRadius, const AcrylicConfig& config);
    void renderAcrylicLuminosityLayer(const Rect& rect, float cornerRadius, const AcrylicConfig& config);
    void renderAcrylicTintLayer(const Rect& rect, float cornerRadius, const AcrylicConfig& config);
    void renderAcrylicNoiseLayer(const Rect& rect, float cornerRadius, const AcrylicConfig& config);
    void renderAcrylicBorder(const Rect& rect, float cornerRadius, const AcrylicConfig& config);
    
    // Utility for noise layer clipping
    bool isPointInRoundedRect(float px, float py, const Rect& rect, float radius);
    
    // Gaussian kernel generation
    std::vector<float> generateGaussianKernel(float sigma, int size);
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace KillerGK
