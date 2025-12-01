/**
 * @file KGK2D.hpp
 * @brief 2D graphics module for KillerGK
 * 
 * Provides complete 2D graphics capabilities including Canvas drawing,
 * Sprites, Particle systems, batch rendering, and tilemaps.
 */

#pragma once

#include "../core/Types.hpp"
#include "../rendering/Texture.hpp"
#include <string>
#include <memory>
#include <vector>
#include <stack>
#include <functional>

namespace KGK2D {

using KillerGK::Color;
using KillerGK::Rect;
using KillerGK::Point;
using KillerGK::Size;
using KillerGK::TextureHandle;

// Forward declarations
class Canvas;
class Sprite;
class ParticleEmitter;
class SpriteBatcher;
class Tilemap;

/**
 * @brief Paint style for drawing operations
 */
struct Paint {
    Color color = Color::White;
    float strokeWidth = 1.0f;
    bool filled = true;
    bool antiAlias = true;
    
    static Paint fill(const Color& c) {
        Paint p;
        p.color = c;
        p.filled = true;
        return p;
    }
    
    static Paint stroke(const Color& c, float width = 1.0f) {
        Paint p;
        p.color = c;
        p.strokeWidth = width;
        p.filled = false;
        return p;
    }
};

/**
 * @brief Text style for text drawing
 */
struct TextStyle {
    std::string fontFamily = "default";
    float fontSize = 16.0f;
    Color color = Color::White;
    bool bold = false;
    bool italic = false;
    
    enum class Align { Left, Center, Right };
    Align align = Align::Left;
    
    enum class Baseline { Top, Middle, Bottom };
    Baseline baseline = Baseline::Top;
};

/**
 * @brief 2D transformation matrix
 */
struct Transform2D {
    float m[6] = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}; // [a, b, c, d, tx, ty]
    
    Transform2D() = default;
    
    static Transform2D identity() {
        return Transform2D();
    }
    
    static Transform2D translation(float tx, float ty) {
        Transform2D t;
        t.m[4] = tx;
        t.m[5] = ty;
        return t;
    }
    
    static Transform2D rotation(float radians) {
        Transform2D t;
        float c = std::cos(radians);
        float s = std::sin(radians);
        t.m[0] = c;  t.m[1] = s;
        t.m[2] = -s; t.m[3] = c;
        return t;
    }
    
    static Transform2D scaling(float sx, float sy) {
        Transform2D t;
        t.m[0] = sx;
        t.m[3] = sy;
        return t;
    }
    
    Transform2D operator*(const Transform2D& other) const {
        Transform2D result;
        result.m[0] = m[0] * other.m[0] + m[2] * other.m[1];
        result.m[1] = m[1] * other.m[0] + m[3] * other.m[1];
        result.m[2] = m[0] * other.m[2] + m[2] * other.m[3];
        result.m[3] = m[1] * other.m[2] + m[3] * other.m[3];
        result.m[4] = m[0] * other.m[4] + m[2] * other.m[5] + m[4];
        result.m[5] = m[1] * other.m[4] + m[3] * other.m[5] + m[5];
        return result;
    }
    
    Point apply(const Point& p) const {
        return Point(
            m[0] * p.x + m[2] * p.y + m[4],
            m[1] * p.x + m[3] * p.y + m[5]
        );
    }
};

/**
 * @brief Path for complex shape drawing
 */
class Path {
public:
    Path() = default;
    
    Path& moveTo(float x, float y);
    Path& lineTo(float x, float y);
    Path& quadraticTo(float cx, float cy, float x, float y);
    Path& cubicTo(float c1x, float c1y, float c2x, float c2y, float x, float y);
    Path& arcTo(float x, float y, float radius, float startAngle, float endAngle);
    Path& close();
    Path& clear();
    
    [[nodiscard]] const std::vector<Point>& getPoints() const { return m_points; }
    [[nodiscard]] bool isClosed() const { return m_closed; }
    
private:
    std::vector<Point> m_points;
    bool m_closed = false;
};

/**
 * @class Canvas
 * @brief 2D drawing canvas with transform stack
 * 
 * Provides immediate-mode 2D drawing with support for shapes,
 * images, text, and transformations.
 */
class Canvas {
public:
    Canvas();
    ~Canvas();
    
    /**
     * @brief Begin drawing on the canvas
     * @param width Canvas width
     * @param height Canvas height
     */
    void begin(float width, float height);
    
    /**
     * @brief End drawing and flush to GPU
     */
    void end();
    
    // Shape drawing
    void drawRect(float x, float y, float w, float h, const Paint& paint);
    void drawRect(const Rect& rect, const Paint& paint);
    void drawRoundRect(float x, float y, float w, float h, float radius, const Paint& paint);
    void drawRoundRect(const Rect& rect, float radius, const Paint& paint);
    void drawCircle(float cx, float cy, float radius, const Paint& paint);
    void drawEllipse(float cx, float cy, float rx, float ry, const Paint& paint);
    void drawLine(float x1, float y1, float x2, float y2, const Paint& paint);
    void drawPolyline(const std::vector<Point>& points, const Paint& paint, bool closed = false);
    void drawPolygon(const std::vector<Point>& points, const Paint& paint);
    void drawPath(const Path& path, const Paint& paint);
    void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, const Paint& paint);
    void drawArc(float cx, float cy, float radius, float startAngle, float sweepAngle, const Paint& paint);
    
    // Bezier curves
    void drawQuadraticBezier(float x1, float y1, float cx, float cy, float x2, float y2, const Paint& paint);
    void drawCubicBezier(float x1, float y1, float c1x, float c1y, float c2x, float c2y, float x2, float y2, const Paint& paint);
    
    // Image drawing
    void drawImage(const TextureHandle& image, float x, float y);
    void drawImage(const TextureHandle& image, float x, float y, float w, float h);
    void drawImage(const TextureHandle& image, const Rect& src, const Rect& dst);
    void drawImage(const TextureHandle& image, const Rect& dst, const Color& tint = Color::White);
    
    // Text drawing
    void drawText(const std::string& text, float x, float y, const TextStyle& style);
    Size measureText(const std::string& text, const TextStyle& style);
    
    // Transform stack
    void save();
    void restore();
    void translate(float x, float y);
    void rotate(float radians);
    void scale(float sx, float sy);
    void transform(const Transform2D& matrix);
    void setTransform(const Transform2D& matrix);
    void resetTransform();
    
    // Clipping
    void clipRect(float x, float y, float w, float h);
    void clipRect(const Rect& rect);
    void resetClip();
    
    // State
    void setGlobalAlpha(float alpha);
    [[nodiscard]] float getGlobalAlpha() const;
    
    // Convenience overloads using Color directly
    void drawRect(float x, float y, float w, float h, const Color& color);
    void drawRoundRect(float x, float y, float w, float h, float radius, const Color& color);
    void drawCircle(float cx, float cy, float radius, const Color& color);
    void drawLine(float x1, float y1, float x2, float y2, const Color& color);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    Point transformPoint(float x, float y) const;
    void applyTransformToRect(float& x, float& y, float& w, float& h) const;
};


/**
 * @brief Handle to a sprite implementation
 */
using SpriteHandle = std::shared_ptr<class SpriteImpl>;

/**
 * @brief Sprite implementation data
 */
class SpriteImpl {
public:
    std::string texturePath;
    TextureHandle texture;
    float x = 0.0f, y = 0.0f;
    float width = 0.0f, height = 0.0f;
    float originX = 0.5f, originY = 0.5f;  // Normalized origin (0-1)
    float rotation = 0.0f;  // Degrees
    float scaleX = 1.0f, scaleY = 1.0f;
    Color tint = Color::White;
    float opacity = 1.0f;
    bool flipX = false, flipY = false;
    
    // Sprite sheet
    int frameCols = 1, frameRows = 1;
    int currentFrame = 0;
    int totalFrames = 1;
    
    // Animation
    bool animating = false;
    float animFps = 0.0f;
    int animStartFrame = 0;
    int animEndFrame = 0;
    bool animLoop = false;
    float animTime = 0.0f;
    
    // Computed values
    Rect getSourceRect() const;
    Rect getDestRect() const;
    Transform2D getTransform() const;
    
    void update(float deltaTime);
    void draw(Canvas& canvas);
};

/**
 * @class Sprite
 * @brief 2D sprite with transformations and animation support
 * 
 * Builder pattern for creating sprites with sprite sheet support,
 * transformations, and frame-based animation.
 */
class Sprite {
public:
    static Sprite create(const std::string& texturePath);
    
    // Position and size
    Sprite& position(float x, float y);
    Sprite& size(float w, float h);
    Sprite& origin(float x, float y);  // Normalized 0-1
    
    // Transformations
    Sprite& rotation(float degrees);
    Sprite& scale(float sx, float sy);
    Sprite& scale(float s);
    
    // Appearance
    Sprite& color(const Color& tint);
    Sprite& opacity(float alpha);
    Sprite& flipX(bool flip);
    Sprite& flipY(bool flip);
    
    // Sprite sheet
    Sprite& frames(int cols, int rows);
    Sprite& frame(int index);
    
    // Animation
    Sprite& animate(float fps, int startFrame, int endFrame, bool loop = true);
    Sprite& stopAnimation();
    
    // Build
    SpriteHandle build();
    
    // Getters for current state
    [[nodiscard]] float getX() const;
    [[nodiscard]] float getY() const;
    [[nodiscard]] float getWidth() const;
    [[nodiscard]] float getHeight() const;
    [[nodiscard]] float getRotation() const;
    [[nodiscard]] int getCurrentFrame() const;
    [[nodiscard]] bool isAnimating() const;
    
private:
    Sprite();
    std::shared_ptr<SpriteImpl> m_impl;
};

/**
 * @brief Single particle data
 */
struct Particle {
    float x, y;           // Position
    float vx, vy;         // Velocity
    float life;           // Remaining lifetime
    float maxLife;        // Initial lifetime
    float size;           // Current size
    float startSize;      // Initial size
    float endSize;        // Final size
    Color color;          // Current color
    Color startColor;     // Initial color
    Color endColor;       // Final color
    float rotation;       // Rotation angle
    float rotationSpeed;  // Rotation speed
    bool active;          // Is particle alive
};

/**
 * @brief Handle to a particle emitter implementation
 */
using ParticleEmitterHandle = std::shared_ptr<class ParticleEmitterImpl>;

/**
 * @brief Particle emitter implementation
 */
class ParticleEmitterImpl {
public:
    // Emitter properties
    float x = 0.0f, y = 0.0f;
    TextureHandle texture;
    float emissionRate = 10.0f;  // Particles per second
    int maxParticles = 1000;
    
    // Particle properties
    float lifetimeMin = 1.0f, lifetimeMax = 2.0f;
    float speedMin = 50.0f, speedMax = 100.0f;
    float angleMin = 0.0f, angleMax = 360.0f;  // Emission angle in degrees
    float sizeStart = 10.0f, sizeEnd = 0.0f;
    Color colorStart = Color::White;
    Color colorEnd = Color(1.0f, 1.0f, 1.0f, 0.0f);
    float gravityX = 0.0f, gravityY = 0.0f;
    float rotationSpeedMin = 0.0f, rotationSpeedMax = 0.0f;
    
    // Emission shape
    enum class EmissionShape { Point, Circle, Rectangle };
    EmissionShape shape = EmissionShape::Point;
    float shapeRadius = 0.0f;
    float shapeWidth = 0.0f, shapeHeight = 0.0f;
    
    // State
    bool emitting = true;
    float emissionAccumulator = 0.0f;
    std::vector<Particle> particles;
    
    void update(float deltaTime);
    void draw(Canvas& canvas);
    void emit(int count = 1);
    void reset();
    [[nodiscard]] int getActiveParticleCount() const;
};

/**
 * @class ParticleEmitter
 * @brief Particle system emitter with configurable properties
 * 
 * Builder pattern for creating particle emitters with physics-based
 * simulation and customizable particle properties.
 */
class ParticleEmitter {
public:
    static ParticleEmitter create();
    
    // Position
    ParticleEmitter& position(float x, float y);
    
    // Appearance
    ParticleEmitter& texture(const std::string& path);
    ParticleEmitter& texture(const TextureHandle& tex);
    
    // Emission
    ParticleEmitter& emissionRate(float particlesPerSecond);
    ParticleEmitter& maxParticles(int max);
    
    // Particle properties
    ParticleEmitter& lifetime(float min, float max);
    ParticleEmitter& speed(float min, float max);
    ParticleEmitter& angle(float min, float max);  // Degrees
    ParticleEmitter& size(float start, float end);
    ParticleEmitter& color(const Color& start, const Color& end);
    ParticleEmitter& gravity(float x, float y);
    ParticleEmitter& rotationSpeed(float min, float max);
    
    // Emission shape
    ParticleEmitter& emitFromPoint();
    ParticleEmitter& emitFromCircle(float radius);
    ParticleEmitter& emitFromRectangle(float width, float height);
    
    // Control
    ParticleEmitter& start();
    ParticleEmitter& stop();
    
    ParticleEmitterHandle build();
    
private:
    ParticleEmitter();
    std::shared_ptr<ParticleEmitterImpl> m_impl;
};

/**
 * @brief Batch item for sprite batching
 */
struct BatchItem {
    TextureHandle texture;
    Rect srcRect;
    Rect dstRect;
    Color tint;
    float rotation;
    float originX, originY;
    int zOrder;
};

/**
 * @class SpriteBatcher
 * @brief Optimized batch rendering for sprites
 * 
 * Collects sprites and renders them in batches to minimize
 * draw calls and improve performance.
 */
class SpriteBatcher {
public:
    SpriteBatcher();
    ~SpriteBatcher();
    
    /**
     * @brief Begin a new batch
     * @param width Viewport width
     * @param height Viewport height
     */
    void begin(float width, float height);
    
    /**
     * @brief Add a sprite to the batch
     */
    void draw(const SpriteHandle& sprite);
    void draw(const TextureHandle& texture, const Rect& dst, const Color& tint = Color::White);
    void draw(const TextureHandle& texture, const Rect& src, const Rect& dst, 
              const Color& tint = Color::White, float rotation = 0.0f,
              float originX = 0.5f, float originY = 0.5f);
    
    /**
     * @brief Set z-order for subsequent draws
     */
    void setZOrder(int z);
    
    /**
     * @brief End batch and flush to GPU
     */
    void end();
    
    /**
     * @brief Get statistics
     */
    struct Stats {
        int drawCalls = 0;
        int spriteCount = 0;
        int batchCount = 0;
    };
    [[nodiscard]] Stats getStats() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    void flush();
    void sortBatch();
};

/**
 * @brief Single tile in a tilemap
 */
struct Tile {
    int tileId = -1;      // -1 = empty
    bool flipX = false;
    bool flipY = false;
    bool flipDiagonal = false;
    Color tint = Color::White;
};

/**
 * @brief Tileset definition
 */
struct Tileset {
    TextureHandle texture;
    int tileWidth = 32;
    int tileHeight = 32;
    int columns = 0;
    int rows = 0;
    int firstGid = 1;
    int spacing = 0;
    int margin = 0;
    
    [[nodiscard]] Rect getTileRect(int tileId) const;
};

/**
 * @brief Tilemap layer
 */
struct TilemapLayer {
    std::string name;
    std::vector<Tile> tiles;
    int width = 0;
    int height = 0;
    float opacity = 1.0f;
    bool visible = true;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float parallaxX = 1.0f;
    float parallaxY = 1.0f;
    
    [[nodiscard]] Tile& getTile(int x, int y);
    [[nodiscard]] const Tile& getTile(int x, int y) const;
    void setTile(int x, int y, const Tile& tile);
    void resize(int newWidth, int newHeight);
};

/**
 * @class Tilemap
 * @brief Tile-based map rendering with culling
 * 
 * Efficient rendering of large tile-based maps with automatic
 * culling of off-screen tiles.
 */
class Tilemap {
public:
    Tilemap();
    ~Tilemap();
    
    /**
     * @brief Create a new tilemap
     * @param width Map width in tiles
     * @param height Map height in tiles
     * @param tileWidth Tile width in pixels
     * @param tileHeight Tile height in pixels
     */
    void create(int width, int height, int tileWidth, int tileHeight);
    
    /**
     * @brief Add a tileset
     */
    void addTileset(const Tileset& tileset);
    
    /**
     * @brief Add a layer
     * @return Layer index
     */
    int addLayer(const std::string& name);
    
    /**
     * @brief Get layer by index
     */
    TilemapLayer& getLayer(int index);
    [[nodiscard]] const TilemapLayer& getLayer(int index) const;
    
    /**
     * @brief Get layer by name
     */
    TilemapLayer* getLayerByName(const std::string& name);
    
    /**
     * @brief Set tile at position
     */
    void setTile(int layer, int x, int y, int tileId);
    void setTile(int layer, int x, int y, const Tile& tile);
    
    /**
     * @brief Get tile at position
     */
    [[nodiscard]] const Tile& getTile(int layer, int x, int y) const;
    
    /**
     * @brief Draw the tilemap
     * @param canvas Canvas to draw on
     * @param viewRect Visible area for culling
     */
    void draw(Canvas& canvas, const Rect& viewRect);
    void draw(Canvas& canvas, float viewX, float viewY, float viewWidth, float viewHeight);
    
    /**
     * @brief Convert world coordinates to tile coordinates
     */
    [[nodiscard]] Point worldToTile(float worldX, float worldY) const;
    
    /**
     * @brief Convert tile coordinates to world coordinates
     */
    [[nodiscard]] Point tileToWorld(int tileX, int tileY) const;
    
    // Getters
    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;
    [[nodiscard]] int getTileWidth() const;
    [[nodiscard]] int getTileHeight() const;
    [[nodiscard]] int getLayerCount() const;
    [[nodiscard]] int getPixelWidth() const;
    [[nodiscard]] int getPixelHeight() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    const Tileset* findTileset(int tileId) const;
    void drawLayer(Canvas& canvas, const TilemapLayer& layer, const Rect& viewRect);
};

} // namespace KGK2D
