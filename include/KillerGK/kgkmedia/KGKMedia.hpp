/**
 * @file KGKMedia.hpp
 * @brief Media module for KillerGK
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <memory>
#include <functional>

namespace KGKMedia {

using KillerGK::Rect;

/**
 * @brief Handle to a video player
 */
using VideoHandle = std::shared_ptr<class VideoImpl>;

/**
 * @class Video
 * @brief Video playback class
 */
class Video {
public:
    static Video load(const std::string& path);

    // Playback controls
    void play();
    void pause();
    void stop();
    void seek(float seconds);

    // Properties
    Video& volume(float volume);
    Video& loop(bool enabled);

    // State
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] float duration() const;
    [[nodiscard]] float currentTime() const;
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;

    VideoHandle build();

private:
    Video();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class Screenshot
 * @brief Screenshot capture utility
 */
class Screenshot {
public:
    /**
     * @brief Capture the entire window
     * @param path Output file path
     */
    static bool captureWindow(const std::string& path);

    /**
     * @brief Capture a region of the screen
     * @param rect Region to capture
     * @param path Output file path
     */
    static bool captureRegion(const Rect& rect, const std::string& path);

    /**
     * @brief Capture the entire screen
     * @param path Output file path
     */
    static bool captureScreen(const std::string& path);
};

/**
 * @brief Handle to an image
 */
using ImageHandle = std::shared_ptr<class ImageImpl>;

/**
 * @class Image
 * @brief Image processing class
 */
class Image {
public:
    static Image load(const std::string& path);
    static Image create(int width, int height);

    // Operations
    Image& resize(int width, int height);
    Image& crop(const Rect& rect);
    Image& rotate(float degrees);
    Image& flipHorizontal();
    Image& flipVertical();

    // Filters
    Image& grayscale();
    Image& blur(float radius);
    Image& brightness(float amount);
    Image& contrast(float amount);

    // Save
    bool save(const std::string& path);

    // Properties
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;

    ImageHandle build();

private:
    Image();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

} // namespace KGKMedia
