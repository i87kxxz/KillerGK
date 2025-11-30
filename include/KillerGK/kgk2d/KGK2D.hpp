/**
 * @file KGK2D.hpp
 * @brief 2D graphics module for KillerGK
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <memory>

namespace KGK2D {

using KillerGK::Color;
using KillerGK::Rect;
using KillerGK::Point;

/**
 * @brief Handle to a sprite
 */
using SpriteHandle = std::shared_ptr<class SpriteImpl>;

/**
 * @class Sprite
 * @brief 2D sprite with transformations
 */
class Sprite {
public:
    static Sprite create(const std::string& texturePath);

    Sprite& position(float x, float y);
    Sprite& size(float w, float h);
    Sprite& origin(float x, float y);
    Sprite& rotation(float degrees);
    Sprite& scale(float sx, float sy);
    Sprite& color(const Color& tint);
    Sprite& opacity(float alpha);
    Sprite& flipX(bool flip);
    Sprite& flipY(bool flip);

    // Animation
    Sprite& frames(int cols, int rows);
    Sprite& frame(int index);
    Sprite& animate(float fps, int startFrame, int endFrame, bool loop);

    SpriteHandle build();

private:
    Sprite();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @brief Handle to a particle emitter
 */
using ParticleEmitterHandle = std::shared_ptr<class ParticleEmitterImpl>;

/**
 * @class ParticleEmitter
 * @brief Particle system emitter
 */
class ParticleEmitter {
public:
    static ParticleEmitter create();

    ParticleEmitter& position(float x, float y);
    ParticleEmitter& texture(const std::string& path);
    ParticleEmitter& emissionRate(float particlesPerSecond);
    ParticleEmitter& lifetime(float min, float max);
    ParticleEmitter& speed(float min, float max);
    ParticleEmitter& angle(float min, float max);
    ParticleEmitter& size(float start, float end);
    ParticleEmitter& color(const Color& start, const Color& end);
    ParticleEmitter& gravity(float x, float y);

    ParticleEmitterHandle build();

private:
    ParticleEmitter();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class Canvas
 * @brief 2D drawing canvas
 */
class Canvas {
public:
    // Shapes
    void drawRect(float x, float y, float w, float h, const Color& color);
    void drawRoundRect(float x, float y, float w, float h, float radius, const Color& color);
    void drawCircle(float cx, float cy, float radius, const Color& color);
    void drawLine(float x1, float y1, float x2, float y2, const Color& color);

    // Transforms
    void save();
    void restore();
    void translate(float x, float y);
    void rotate(float radians);
    void scale(float sx, float sy);
};

} // namespace KGK2D
