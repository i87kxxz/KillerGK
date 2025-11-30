/**
 * @file Renderer2D.hpp
 * @brief 2D rendering primitives for KillerGK
 * 
 * Provides efficient batched rendering of 2D shapes including
 * rectangles, circles, lines, and polygons with anti-aliasing.
 */

#pragma once

#include "../core/Types.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace KillerGK {

/**
 * @brief Configuration for 2D renderer
 */
struct Renderer2DConfig {
    uint32_t maxVertices = 100000;
    uint32_t maxIndices = 300000;
    bool enableAntiAliasing = true;
};

/**
 * @class Renderer2D
 * @brief Batched 2D rendering with anti-aliasing support
 * 
 * Provides efficient rendering of 2D primitives using vertex batching
 * and GPU-accelerated anti-aliasing.
 */
class Renderer2D {
public:
    /**
     * @brief Get the singleton instance
     */
    static Renderer2D& instance();
    
    /**
     * @brief Initialize the 2D renderer
     * @param config Renderer configuration
     * @return true if initialization succeeded
     */
    bool initialize(const Renderer2DConfig& config = Renderer2DConfig{});
    
    /**
     * @brief Shutdown and cleanup
     */
    void shutdown();
    
    /**
     * @brief Check if initialized
     */
    [[nodiscard]] bool isInitialized() const;
    
    /**
     * @brief Begin a new batch
     * @param width Viewport width
     * @param height Viewport height
     */
    void beginBatch(float width, float height);
    
    /**
     * @brief End the current batch and flush to GPU
     */
    void endBatch();
    
    /**
     * @brief Flush the current batch without ending
     */
    void flush();
    
    /**
     * @brief Draw a filled rectangle
     * @param rect Rectangle bounds
     * @param color Fill color
     */
    void drawRect(const Rect& rect, const Color& color);
    
    /**
     * @brief Draw a filled rectangle with per-corner colors
     * @param rect Rectangle bounds
     * @param topLeft Top-left corner color
     * @param topRight Top-right corner color
     * @param bottomRight Bottom-right corner color
     * @param bottomLeft Bottom-left corner color
     */
    void drawRectGradient(const Rect& rect, 
                          const Color& topLeft, const Color& topRight,
                          const Color& bottomRight, const Color& bottomLeft);
    
    /**
     * @brief Draw a rounded rectangle
     * @param rect Rectangle bounds
     * @param radius Corner radius
     * @param color Fill color
     * @param segments Number of segments per corner (default: 8)
     */
    void drawRoundRect(const Rect& rect, float radius, const Color& color, int segments = 8);
    
    /**
     * @brief Draw a filled circle
     * @param cx Center X
     * @param cy Center Y
     * @param radius Circle radius
     * @param color Fill color
     * @param segments Number of segments (default: 32)
     */
    void drawCircle(float cx, float cy, float radius, const Color& color, int segments = 32);
    
    /**
     * @brief Draw a filled ellipse
     * @param cx Center X
     * @param cy Center Y
     * @param rx Horizontal radius
     * @param ry Vertical radius
     * @param color Fill color
     * @param segments Number of segments (default: 32)
     */
    void drawEllipse(float cx, float cy, float rx, float ry, const Color& color, int segments = 32);
    
    /**
     * @brief Draw a line with thickness
     * @param x1 Start X
     * @param y1 Start Y
     * @param x2 End X
     * @param y2 End Y
     * @param color Line color
     * @param thickness Line thickness (default: 1.0)
     */
    void drawLine(float x1, float y1, float x2, float y2, const Color& color, float thickness = 1.0f);
    
    /**
     * @brief Draw a polyline (connected line segments)
     * @param points Array of points
     * @param color Line color
     * @param thickness Line thickness
     * @param closed Whether to close the polyline
     */
    void drawPolyline(const std::vector<Point>& points, const Color& color, 
                      float thickness = 1.0f, bool closed = false);
    
    /**
     * @brief Draw a filled polygon
     * @param points Polygon vertices
     * @param color Fill color
     */
    void drawPolygon(const std::vector<Point>& points, const Color& color);
    
    /**
     * @brief Draw a filled triangle
     * @param p1 First vertex
     * @param p2 Second vertex
     * @param p3 Third vertex
     * @param color Fill color
     */
    void drawTriangle(const Point& p1, const Point& p2, const Point& p3, const Color& color);
    
    /**
     * @brief Draw a rectangle outline
     * @param rect Rectangle bounds
     * @param color Outline color
     * @param thickness Line thickness
     */
    void drawRectOutline(const Rect& rect, const Color& color, float thickness = 1.0f);
    
    /**
     * @brief Draw a circle outline
     * @param cx Center X
     * @param cy Center Y
     * @param radius Circle radius
     * @param color Outline color
     * @param thickness Line thickness
     * @param segments Number of segments
     */
    void drawCircleOutline(float cx, float cy, float radius, const Color& color, 
                           float thickness = 1.0f, int segments = 32);
    
    /**
     * @brief Get statistics for the current frame
     */
    struct Stats {
        uint32_t drawCalls = 0;
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
    };
    [[nodiscard]] Stats getStats() const;
    
    /**
     * @brief Reset statistics
     */
    void resetStats();

private:
    Renderer2D();
    ~Renderer2D();
    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;
    
    bool createVertexBuffer();
    bool createIndexBuffer();
    void updateProjectionMatrix(float width, float height);
    void addVertex(float x, float y, const Color& color, float u = 0.0f, float v = 0.0f);
    void addIndex(uint32_t index);
    void addTriangleIndices(uint32_t i0, uint32_t i1, uint32_t i2);
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace KillerGK
