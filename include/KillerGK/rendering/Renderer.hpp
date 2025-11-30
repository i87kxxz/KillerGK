/**
 * @file Renderer.hpp
 * @brief Rendering system for KillerGK
 */

#pragma once

#include "../core/Types.hpp"
#include <memory>

namespace KillerGK {

/**
 * @class Renderer
 * @brief Vulkan-based renderer
 */
class Renderer {
public:
    static Renderer& instance();

    /**
     * @brief Initialize the renderer
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Shutdown the renderer
     */
    void shutdown();

    /**
     * @brief Begin a new frame
     */
    void beginFrame();

    /**
     * @brief End the current frame
     */
    void endFrame();

    /**
     * @brief Draw a filled rectangle
     */
    void drawRect(const Rect& rect, const Color& color);

    /**
     * @brief Draw a rounded rectangle
     */
    void drawRoundRect(const Rect& rect, float radius, const Color& color);

    /**
     * @brief Draw a circle
     */
    void drawCircle(float cx, float cy, float radius, const Color& color);

    /**
     * @brief Draw a line
     */
    void drawLine(float x1, float y1, float x2, float y2, const Color& color, float thickness = 1.0f);

    /**
     * @brief Check if renderer is initialized
     */
    [[nodiscard]] bool isInitialized() const;

private:
    Renderer();
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace KillerGK
