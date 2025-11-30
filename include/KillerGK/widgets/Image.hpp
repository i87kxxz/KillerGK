/**
 * @file Image.hpp
 * @brief Image widget for KillerGK with Builder Pattern API
 */

#pragma once

#include "Widget.hpp"
#include "../core/Types.hpp"
#include "../rendering/Texture.hpp"
#include <string>
#include <memory>

namespace KillerGK {

/**
 * @enum ImageScaleMode
 * @brief Image scaling modes
 */
enum class ImageScaleMode {
    Fit,        ///< Scale to fit within bounds, maintaining aspect ratio
    Fill,       ///< Scale to fill bounds, maintaining aspect ratio (may crop)
    Stretch,    ///< Stretch to fill bounds exactly (may distort)
    None,       ///< Display at original size
    Tile        ///< Tile the image to fill bounds
};

/**
 * @enum ImageAlignment
 * @brief Image alignment within widget bounds
 */
enum class ImageAlignment {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

/**
 * @class Image
 * @brief Image display widget with scaling and alignment support
 * 
 * Supports loading images from files or textures, with various
 * scaling modes and alignment options.
 * 
 * Example:
 * @code
 * auto image = Image::create()
 *     .source("images/logo.png")
 *     .scaleMode(ImageScaleMode::Fit)
 *     .alignment(ImageAlignment::Center)
 *     .width(200)
 *     .height(150);
 * @endcode
 */
class Image : public Widget {
public:
    virtual ~Image() = default;

    /**
     * @brief Create a new Image instance
     * @return New Image with default properties
     */
    static Image create();

    // =========================================================================
    // Image Source
    // =========================================================================

    /**
     * @brief Set image source from file path
     * @param path Path to image file
     * @return Reference to this Image for chaining
     */
    Image& source(const std::string& path);

    /**
     * @brief Get image source path
     * @return Current source path
     */
    [[nodiscard]] const std::string& getSource() const;

    /**
     * @brief Set image from texture handle
     * @param texture Texture handle
     * @return Reference to this Image for chaining
     */
    Image& texture(TextureHandle texture);

    /**
     * @brief Get texture handle
     * @return Current texture or nullptr
     */
    [[nodiscard]] TextureHandle getTexture() const;

    /**
     * @brief Check if image is loaded
     * @return true if image is loaded and ready
     */
    [[nodiscard]] bool isLoaded() const;

    /**
     * @brief Get original image width
     * @return Original width in pixels (0 if not loaded)
     */
    [[nodiscard]] int getImageWidth() const;

    /**
     * @brief Get original image height
     * @return Original height in pixels (0 if not loaded)
     */
    [[nodiscard]] int getImageHeight() const;

    // =========================================================================
    // Scaling and Alignment
    // =========================================================================

    /**
     * @brief Set image scaling mode
     * @param mode Scaling mode
     * @return Reference to this Image for chaining
     */
    Image& scaleMode(ImageScaleMode mode);

    /**
     * @brief Get image scaling mode
     * @return Current scaling mode
     */
    [[nodiscard]] ImageScaleMode getScaleMode() const;

    /**
     * @brief Set image alignment within bounds
     * @param align Alignment
     * @return Reference to this Image for chaining
     */
    Image& alignment(ImageAlignment align);

    /**
     * @brief Get image alignment
     * @return Current alignment
     */
    [[nodiscard]] ImageAlignment getAlignment() const;

    // =========================================================================
    // Visual Properties
    // =========================================================================

    /**
     * @brief Set tint color
     * @param color Tint color (multiplied with image colors)
     * @return Reference to this Image for chaining
     */
    Image& tint(const Color& color);

    /**
     * @brief Get tint color
     * @return Current tint color
     */
    [[nodiscard]] const Color& getTint() const;

    /**
     * @brief Enable or disable grayscale rendering
     * @param enabled Whether grayscale is enabled
     * @return Reference to this Image for chaining
     */
    Image& grayscale(bool enabled);

    /**
     * @brief Check if grayscale is enabled
     * @return true if grayscale
     */
    [[nodiscard]] bool isGrayscale() const;

    /**
     * @brief Set image rotation in degrees
     * @param degrees Rotation angle
     * @return Reference to this Image for chaining
     */
    Image& rotation(float degrees);

    /**
     * @brief Get image rotation
     * @return Current rotation in degrees
     */
    [[nodiscard]] float getRotation() const;

    /**
     * @brief Enable or disable horizontal flip
     * @param flip Whether to flip horizontally
     * @return Reference to this Image for chaining
     */
    Image& flipHorizontal(bool flip);

    /**
     * @brief Check if horizontal flip is enabled
     * @return true if flipped horizontally
     */
    [[nodiscard]] bool isFlippedHorizontal() const;

    /**
     * @brief Enable or disable vertical flip
     * @param flip Whether to flip vertically
     * @return Reference to this Image for chaining
     */
    Image& flipVertical(bool flip);

    /**
     * @brief Check if vertical flip is enabled
     * @return true if flipped vertically
     */
    [[nodiscard]] bool isFlippedVertical() const;

    // =========================================================================
    // Source Rectangle (for sprite sheets)
    // =========================================================================

    /**
     * @brief Set source rectangle for displaying part of image
     * @param rect Source rectangle in image coordinates
     * @return Reference to this Image for chaining
     */
    Image& sourceRect(const Rect& rect);

    /**
     * @brief Get source rectangle
     * @return Current source rectangle
     */
    [[nodiscard]] const Rect& getSourceRect() const;

    /**
     * @brief Check if using a source rectangle
     * @return true if source rectangle is set
     */
    [[nodiscard]] bool hasSourceRect() const;

    /**
     * @brief Clear source rectangle (use full image)
     * @return Reference to this Image for chaining
     */
    Image& clearSourceRect();

    // =========================================================================
    // Loading
    // =========================================================================

    /**
     * @brief Load image from current source path
     * @return true if loading succeeded
     */
    bool load();

    /**
     * @brief Unload image and free resources
     */
    void unload();

    /**
     * @brief Set callback for when image finishes loading
     * @param callback Function called on load complete
     * @return Reference to this Image for chaining
     */
    Image& onLoad(std::function<void(bool success)> callback);

    /**
     * @brief Set fallback image path for load failures
     * @param path Path to fallback image
     * @return Reference to this Image for chaining
     */
    Image& fallback(const std::string& path);

    /**
     * @brief Get fallback image path
     * @return Current fallback path
     */
    [[nodiscard]] const std::string& getFallback() const;

    // =========================================================================
    // Computed Properties
    // =========================================================================

    /**
     * @brief Get computed display rectangle based on scale mode and alignment
     * @return Rectangle where image will be rendered
     */
    [[nodiscard]] Rect getDisplayRect() const;

    /**
     * @brief Get aspect ratio of original image
     * @return Aspect ratio (width/height) or 1.0 if not loaded
     */
    [[nodiscard]] float getAspectRatio() const;

protected:
    Image();

    /**
     * @brief Calculate display rectangle based on current settings
     */
    void calculateDisplayRect();

    struct ImageData;
    std::shared_ptr<ImageData> m_imageData;
};

} // namespace KillerGK
