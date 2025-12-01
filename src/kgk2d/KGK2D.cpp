/**
 * @file KGK2D.cpp
 * @brief 2D graphics module implementation
 */

#include "KillerGK/kgk2d/KGK2D.hpp"
#include "KillerGK/rendering/Renderer2D.hpp"
#include "KillerGK/rendering/Texture.hpp"
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>

namespace KGK2D {

static constexpr float PI = 3.14159265358979323846f;
static constexpr float DEG_TO_RAD = PI / 180.0f;

static std::mt19937& getRandomEngine() {
    static std::mt19937 engine(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
    return engine;
}

static float randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(getRandomEngine());
}

// ============================================================================
// Path Implementation
// ============================================================================

Path& Path::moveTo(float x, float y) {
    m_points.clear();
    m_points.push_back(Point(x, y));
    m_closed = false;
    return *this;
}

Path& Path::lineTo(float x, float y) {
    m_points.push_back(Point(x, y));
    return *this;
}

Path& Path::quadraticTo(float cx, float cy, float x, float y) {
    if (m_points.empty()) return *this;
    const Point& start = m_points.back();
    const int segments = 16;
    for (int i = 1; i <= segments; i++) {
        float t = static_cast<float>(i) / segments;
        float mt = 1.0f - t;
        float px = mt * mt * start.x + 2 * mt * t * cx + t * t * x;
        float py = mt * mt * start.y + 2 * mt * t * cy + t * t * y;
        m_points.push_back(Point(px, py));
    }
    return *this;
}

Path& Path::cubicTo(float c1x, float c1y, float c2x, float c2y, float x, float y) {
    if (m_points.empty()) return *this;
    const Point& start = m_points.back();
    const int segments = 20;
    for (int i = 1; i <= segments; i++) {
        float t = static_cast<float>(i) / segments;
        float mt = 1.0f - t;
        float mt2 = mt * mt;
        float mt3 = mt2 * mt;
        float t2 = t * t;
        float t3 = t2 * t;
        float px = mt3 * start.x + 3 * mt2 * t * c1x + 3 * mt * t2 * c2x + t3 * x;
        float py = mt3 * start.y + 3 * mt2 * t * c1y + 3 * mt * t2 * c2y + t3 * y;
        m_points.push_back(Point(px, py));
    }
    return *this;
}

Path& Path::arcTo(float x, float y, float radius, float startAngle, float endAngle) {
    const int segments = std::max(8, static_cast<int>(std::abs(endAngle - startAngle) / (PI / 16)));
    float angleStep = (endAngle - startAngle) / segments;
    for (int i = 0; i <= segments; i++) {
        float angle = startAngle + i * angleStep;
        m_points.push_back(Point(x + radius * std::cos(angle), y + radius * std::sin(angle)));
    }
    return *this;
}

Path& Path::close() { m_closed = true; return *this; }
Path& Path::clear() { m_points.clear(); m_closed = false; return *this; }

// ============================================================================
// Canvas Implementation
// ============================================================================

struct Canvas::Impl {
    std::stack<Transform2D> transformStack;
    Transform2D currentTransform;
    float globalAlpha = 1.0f;
    float canvasWidth = 0.0f;
    float canvasHeight = 0.0f;
    bool active = false;
    bool hasClip = false;
    Rect clipRect;
};

Canvas::Canvas() : m_impl(std::make_unique<Impl>()) {
    m_impl->currentTransform = Transform2D::identity();
}

Canvas::~Canvas() = default;

void Canvas::begin(float width, float height) {
    m_impl->canvasWidth = width;
    m_impl->canvasHeight = height;
    m_impl->active = true;
    m_impl->currentTransform = Transform2D::identity();
    while (!m_impl->transformStack.empty()) m_impl->transformStack.pop();
    KillerGK::Renderer2D::instance().beginBatch(width, height);
}

void Canvas::end() {
    KillerGK::Renderer2D::instance().endBatch();
    m_impl->active = false;
}

Point Canvas::transformPoint(float x, float y) const {
    return m_impl->currentTransform.apply(Point(x, y));
}

void Canvas::applyTransformToRect(float& x, float& y, float& w, float& h) const {
    Point p = transformPoint(x, y);
    x = p.x; y = p.y;
    w *= m_impl->currentTransform.m[0];
    h *= m_impl->currentTransform.m[3];
}

void Canvas::drawRect(float x, float y, float w, float h, const Paint& paint) {
    Point p = transformPoint(x, y);
    Color c = paint.color;
    c.a *= m_impl->globalAlpha;
    auto& renderer = KillerGK::Renderer2D::instance();
    if (paint.filled) {
        renderer.drawRect(Rect(p.x, p.y, w * m_impl->currentTransform.m[0], h * m_impl->currentTransform.m[3]), c);
    } else {
        renderer.drawRectOutline(Rect(p.x, p.y, w * m_impl->currentTransform.m[0], h * m_impl->currentTransform.m[3]), c, paint.strokeWidth);
    }
}

void Canvas::drawRect(const Rect& rect, const Paint& paint) {
    drawRect(rect.x, rect.y, rect.width, rect.height, paint);
}

void Canvas::drawRoundRect(float x, float y, float w, float h, float radius, const Paint& paint) {
    Point p = transformPoint(x, y);
    Color c = paint.color;
    c.a *= m_impl->globalAlpha;
    KillerGK::Renderer2D::instance().drawRoundRect(Rect(p.x, p.y, w * m_impl->currentTransform.m[0], h * m_impl->currentTransform.m[3]), radius, c);
}

void Canvas::drawRoundRect(const Rect& rect, float radius, const Paint& paint) {
    drawRoundRect(rect.x, rect.y, rect.width, rect.height, radius, paint);
}

void Canvas::drawCircle(float cx, float cy, float radius, const Paint& paint) {
    Point p = transformPoint(cx, cy);
    Color c = paint.color;
    c.a *= m_impl->globalAlpha;
    auto& renderer = KillerGK::Renderer2D::instance();
    if (paint.filled) {
        renderer.drawCircle(p.x, p.y, radius * m_impl->currentTransform.m[0], c);
    } else {
        renderer.drawCircleOutline(p.x, p.y, radius * m_impl->currentTransform.m[0], c, paint.strokeWidth);
    }
}

void Canvas::drawEllipse(float cx, float cy, float rx, float ry, const Paint& paint) {
    Point p = transformPoint(cx, cy);
    Color c = paint.color;
    c.a *= m_impl->globalAlpha;
    KillerGK::Renderer2D::instance().drawEllipse(p.x, p.y, rx * m_impl->currentTransform.m[0], ry * m_impl->currentTransform.m[3], c);
}

void Canvas::drawLine(float x1, float y1, float x2, float y2, const Paint& paint) {
    Point p1 = transformPoint(x1, y1);
    Point p2 = transformPoint(x2, y2);
    Color c = paint.color;
    c.a *= m_impl->globalAlpha;
    KillerGK::Renderer2D::instance().drawLine(p1.x, p1.y, p2.x, p2.y, c, paint.strokeWidth);
}

void Canvas::drawPolyline(const std::vector<Point>& points, const Paint& paint, bool closed) {
    if (points.size() < 2) return;
    std::vector<Point> transformed;
    transformed.reserve(points.size());
    for (const auto& p : points) transformed.push_back(transformPoint(p.x, p.y));
    Color c = paint.color;
    c.a *= m_impl->globalAlpha;
    KillerGK::Renderer2D::instance().drawPolyline(transformed, c, paint.strokeWidth, closed);
}

void Canvas::drawPolygon(const std::vector<Point>& points, const Paint& paint) {
    if (points.size() < 3) return;
    std::vector<Point> transformed;
    transformed.reserve(points.size());
    for (const auto& p : points) transformed.push_back(transformPoint(p.x, p.y));
    Color c = paint.color;
    c.a *= m_impl->globalAlpha;
    auto& renderer = KillerGK::Renderer2D::instance();
    if (paint.filled) renderer.drawPolygon(transformed, c);
    else renderer.drawPolyline(transformed, c, paint.strokeWidth, true);
}

void Canvas::drawPath(const Path& path, const Paint& paint) {
    const auto& points = path.getPoints();
    if (points.size() < 2) return;
    if (paint.filled && path.isClosed()) drawPolygon(points, paint);
    else drawPolyline(points, paint, path.isClosed());
}

void Canvas::drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, const Paint& paint) {
    Point p1 = transformPoint(x1, y1);
    Point p2 = transformPoint(x2, y2);
    Point p3 = transformPoint(x3, y3);
    Color c = paint.color;
    c.a *= m_impl->globalAlpha;
    auto& renderer = KillerGK::Renderer2D::instance();
    if (paint.filled) renderer.drawTriangle(p1, p2, p3, c);
    else {
        renderer.drawLine(p1.x, p1.y, p2.x, p2.y, c, paint.strokeWidth);
        renderer.drawLine(p2.x, p2.y, p3.x, p3.y, c, paint.strokeWidth);
        renderer.drawLine(p3.x, p3.y, p1.x, p1.y, c, paint.strokeWidth);
    }
}

void Canvas::drawArc(float cx, float cy, float radius, float startAngle, float sweepAngle, const Paint& paint) {
    Point center = transformPoint(cx, cy);
    float r = radius * m_impl->currentTransform.m[0];
    Color c = paint.color;
    c.a *= m_impl->globalAlpha;
    const int segments = std::max(8, static_cast<int>(std::abs(sweepAngle) / (PI / 16)));
    float angleStep = sweepAngle / segments;
    std::vector<Point> points;
    for (int i = 0; i <= segments; i++) {
        float angle = startAngle + i * angleStep;
        points.push_back(Point(center.x + r * std::cos(angle), center.y + r * std::sin(angle)));
    }
    KillerGK::Renderer2D::instance().drawPolyline(points, c, paint.strokeWidth, false);
}

void Canvas::drawQuadraticBezier(float x1, float y1, float cx, float cy, float x2, float y2, const Paint& paint) {
    Path path;
    path.moveTo(x1, y1).quadraticTo(cx, cy, x2, y2);
    drawPath(path, paint);
}

void Canvas::drawCubicBezier(float x1, float y1, float c1x, float c1y, float c2x, float c2y, float x2, float y2, const Paint& paint) {
    Path path;
    path.moveTo(x1, y1).cubicTo(c1x, c1y, c2x, c2y, x2, y2);
    drawPath(path, paint);
}

void Canvas::drawImage(const TextureHandle& image, float x, float y) {
    if (!image) return;
    drawImage(image, x, y, static_cast<float>(image->getWidth()), static_cast<float>(image->getHeight()));
}

void Canvas::drawImage(const TextureHandle& image, float x, float y, float w, float h) {
    if (!image) return;
    Point p = transformPoint(x, y);
    Rect dst(p.x, p.y, w * m_impl->currentTransform.m[0], h * m_impl->currentTransform.m[3]);
    Color tint = Color::White;
    tint.a = m_impl->globalAlpha;
    KillerGK::Renderer2D::instance().drawTexturedRect(dst, image, tint);
}

void Canvas::drawImage(const TextureHandle& image, const Rect& src, const Rect& dst) {
    if (!image) return;
    Point p = transformPoint(dst.x, dst.y);
    Rect transformedDst(p.x, p.y, dst.width * m_impl->currentTransform.m[0], dst.height * m_impl->currentTransform.m[3]);
    Color tint = Color::White;
    tint.a = m_impl->globalAlpha;
    KillerGK::Renderer2D::instance().drawTexturedRect(transformedDst, image, src, tint);
}

void Canvas::drawImage(const TextureHandle& image, const Rect& dst, const Color& tint) {
    if (!image) return;
    Point p = transformPoint(dst.x, dst.y);
    Rect transformedDst(p.x, p.y, dst.width * m_impl->currentTransform.m[0], dst.height * m_impl->currentTransform.m[3]);
    Color c = tint;
    c.a *= m_impl->globalAlpha;
    KillerGK::Renderer2D::instance().drawTexturedRect(transformedDst, image, c);
}

void Canvas::drawText(const std::string&, float, float, const TextStyle&) {}

Size Canvas::measureText(const std::string& text, const TextStyle& style) {
    return Size(static_cast<float>(text.length()) * style.fontSize * 0.6f, style.fontSize);
}

void Canvas::save() { m_impl->transformStack.push(m_impl->currentTransform); }

void Canvas::restore() {
    if (!m_impl->transformStack.empty()) {
        m_impl->currentTransform = m_impl->transformStack.top();
        m_impl->transformStack.pop();
    }
}

void Canvas::translate(float x, float y) { m_impl->currentTransform = m_impl->currentTransform * Transform2D::translation(x, y); }
void Canvas::rotate(float radians) { m_impl->currentTransform = m_impl->currentTransform * Transform2D::rotation(radians); }
void Canvas::scale(float sx, float sy) { m_impl->currentTransform = m_impl->currentTransform * Transform2D::scaling(sx, sy); }
void Canvas::transform(const Transform2D& matrix) { m_impl->currentTransform = m_impl->currentTransform * matrix; }
void Canvas::setTransform(const Transform2D& matrix) { m_impl->currentTransform = matrix; }
void Canvas::resetTransform() { m_impl->currentTransform = Transform2D::identity(); }
void Canvas::clipRect(float x, float y, float w, float h) { m_impl->hasClip = true; m_impl->clipRect = Rect(x, y, w, h); }
void Canvas::clipRect(const Rect& rect) { clipRect(rect.x, rect.y, rect.width, rect.height); }
void Canvas::resetClip() { m_impl->hasClip = false; }
void Canvas::setGlobalAlpha(float alpha) { m_impl->globalAlpha = std::clamp(alpha, 0.0f, 1.0f); }
float Canvas::getGlobalAlpha() const { return m_impl->globalAlpha; }

void Canvas::drawRect(float x, float y, float w, float h, const Color& color) { drawRect(x, y, w, h, Paint::fill(color)); }
void Canvas::drawRoundRect(float x, float y, float w, float h, float radius, const Color& color) { drawRoundRect(x, y, w, h, radius, Paint::fill(color)); }
void Canvas::drawCircle(float cx, float cy, float radius, const Color& color) { drawCircle(cx, cy, radius, Paint::fill(color)); }
void Canvas::drawLine(float x1, float y1, float x2, float y2, const Color& color) { drawLine(x1, y1, x2, y2, Paint::stroke(color, 1.0f)); }


// ============================================================================
// SpriteImpl Implementation
// ============================================================================

Rect SpriteImpl::getSourceRect() const {
    if (frameCols <= 1 && frameRows <= 1) {
        if (texture) return Rect(0, 0, static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight()));
        return Rect(0, 0, width, height);
    }
    float frameWidth = texture ? static_cast<float>(texture->getWidth()) / frameCols : width / frameCols;
    float frameHeight = texture ? static_cast<float>(texture->getHeight()) / frameRows : height / frameRows;
    int col = currentFrame % frameCols;
    int row = currentFrame / frameCols;
    return Rect(col * frameWidth, row * frameHeight, frameWidth, frameHeight);
}

Rect SpriteImpl::getDestRect() const {
    float w = width > 0 ? width : (texture ? static_cast<float>(texture->getWidth()) / frameCols : 0);
    float h = height > 0 ? height : (texture ? static_cast<float>(texture->getHeight()) / frameRows : 0);
    w *= scaleX;
    h *= scaleY;
    return Rect(x - w * originX, y - h * originY, w, h);
}

Transform2D SpriteImpl::getTransform() const {
    Transform2D t = Transform2D::identity();
    t = t * Transform2D::translation(x, y);
    if (rotation != 0.0f) t = t * Transform2D::rotation(rotation * DEG_TO_RAD);
    float sx = scaleX * (flipX ? -1.0f : 1.0f);
    float sy = scaleY * (flipY ? -1.0f : 1.0f);
    t = t * Transform2D::scaling(sx, sy);
    float w = width > 0 ? width : (texture ? static_cast<float>(texture->getWidth()) / frameCols : 0);
    float h = height > 0 ? height : (texture ? static_cast<float>(texture->getHeight()) / frameRows : 0);
    t = t * Transform2D::translation(-w * originX, -h * originY);
    return t;
}

void SpriteImpl::update(float deltaTime) {
    if (!animating) return;
    animTime += deltaTime;
    float frameDuration = 1.0f / animFps;
    while (animTime >= frameDuration) {
        animTime -= frameDuration;
        currentFrame++;
        if (currentFrame > animEndFrame) {
            if (animLoop) currentFrame = animStartFrame;
            else { currentFrame = animEndFrame; animating = false; }
        }
    }
}

void SpriteImpl::draw(Canvas& canvas) {
    if (!texture) return;
    Rect src = getSourceRect();
    Rect dst = getDestRect();
    Color c = tint;
    c.a *= opacity;
    canvas.save();
    if (rotation != 0.0f) {
        canvas.translate(x, y);
        canvas.rotate(rotation * DEG_TO_RAD);
        canvas.translate(-x, -y);
    }
    canvas.drawImage(texture, src, dst);
    canvas.restore();
}

// ============================================================================
// Sprite Implementation
// ============================================================================

Sprite::Sprite() : m_impl(std::make_shared<SpriteImpl>()) {}

Sprite Sprite::create(const std::string& texturePath) {
    Sprite sprite;
    sprite.m_impl->texturePath = texturePath;
    sprite.m_impl->texture = KillerGK::Texture::loadFromFile(texturePath);
    if (sprite.m_impl->texture) {
        sprite.m_impl->width = static_cast<float>(sprite.m_impl->texture->getWidth());
        sprite.m_impl->height = static_cast<float>(sprite.m_impl->texture->getHeight());
    }
    return sprite;
}

Sprite& Sprite::position(float x, float y) { m_impl->x = x; m_impl->y = y; return *this; }
Sprite& Sprite::size(float w, float h) { m_impl->width = w; m_impl->height = h; return *this; }
Sprite& Sprite::origin(float x, float y) { m_impl->originX = x; m_impl->originY = y; return *this; }
Sprite& Sprite::rotation(float degrees) { m_impl->rotation = degrees; return *this; }
Sprite& Sprite::scale(float sx, float sy) { m_impl->scaleX = sx; m_impl->scaleY = sy; return *this; }
Sprite& Sprite::scale(float s) { return scale(s, s); }
Sprite& Sprite::color(const Color& tint) { m_impl->tint = tint; return *this; }
Sprite& Sprite::opacity(float alpha) { m_impl->opacity = std::clamp(alpha, 0.0f, 1.0f); return *this; }
Sprite& Sprite::flipX(bool flip) { m_impl->flipX = flip; return *this; }
Sprite& Sprite::flipY(bool flip) { m_impl->flipY = flip; return *this; }

Sprite& Sprite::frames(int cols, int rows) {
    m_impl->frameCols = std::max(1, cols);
    m_impl->frameRows = std::max(1, rows);
    m_impl->totalFrames = m_impl->frameCols * m_impl->frameRows;
    return *this;
}

Sprite& Sprite::frame(int index) {
    m_impl->currentFrame = std::clamp(index, 0, m_impl->totalFrames - 1);
    return *this;
}

Sprite& Sprite::animate(float fps, int startFrame, int endFrame, bool loop) {
    m_impl->animating = true;
    m_impl->animFps = fps;
    m_impl->animStartFrame = std::clamp(startFrame, 0, m_impl->totalFrames - 1);
    m_impl->animEndFrame = std::clamp(endFrame, 0, m_impl->totalFrames - 1);
    m_impl->animLoop = loop;
    m_impl->animTime = 0.0f;
    m_impl->currentFrame = m_impl->animStartFrame;
    return *this;
}

Sprite& Sprite::stopAnimation() { m_impl->animating = false; return *this; }
SpriteHandle Sprite::build() { return m_impl; }
float Sprite::getX() const { return m_impl->x; }
float Sprite::getY() const { return m_impl->y; }
float Sprite::getWidth() const { return m_impl->width; }
float Sprite::getHeight() const { return m_impl->height; }
float Sprite::getRotation() const { return m_impl->rotation; }
int Sprite::getCurrentFrame() const { return m_impl->currentFrame; }
bool Sprite::isAnimating() const { return m_impl->animating; }

// ============================================================================
// ParticleEmitterImpl Implementation
// ============================================================================

void ParticleEmitterImpl::update(float deltaTime) {
    for (auto& p : particles) {
        if (!p.active) continue;
        p.life -= deltaTime;
        if (p.life <= 0.0f) { p.active = false; continue; }
        p.vx += gravityX * deltaTime;
        p.vy += gravityY * deltaTime;
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
        p.rotation += p.rotationSpeed * deltaTime;
        float t = 1.0f - (p.life / p.maxLife);
        p.size = p.startSize + (p.endSize - p.startSize) * t;
        p.color.r = p.startColor.r + (p.endColor.r - p.startColor.r) * t;
        p.color.g = p.startColor.g + (p.endColor.g - p.startColor.g) * t;
        p.color.b = p.startColor.b + (p.endColor.b - p.startColor.b) * t;
        p.color.a = p.startColor.a + (p.endColor.a - p.startColor.a) * t;
    }
    if (emitting) {
        emissionAccumulator += emissionRate * deltaTime;
        while (emissionAccumulator >= 1.0f) { emit(1); emissionAccumulator -= 1.0f; }
    }
}

void ParticleEmitterImpl::draw(Canvas& canvas) {
    for (const auto& p : particles) {
        if (!p.active) continue;
        if (texture) {
            canvas.save();
            canvas.translate(p.x, p.y);
            canvas.rotate(p.rotation);
            canvas.drawImage(texture, Rect(-p.size / 2, -p.size / 2, p.size, p.size), p.color);
            canvas.restore();
        } else {
            canvas.drawCircle(p.x, p.y, p.size / 2, Paint::fill(p.color));
        }
    }
}

void ParticleEmitterImpl::emit(int count) {
    for (int i = 0; i < count; i++) {
        Particle* p = nullptr;
        for (auto& particle : particles) {
            if (!particle.active) { p = &particle; break; }
        }
        if (!p) {
            if (static_cast<int>(particles.size()) >= maxParticles) return;
            particles.push_back(Particle());
            p = &particles.back();
        }
        p->active = true;
        switch (shape) {
            case EmissionShape::Point: p->x = x; p->y = y; break;
            case EmissionShape::Circle: {
                float angle = randomFloat(0.0f, 2.0f * PI);
                float r = randomFloat(0.0f, shapeRadius);
                p->x = x + r * std::cos(angle);
                p->y = y + r * std::sin(angle);
                break;
            }
            case EmissionShape::Rectangle:
                p->x = x + randomFloat(-shapeWidth / 2, shapeWidth / 2);
                p->y = y + randomFloat(-shapeHeight / 2, shapeHeight / 2);
                break;
        }
        float angle = randomFloat(angleMin, angleMax) * DEG_TO_RAD;
        float speed = randomFloat(speedMin, speedMax);
        p->vx = speed * std::cos(angle);
        p->vy = speed * std::sin(angle);
        p->life = randomFloat(lifetimeMin, lifetimeMax);
        p->maxLife = p->life;
        p->size = sizeStart;
        p->startSize = sizeStart;
        p->endSize = sizeEnd;
        p->color = colorStart;
        p->startColor = colorStart;
        p->endColor = colorEnd;
        p->rotation = 0.0f;
        p->rotationSpeed = randomFloat(rotationSpeedMin, rotationSpeedMax);
    }
}

void ParticleEmitterImpl::reset() {
    for (auto& p : particles) p.active = false;
    emissionAccumulator = 0.0f;
}

int ParticleEmitterImpl::getActiveParticleCount() const {
    int count = 0;
    for (const auto& p : particles) if (p.active) count++;
    return count;
}

// ============================================================================
// ParticleEmitter Implementation
// ============================================================================

ParticleEmitter::ParticleEmitter() : m_impl(std::make_shared<ParticleEmitterImpl>()) {}
ParticleEmitter ParticleEmitter::create() { return ParticleEmitter(); }
ParticleEmitter& ParticleEmitter::position(float x, float y) { m_impl->x = x; m_impl->y = y; return *this; }
ParticleEmitter& ParticleEmitter::texture(const std::string& path) { m_impl->texture = KillerGK::Texture::loadFromFile(path); return *this; }
ParticleEmitter& ParticleEmitter::texture(const TextureHandle& tex) { m_impl->texture = tex; return *this; }
ParticleEmitter& ParticleEmitter::emissionRate(float particlesPerSecond) { m_impl->emissionRate = particlesPerSecond; return *this; }
ParticleEmitter& ParticleEmitter::maxParticles(int max) { m_impl->maxParticles = max; return *this; }
ParticleEmitter& ParticleEmitter::lifetime(float min, float max) { m_impl->lifetimeMin = min; m_impl->lifetimeMax = max; return *this; }
ParticleEmitter& ParticleEmitter::speed(float min, float max) { m_impl->speedMin = min; m_impl->speedMax = max; return *this; }
ParticleEmitter& ParticleEmitter::angle(float min, float max) { m_impl->angleMin = min; m_impl->angleMax = max; return *this; }
ParticleEmitter& ParticleEmitter::size(float start, float end) { m_impl->sizeStart = start; m_impl->sizeEnd = end; return *this; }
ParticleEmitter& ParticleEmitter::color(const Color& start, const Color& end) { m_impl->colorStart = start; m_impl->colorEnd = end; return *this; }
ParticleEmitter& ParticleEmitter::gravity(float x, float y) { m_impl->gravityX = x; m_impl->gravityY = y; return *this; }
ParticleEmitter& ParticleEmitter::rotationSpeed(float min, float max) { m_impl->rotationSpeedMin = min; m_impl->rotationSpeedMax = max; return *this; }
ParticleEmitter& ParticleEmitter::emitFromPoint() { m_impl->shape = ParticleEmitterImpl::EmissionShape::Point; return *this; }
ParticleEmitter& ParticleEmitter::emitFromCircle(float radius) { m_impl->shape = ParticleEmitterImpl::EmissionShape::Circle; m_impl->shapeRadius = radius; return *this; }
ParticleEmitter& ParticleEmitter::emitFromRectangle(float width, float height) { m_impl->shape = ParticleEmitterImpl::EmissionShape::Rectangle; m_impl->shapeWidth = width; m_impl->shapeHeight = height; return *this; }
ParticleEmitter& ParticleEmitter::start() { m_impl->emitting = true; return *this; }
ParticleEmitter& ParticleEmitter::stop() { m_impl->emitting = false; return *this; }
ParticleEmitterHandle ParticleEmitter::build() { return m_impl; }


// ============================================================================
// SpriteBatcher Implementation
// ============================================================================

struct SpriteBatcher::Impl {
    std::vector<BatchItem> items;
    int currentZOrder = 0;
    float viewWidth = 0.0f;
    float viewHeight = 0.0f;
    bool active = false;
    Stats stats;
};

SpriteBatcher::SpriteBatcher() : m_impl(std::make_unique<Impl>()) {}
SpriteBatcher::~SpriteBatcher() = default;

void SpriteBatcher::begin(float width, float height) {
    m_impl->items.clear();
    m_impl->viewWidth = width;
    m_impl->viewHeight = height;
    m_impl->active = true;
    m_impl->currentZOrder = 0;
    m_impl->stats = Stats{};
}

void SpriteBatcher::draw(const SpriteHandle& sprite) {
    if (!sprite || !sprite->texture) return;
    BatchItem item;
    item.texture = sprite->texture;
    item.srcRect = sprite->getSourceRect();
    item.dstRect = sprite->getDestRect();
    item.tint = sprite->tint;
    item.tint.a *= sprite->opacity;
    item.rotation = sprite->rotation;
    item.originX = sprite->originX;
    item.originY = sprite->originY;
    item.zOrder = m_impl->currentZOrder;
    m_impl->items.push_back(item);
}

void SpriteBatcher::draw(const TextureHandle& texture, const Rect& dst, const Color& tint) {
    if (!texture) return;
    BatchItem item;
    item.texture = texture;
    item.srcRect = Rect(0, 0, static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight()));
    item.dstRect = dst;
    item.tint = tint;
    item.rotation = 0.0f;
    item.originX = 0.5f;
    item.originY = 0.5f;
    item.zOrder = m_impl->currentZOrder;
    m_impl->items.push_back(item);
}

void SpriteBatcher::draw(const TextureHandle& texture, const Rect& src, const Rect& dst, const Color& tint, float rotation, float originX, float originY) {
    if (!texture) return;
    BatchItem item;
    item.texture = texture;
    item.srcRect = src;
    item.dstRect = dst;
    item.tint = tint;
    item.rotation = rotation;
    item.originX = originX;
    item.originY = originY;
    item.zOrder = m_impl->currentZOrder;
    m_impl->items.push_back(item);
}

void SpriteBatcher::setZOrder(int z) { m_impl->currentZOrder = z; }

void SpriteBatcher::sortBatch() {
    std::stable_sort(m_impl->items.begin(), m_impl->items.end(),
        [](const BatchItem& a, const BatchItem& b) {
            if (a.zOrder != b.zOrder) return a.zOrder < b.zOrder;
            return a.texture.get() < b.texture.get();
        });
}

void SpriteBatcher::flush() {
    if (m_impl->items.empty()) return;
    
    auto& renderer = KillerGK::Renderer2D::instance();
    TextureHandle currentTexture = nullptr;
    int batchCount = 0;
    
    for (const auto& item : m_impl->items) {
        if (item.texture != currentTexture) {
            currentTexture = item.texture;
            batchCount++;
        }
        renderer.drawTexturedRect(item.dstRect, item.texture, item.srcRect, item.tint);
        m_impl->stats.spriteCount++;
    }
    
    m_impl->stats.batchCount = batchCount;
    m_impl->stats.drawCalls = batchCount;
}

void SpriteBatcher::end() {
    sortBatch();
    KillerGK::Renderer2D::instance().beginBatch(m_impl->viewWidth, m_impl->viewHeight);
    flush();
    KillerGK::Renderer2D::instance().endBatch();
    m_impl->active = false;
}

SpriteBatcher::Stats SpriteBatcher::getStats() const { return m_impl->stats; }

// ============================================================================
// Tileset Implementation
// ============================================================================

Rect Tileset::getTileRect(int tileId) const {
    int localId = tileId - firstGid;
    if (localId < 0 || columns <= 0) return Rect(0, 0, static_cast<float>(tileWidth), static_cast<float>(tileHeight));
    int col = localId % columns;
    int row = localId / columns;
    float x = static_cast<float>(margin + col * (tileWidth + spacing));
    float y = static_cast<float>(margin + row * (tileHeight + spacing));
    return Rect(x, y, static_cast<float>(tileWidth), static_cast<float>(tileHeight));
}

// ============================================================================
// TilemapLayer Implementation
// ============================================================================

Tile& TilemapLayer::getTile(int x, int y) {
    static Tile emptyTile;
    if (x < 0 || x >= width || y < 0 || y >= height) return emptyTile;
    return tiles[y * width + x];
}

const Tile& TilemapLayer::getTile(int x, int y) const {
    static Tile emptyTile;
    if (x < 0 || x >= width || y < 0 || y >= height) return emptyTile;
    return tiles[y * width + x];
}

void TilemapLayer::setTile(int x, int y, const Tile& tile) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    tiles[y * width + x] = tile;
}

void TilemapLayer::resize(int newWidth, int newHeight) {
    std::vector<Tile> newTiles(newWidth * newHeight);
    for (int y = 0; y < std::min(height, newHeight); y++) {
        for (int x = 0; x < std::min(width, newWidth); x++) {
            newTiles[y * newWidth + x] = tiles[y * width + x];
        }
    }
    tiles = std::move(newTiles);
    width = newWidth;
    height = newHeight;
}

// ============================================================================
// Tilemap Implementation
// ============================================================================

struct Tilemap::Impl {
    int width = 0;
    int height = 0;
    int tileWidth = 32;
    int tileHeight = 32;
    std::vector<Tileset> tilesets;
    std::vector<TilemapLayer> layers;
};

Tilemap::Tilemap() : m_impl(std::make_unique<Impl>()) {}
Tilemap::~Tilemap() = default;

void Tilemap::create(int width, int height, int tileWidth, int tileHeight) {
    m_impl->width = width;
    m_impl->height = height;
    m_impl->tileWidth = tileWidth;
    m_impl->tileHeight = tileHeight;
    m_impl->layers.clear();
    m_impl->tilesets.clear();
}

void Tilemap::addTileset(const Tileset& tileset) {
    m_impl->tilesets.push_back(tileset);
}

int Tilemap::addLayer(const std::string& name) {
    TilemapLayer layer;
    layer.name = name;
    layer.width = m_impl->width;
    layer.height = m_impl->height;
    layer.tiles.resize(m_impl->width * m_impl->height);
    m_impl->layers.push_back(std::move(layer));
    return static_cast<int>(m_impl->layers.size()) - 1;
}

TilemapLayer& Tilemap::getLayer(int index) { return m_impl->layers[index]; }
const TilemapLayer& Tilemap::getLayer(int index) const { return m_impl->layers[index]; }

TilemapLayer* Tilemap::getLayerByName(const std::string& name) {
    for (auto& layer : m_impl->layers) {
        if (layer.name == name) return &layer;
    }
    return nullptr;
}

void Tilemap::setTile(int layer, int x, int y, int tileId) {
    if (layer < 0 || layer >= static_cast<int>(m_impl->layers.size())) return;
    Tile tile;
    tile.tileId = tileId;
    m_impl->layers[layer].setTile(x, y, tile);
}

void Tilemap::setTile(int layer, int x, int y, const Tile& tile) {
    if (layer < 0 || layer >= static_cast<int>(m_impl->layers.size())) return;
    m_impl->layers[layer].setTile(x, y, tile);
}

const Tile& Tilemap::getTile(int layer, int x, int y) const {
    static Tile emptyTile;
    if (layer < 0 || layer >= static_cast<int>(m_impl->layers.size())) return emptyTile;
    return m_impl->layers[layer].getTile(x, y);
}

const Tileset* Tilemap::findTileset(int tileId) const {
    const Tileset* result = nullptr;
    for (const auto& ts : m_impl->tilesets) {
        if (tileId >= ts.firstGid) {
            if (!result || ts.firstGid > result->firstGid) result = &ts;
        }
    }
    return result;
}

void Tilemap::drawLayer(Canvas& canvas, const TilemapLayer& layer, const Rect& viewRect) {
    if (!layer.visible) return;
    
    int startX = std::max(0, static_cast<int>((viewRect.x - layer.offsetX) / m_impl->tileWidth));
    int startY = std::max(0, static_cast<int>((viewRect.y - layer.offsetY) / m_impl->tileHeight));
    int endX = std::min(layer.width, static_cast<int>((viewRect.x + viewRect.width - layer.offsetX) / m_impl->tileWidth) + 2);
    int endY = std::min(layer.height, static_cast<int>((viewRect.y + viewRect.height - layer.offsetY) / m_impl->tileHeight) + 2);
    
    canvas.save();
    canvas.setGlobalAlpha(layer.opacity);
    
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            const Tile& tile = layer.getTile(x, y);
            if (tile.tileId < 0) continue;
            
            const Tileset* tileset = findTileset(tile.tileId);
            if (!tileset || !tileset->texture) continue;
            
            Rect srcRect = tileset->getTileRect(tile.tileId);
            float dstX = layer.offsetX + x * m_impl->tileWidth;
            float dstY = layer.offsetY + y * m_impl->tileHeight;
            Rect dstRect(dstX, dstY, static_cast<float>(m_impl->tileWidth), static_cast<float>(m_impl->tileHeight));
            
            canvas.drawImage(tileset->texture, srcRect, dstRect);
        }
    }
    
    canvas.restore();
}

void Tilemap::draw(Canvas& canvas, const Rect& viewRect) {
    for (const auto& layer : m_impl->layers) {
        drawLayer(canvas, layer, viewRect);
    }
}

void Tilemap::draw(Canvas& canvas, float viewX, float viewY, float viewWidth, float viewHeight) {
    draw(canvas, Rect(viewX, viewY, viewWidth, viewHeight));
}

Point Tilemap::worldToTile(float worldX, float worldY) const {
    return Point(std::floor(worldX / m_impl->tileWidth), std::floor(worldY / m_impl->tileHeight));
}

Point Tilemap::tileToWorld(int tileX, int tileY) const {
    return Point(static_cast<float>(tileX * m_impl->tileWidth), static_cast<float>(tileY * m_impl->tileHeight));
}

int Tilemap::getWidth() const { return m_impl->width; }
int Tilemap::getHeight() const { return m_impl->height; }
int Tilemap::getTileWidth() const { return m_impl->tileWidth; }
int Tilemap::getTileHeight() const { return m_impl->tileHeight; }
int Tilemap::getLayerCount() const { return static_cast<int>(m_impl->layers.size()); }
int Tilemap::getPixelWidth() const { return m_impl->width * m_impl->tileWidth; }
int Tilemap::getPixelHeight() const { return m_impl->height * m_impl->tileHeight; }

} // namespace KGK2D
