/**
 * @file KGKMedia.hpp
 * @brief Media module for KillerGK - video playback, screenshot capture, and image processing
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <cstdint>

namespace KGKMedia {

using KillerGK::Rect;
using KillerGK::Color;

/**
 * @brief Video playback state
 */
enum class VideoState {
    Stopped,
    Playing,
    Paused,
    Error
};

/**
 * @brief Video frame data for rendering
 */
struct VideoFrame {
    std::vector<uint8_t> data;  // RGBA pixel data
    int width = 0;
    int height = 0;
    double timestamp = 0.0;
    bool valid = false;
};

/**
 * @brief Handle to a video player
 */
using VideoHandle = std::shared_ptr<class VideoImpl>;

/**
 * @brief Video frame callback type
 */
using FrameCallback = std::function<void(const VideoFrame&)>;

/**
 * @brief Video state change callback type
 */
using StateCallback = std::function<void(VideoState)>;

/**
 * @class Video
 * @brief Video playback class with decoding and frame extraction
 * 
 * Supports common video formats through platform-specific decoders.
 * Provides frame-by-frame access for rendering in widgets.
 */
class Video {
public:
    /**
     * @brief Load a video file
     * @param path Path to the video file
     * @return Video object (check isValid() for success)
     */
    static Video load(const std::string& path);

    /**
     * @brief Check if video was loaded successfully
     */
    [[nodiscard]] bool isValid() const;

    // Playback controls
    void play();
    void pause();
    void stop();
    void seek(float seconds);

    // Properties (Builder pattern)
    Video& volume(float volume);
    Video& loop(bool enabled);
    Video& playbackRate(float rate);

    // Callbacks
    Video& onFrame(FrameCallback callback);
    Video& onStateChange(StateCallback callback);
    Video& onEnd(std::function<void()> callback);

    // State queries
    [[nodiscard]] VideoState state() const;
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] bool isPaused() const;
    [[nodiscard]] bool isStopped() const;
    [[nodiscard]] float duration() const;
    [[nodiscard]] float currentTime() const;
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] float frameRate() const;
    [[nodiscard]] float getVolume() const;
    [[nodiscard]] bool isLooping() const;

    /**
     * @brief Get the current frame for rendering
     * @return Current video frame data
     */
    [[nodiscard]] VideoFrame getCurrentFrame() const;

    /**
     * @brief Update video playback (call each frame)
     * @param deltaTime Time since last update in seconds
     */
    void update(float deltaTime);

    VideoHandle build();

private:
    Video();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @brief Screenshot image format
 */
enum class ImageFormat {
    PNG,
    JPG,
    BMP,
    TGA
};

/**
 * @brief Screenshot capture result
 */
struct CaptureResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint8_t> data;  // Raw RGBA pixel data
    int width = 0;
    int height = 0;
};

/**
 * @class Screenshot
 * @brief Screenshot capture utility for windows and screen regions
 * 
 * Provides platform-specific screenshot capture functionality.
 * Supports saving to various image formats.
 */
class Screenshot {
public:
    /**
     * @brief Capture the entire window content
     * @param windowHandle Native window handle (HWND on Windows)
     * @param path Output file path (format determined by extension)
     * @return true if capture and save succeeded
     */
    static bool captureWindow(void* windowHandle, const std::string& path);

    /**
     * @brief Capture the active/focused window
     * @param path Output file path
     * @return true if capture and save succeeded
     */
    static bool captureActiveWindow(const std::string& path);

    /**
     * @brief Capture a region of the screen
     * @param rect Region to capture (screen coordinates)
     * @param path Output file path
     * @return true if capture and save succeeded
     */
    static bool captureRegion(const Rect& rect, const std::string& path);

    /**
     * @brief Capture the entire primary screen
     * @param path Output file path
     * @return true if capture and save succeeded
     */
    static bool captureScreen(const std::string& path);

    /**
     * @brief Capture the entire primary screen to memory
     * @return CaptureResult with pixel data
     */
    static CaptureResult captureScreenToMemory();

    /**
     * @brief Capture a window to memory
     * @param windowHandle Native window handle
     * @return CaptureResult with pixel data
     */
    static CaptureResult captureWindowToMemory(void* windowHandle);

    /**
     * @brief Capture a region to memory
     * @param rect Region to capture
     * @return CaptureResult with pixel data
     */
    static CaptureResult captureRegionToMemory(const Rect& rect);

    /**
     * @brief Save raw pixel data to file
     * @param data RGBA pixel data
     * @param width Image width
     * @param height Image height
     * @param path Output file path
     * @param format Image format (auto-detected from extension if not specified)
     * @return true if save succeeded
     */
    static bool saveToFile(const uint8_t* data, int width, int height, 
                          const std::string& path, 
                          ImageFormat format = ImageFormat::PNG);

    /**
     * @brief Detect image format from file extension
     * @param path File path
     * @return Detected image format
     */
    static ImageFormat detectFormat(const std::string& path);
};

/**
 * @brief Handle to an image
 */
using ImageHandle = std::shared_ptr<class ImageImpl>;

/**
 * @brief Resize interpolation mode
 */
enum class ResizeMode {
    NearestNeighbor,  // Fast, pixelated
    Bilinear,         // Smooth, good for downscaling
    Bicubic           // High quality, slower
};

/**
 * @brief Image filter type
 */
enum class FilterType {
    Grayscale,
    Sepia,
    Invert,
    Sharpen,
    EdgeDetect,
    Emboss
};

/**
 * @class Image
 * @brief Image processing class with resize, crop, rotate, and filter operations
 * 
 * Provides a fluent Builder Pattern API for image manipulation.
 * Supports loading from files and creating blank images.
 */
class Image {
public:
    /**
     * @brief Load an image from file
     * @param path Path to image file (PNG, JPG, BMP, TGA supported)
     * @return Image object (check isValid() for success)
     */
    static Image load(const std::string& path);

    /**
     * @brief Create a blank image with specified dimensions
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @param fillColor Optional fill color (default: transparent)
     * @return New Image object
     */
    static Image create(int width, int height, const Color& fillColor = Color::Transparent);

    /**
     * @brief Create an image from raw pixel data
     * @param data RGBA pixel data
     * @param width Image width
     * @param height Image height
     * @return New Image object
     */
    static Image fromData(const uint8_t* data, int width, int height);

    /**
     * @brief Check if image is valid
     */
    [[nodiscard]] bool isValid() const;

    // Geometric operations (Builder pattern - returns *this)
    
    /**
     * @brief Resize the image
     * @param width New width
     * @param height New height
     * @param mode Interpolation mode
     */
    Image& resize(int width, int height, ResizeMode mode = ResizeMode::Bilinear);

    /**
     * @brief Crop the image to a region
     * @param rect Region to keep
     */
    Image& crop(const Rect& rect);

    /**
     * @brief Rotate the image
     * @param degrees Rotation angle in degrees (positive = clockwise)
     */
    Image& rotate(float degrees);

    /**
     * @brief Rotate by 90 degrees clockwise
     */
    Image& rotate90();

    /**
     * @brief Rotate by 180 degrees
     */
    Image& rotate180();

    /**
     * @brief Rotate by 270 degrees clockwise (90 counter-clockwise)
     */
    Image& rotate270();

    /**
     * @brief Flip horizontally (mirror)
     */
    Image& flipHorizontal();

    /**
     * @brief Flip vertically
     */
    Image& flipVertical();

    // Color adjustment filters
    
    /**
     * @brief Convert to grayscale
     */
    Image& grayscale();

    /**
     * @brief Apply sepia tone effect
     */
    Image& sepia();

    /**
     * @brief Invert colors
     */
    Image& invert();

    /**
     * @brief Apply box blur
     * @param radius Blur radius in pixels
     */
    Image& blur(float radius);

    /**
     * @brief Apply Gaussian blur
     * @param radius Blur radius
     * @param sigma Gaussian sigma (default: radius/3)
     */
    Image& gaussianBlur(float radius, float sigma = 0.0f);

    /**
     * @brief Adjust brightness
     * @param amount Brightness adjustment (-1.0 to 1.0, 0 = no change)
     */
    Image& brightness(float amount);

    /**
     * @brief Adjust contrast
     * @param amount Contrast adjustment (-1.0 to 1.0, 0 = no change)
     */
    Image& contrast(float amount);

    /**
     * @brief Adjust saturation
     * @param amount Saturation adjustment (-1.0 to 1.0, 0 = no change)
     */
    Image& saturation(float amount);

    /**
     * @brief Adjust hue
     * @param degrees Hue rotation in degrees
     */
    Image& hue(float degrees);

    /**
     * @brief Apply sharpen filter
     * @param amount Sharpening strength (0.0 to 1.0)
     */
    Image& sharpen(float amount = 0.5f);

    /**
     * @brief Apply edge detection filter
     */
    Image& edgeDetect();

    /**
     * @brief Apply emboss effect
     */
    Image& emboss();

    /**
     * @brief Apply a generic filter
     * @param type Filter type to apply
     */
    Image& applyFilter(FilterType type);

    /**
     * @brief Apply a custom convolution kernel
     * @param kernel 3x3 kernel values (9 floats)
     */
    Image& convolve(const float kernel[9]);

    // Pixel access
    
    /**
     * @brief Get pixel color at coordinates
     * @param x X coordinate
     * @param y Y coordinate
     * @return Pixel color (or transparent if out of bounds)
     */
    [[nodiscard]] Color getPixel(int x, int y) const;

    /**
     * @brief Set pixel color at coordinates
     * @param x X coordinate
     * @param y Y coordinate
     * @param color New pixel color
     */
    Image& setPixel(int x, int y, const Color& color);

    /**
     * @brief Fill entire image with color
     * @param color Fill color
     */
    Image& fill(const Color& color);

    /**
     * @brief Get raw pixel data (RGBA format)
     */
    [[nodiscard]] const uint8_t* data() const;

    /**
     * @brief Get mutable raw pixel data
     */
    [[nodiscard]] uint8_t* data();

    // Save operations
    
    /**
     * @brief Save image to file
     * @param path Output file path (format determined by extension)
     * @param quality JPEG quality (1-100, ignored for other formats)
     * @return true if save succeeded
     */
    bool save(const std::string& path, int quality = 90);

    /**
     * @brief Save image to file with specific format
     * @param path Output file path
     * @param format Image format
     * @param quality JPEG quality (1-100)
     * @return true if save succeeded
     */
    bool save(const std::string& path, ImageFormat format, int quality = 90);

    // Properties
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] int channels() const;  // Always 4 (RGBA)
    [[nodiscard]] size_t dataSize() const;

    /**
     * @brief Create a copy of this image
     */
    [[nodiscard]] Image clone() const;

    ImageHandle build();

private:
    Image();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

} // namespace KGKMedia
